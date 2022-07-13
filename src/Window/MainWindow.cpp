//
// Created by alex2772 on 2/15/21.
//

#include <AUI/View/AButton.h>
#include <AUI/View/AComboBox.h>
#include <AUI/View/AListView.h>
#include <AUI/Model/AListModelAdapter.h>
#include <Repository/UsersRepository.h>
#include <Repository/GameProfilesRepository.h>
#include <Source/LegacyLauncherJsonSource.h>
#include <AUI/ASS/ASS.h>
#include <Launcher.h>
#include <AUI/Util/APrettyFormatter.h>
#include <AUI/Platform/AMessageBox.h>
#include "MainWindow.h"
#include "AccountWindow.h"
#include "ImportVersionWindow.h"
#include "LauncherSettingsWindow.h"
#include "GameProfileWindow.h"
#include "GameConsoleWindow.h"
#include "Model/GameProcess.h"
#include "Model/Settings.h"
#include <chrono>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AScrollArea.h>
#include <View/AccountsComboBox.h>
#include <View/GameProfilesView.h>
#include <AUI/View/AHDividerView.h>
#include <AUI/View/ASpinner.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/Traits/iterators.h>
#include <AUI/Platform/ADesktop.h>

using namespace declarative;

MainWindow::MainWindow():
    AWindow("Hacker's Minecraft Launcher", 600_dp, 380_dp)
{
    setContents(
        Vertical {
            Centered::Expanding {
                (_new<AScrollArea>() let {
                    it->getContentContainer()->setLayout(_new<AVerticalLayout>());
                    it->getContentContainer()->addView(mGameProfilesView = _new<GameProfilesView>(GameProfilesRepository::inst().getModel()));
                }) with_style { MinSize { 300_dp } },
                mSpinnerView = _new<ASpinner>(),
            },
            _new<AView>() with_style { FixedSize { {}, 1_px }, Margin { 0 }, BackgroundSolid { 0x80808080_argb } },
            Stacked {
                Horizontal {
                    Vertical {
                        _new<ALabel>("Account:"),
                            mUsersListView = _new<AccountsComboBox>(AModels::adapt<AString>(UsersRepository::inst().getModel(), [](const Account& u) {
                            return u.username;
                        })) with_style { MinSize { 100_dp, {} } },
                    },
                    _new<ASpacer>(),
                    Centered {
                        Vertical {
                            Button {
                                Icon { ":svg/dir.svg" },
                                Label { "Game dir" },
                            }.clicked(me::openGameDir),
                            Button {
                                Icon { ":svg/cog.svg" },
                                Label { "Settings" }
                            }.clicked(me::showLauncherSettings),
                        }
                    },
                } with_style { Expanding{} },

                // Play button / download panel
                Stacked {
                    // downloading panel
                    mDownloadingPanel = Vertical {
                        Horizontal {
                            mStatusLabel = _new<ALabel>("Running...") let {
                                it->setCustomStyle({
                                    FontSize { 12_pt },
                                    TextColor { 0_rgb },
                                });
                            },

                            _new<ASpacer>(),

                            mDownloadedLabel = _new<ALabel>() << ".secondary",
                            _new<ALabel>("of") << ".secondary",
                            mTotalLabel = _new<ALabel>() << ".secondary",
                        },
                        mTargetFileLabel = _new<ALabel>() << ".secondary",
                    } let {
                        it->setVisibility(Visibility::GONE);
                        it << "#downloading_panel";
                    },

                    // download button
                    mPlayButton = Centered { Horizontal {
                        _new<ADrawableView>(":svg/play.svg"),
                        _new<ALabel>("Play"),
                    }} let {
                        it << "#play" << ".btn" << ".btn_default";
                        connect(it->clicked, me::onPlayButtonClicked);
                    },
                },
            } with_style { Padding { 8_dp } },
        }
    );
    showProfileLoading();
    mTask = async {
        LegacyLauncherJsonSource::load();
        ui_thread {
            hideProfileLoading();
            connect(reloadProfiles, [&] {
                showProfileLoading();
                LegacyLauncherJsonSource::reload();
                hideProfileLoading();
            });
        };
    };
}

