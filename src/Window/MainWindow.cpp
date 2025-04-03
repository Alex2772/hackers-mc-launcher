//
// Created by alex2772 on 2/15/21.
//

#include <AUI/View/AButton.h>
#include <AUI/View/ADropdownList.h>
#include <AUI/View/AListView.h>
#include <AUI/Model/AListModelAdapter.h>
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
#include <View/GameProfilesView.h>
#include <AUI/View/AHDividerView.h>
#include <AUI/View/ASpinnerV2.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/Traits/iterators.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/ACleanup.h>
#include <AUI/Platform/APlatform.h>

using namespace declarative;

MainWindow::MainWindow():
    AWindow("Hacker's Minecraft Launcher", 600_dp, 380_dp)
{
    setContents(
        Vertical {
            Centered::Expanding {
                (AScrollArea::Builder().withContents(_new<GameProfilesView>(mState.profile)).build()) with_style { MinSize { 300_dp } },
                mSpinnerView = _new<ASpinnerV2>(),
            },
            _new<AView>() with_style { FixedSize { {}, 1_px }, Margin { 0 }, BackgroundSolid { 0x80808080_argb } },
            Stacked {
                Horizontal {
                    Vertical {
                        _new<ALabel>("Username:"),
                        _new<ATextField>() && mState.accounts.current->username,
                    } with_style { MinSize { 100_dp, {} } },
                    SpacerExpanding{},
                    Centered {
                        Vertical {
                            Button {
                                Icon { ":svg/plus.svg" },
                                Label { "Import version..." },
                            }.clicked(this, [this] {
                                _new<ImportVersionWindow>(mState)->show();
                            }),
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
                            _new<ASpinnerV2>(),
                            mStatusLabel = _new<ALabel>("Running...") let {
                                it->setCustomStyle({
                                    FontSize { 12_pt },
                                    TextColor { 0_rgb },
                                });
                            },

                            SpacerExpanding{},

                            mDownloadedLabel = _new<ALabel>() << ".secondary",
                            _new<ALabel>("/") << ".secondary",
                            mTotalLabel = _new<ALabel>() << ".secondary",
                        },
                        mTargetFileLabel = _new<ALabel>() << ".secondary" with_style { ATextOverflow::ELLIPSIS },
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
    auto state = _new<State>(mState);
    mTask = async {
        AUI_DEFER {
            ui_thread {
                mState = std::move(*state);
                hideProfileLoading();
                connect(reloadProfiles, [&] {
                    showProfileLoading();
                    AUI_DEFER { hideProfileLoading(); };
                    LegacyLauncherJsonSource::reload(mState);
                });
            };
        };
        LegacyLauncherJsonSource::load(*state);
    };
}



void MainWindow::onPlayButtonClicked() {
    auto account = mState.accounts.current;
    if (account == nullptr || account->username->empty()) {
        // ask username
        auto accountWindow = _new<AccountWindow>(mState, nullptr);
        connect(accountWindow->positiveAction, me::onPlayButtonClicked);
        accountWindow->show();
        return;
    }

    _<GameProfile> profile = *mState.profile.selected;
    if (!profile) {
        if (mState.profile.list->empty()) {
            _new<ImportVersionWindow>(mState)->show();
            return;
        }
        AMessageBox::show(this, "Hacker's MC Launcher", "Please select a profile.");
        return;
    }

    showDownloadingPanel();
    mDownloadedLabel->setText("0");
    mTotalLabel->setText("0");
    mTargetFileLabel->setText("");
    mTask = asyncX [this, account = std::move(account), profile = std::move(profile)] {
        try {
            *account->populateUuid();

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
                    *account,
                    *profile,
                    true
            );

            auto game = _new<GameProcess>();
            game->process = process;

            connect(process->finished, [this, game] { // capture process in order to keep reference
                show();
                GameConsoleWindow::handleGameExit(this, std::move(game));
                game->process->finished.clearAllOutgoingConnectionsWith(this);
                game->process->stdOut.clearAllOutgoingConnectionsWith(this);
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

void MainWindow::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AWindow::onPointerMove(pos, event);
    checkForDiskProfileUpdates();
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
                decltype(auto) loadedProfiles = mState.profile.uuids();

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
    mSpinnerView->setVisibility(Visibility::VISIBLE);
}
void MainWindow::hideProfileLoading() {
    mSpinnerView->setVisibility(Visibility::GONE);
}


void MainWindow::openGameDir() {
    APlatform::openUrl(*Settings::inst().gameDir);
}

void MainWindow::showLauncherSettings() {
    _new<LauncherSettingsWindow>()->show();
}

MainWindow& MainWindow::inst() {
    static auto a = aui::ptr::manage(new MainWindow);
    ACleanup::afterEntry([&] {
        a = nullptr;
    });
    return *a;
}