void MainWindow::onPlayButtonClicked() {
    Account account;
    try {
        account = UsersRepository::inst().getModel()->at(mUsersListView->getSelectedId());
    } catch (...) {
        assert(("invalid user id", UsersRepository::inst().getModel()->empty()));

        // ask username
        auto accountWindow = _new<AccountWindow>(nullptr);
        connect(accountWindow->positiveAction, me::onPlayButtonClicked);
        accountWindow->show();

        return;
    }

    showDownloadingPanel();
    mDownloadedLabel->setText("0");
    mTotalLabel->setText("0");
    mTargetFileLabel->setText("");
    mTask = asyncX [this, account = std::move(account)] {
        try {
            auto launcher = _new<Launcher>();
            connect(launcher->updateStatus, slot(mStatusLabel)::setText);
            connect(launcher->updateTargetFile, slot(mTargetFileLabel)::setText);

            connect(launcher->updateTotalDownloadSize, [&](size_t s) {
                mTotalLabel->setText(APrettyFormatter::sizeInBytes(s));
            });
            connect(launcher->updateDownloadedSize, [&](size_t s) {
                mDownloadedLabel->setText(APrettyFormatter::sizeInBytes(s));
            });

            auto process = launcher->play(
                    UsersRepository::inst().getModel()->at(mUsersListView->getSelectedId()),
                    GameProfilesRepository::inst().getModel()->at(mGameProfilesView->getSelectedProfileIndex()),
                    true
            );

            auto game = _new<GameProcess>();
            game->process = process;

            connect(process->finished, [this, game] { // capture process in order to keep reference
                show();
                GameConsoleWindow::handleGameExit(this, std::move(game));
                game->process->finished.clearAllConnectionsWith(this);
                game->process->stdOut.clearAllConnectionsWith(this);
            });

            process->run(ASubProcessExecutionFlags::MERGE_STDOUT_STDERR);

            connect(process->stdOut, [game](const AByteBuffer& buffer) {
                game->stdoutBuffer << buffer;
            });

            ui_thread {
                hide();
            };

        } catch (const AException& e) {
            ALogger::err("GameLauncher") << "Could not run game: " << e;
            ui_thread {
                AMessageBox::show(this, "Could not run game", e.getMessage(), AMessageBox::Icon::CRITICAL);
            };
        }
        ui_thread {
            mPlayButton->enable();
            showPlayButton();
        };
    };
}

void MainWindow::showDownloadingPanel() {
    mPlayButton->setVisibility(Visibility::GONE);
    mDownloadingPanel->setVisibility(Visibility::VISIBLE);
}

void MainWindow::showPlayButton() {
    mDownloadingPanel->setVisibility(Visibility::GONE);
    mPlayButton->setVisibility(Visibility::VISIBLE);
}

void MainWindow::onMouseMove(glm::ivec2 pos) {
    AWindow::onMouseMove(pos);
    checkForDiskProfileUpdates();
}

void MainWindow::showUserConfigureDialogFor(unsigned int index) {
    _new<AccountWindow>(&UsersRepository::inst().getModel()->at(index)) let {
        connect(it->positiveAction, this, [&, index] {
            UsersRepository::inst().getModel()->invalidate(index);
        });
        connect(it->deleteUser, this, [&, index] {
            UsersRepository::inst().getModel()->removeAt(index);
        });
        it->show();
    };
}

void MainWindow::showGameProfileConfigureDialogFor(unsigned int index) {
    _new<GameProfileWindow>(GameProfilesRepository::inst().getModel()->at(index))->show();
}

void MainWindow::checkForDiskProfileUpdates() {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    // check for new profiles every 5 secs when cursor moves
    static milliseconds lastCheckTime = 0ms;
    if (high_resolution_clock::now().time_since_epoch() - lastCheckTime > 5s) {
        if (!mTask.isWaitNeeded()) {
            mTask = async {
                // load actual set of profiles
                decltype(auto) actualProfiles = LegacyLauncherJsonSource::getSetOfProfilesOnDisk();
                decltype(auto) loadedProfiles = GameProfilesRepository::inst().getCurrentlyLoadedSetOfProfiles();

                // and compare it with actual profiles
                if (actualProfiles != loadedProfiles) {
                    // found new game profile!
                    ALogger::info("Detected changes in launcher_profiles.json, reloading");
                    emit reloadProfiles();
                }
            };
        }

        lastCheckTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    }

}

void MainWindow::showProfileLoading() {
    mGameProfilesView->setCustomStyle({ Opacity { 0.1f } });
    mSpinnerView->setVisibility(Visibility::VISIBLE);
}
void MainWindow::hideProfileLoading() {
    mGameProfilesView->setCustomStyle({ Opacity { 1.f } });
    mSpinnerView->setVisibility(Visibility::GONE);
}


void MainWindow::openGameDir() {
    ADesktop::openUrl(Settings::inst().gameDir);
}

void MainWindow::showLauncherSettings() {
    _new<LauncherSettingsWindow>()->show();
}
