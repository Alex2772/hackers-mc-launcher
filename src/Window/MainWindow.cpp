//
// Created by alex2772 on 2/15/21.
//

#include <range/v3/all.hpp>
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
#include "AUI/View/ASpacerFixed.h"
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
static constexpr auto LOG_TAG = "MainWindow";

MainWindow::MainWindow():
    AWindow("Hacker's Minecraft Launcher", 600_dp, 380_dp)
{
    setContents(
        Vertical {
            Centered::Expanding {
                (AScrollArea::Builder().withContents(_new<GameProfilesView>(mState.profile)).build()) AUI_OVERRIDE_STYLE { MinSize { 300_dp } },
                mSpinnerView = _new<ASpinnerV2>(),
            },
            _new<AView>() AUI_OVERRIDE_STYLE { FixedSize { {}, 1_px }, Margin { 0 }, BackgroundSolid { 0x80808080_argb } },
           Horizontal {
               Centered {
                   Vertical {
                       _new<ALabel>("Username:"),
                       _new<ATextField>() && mState.accounts.current->username,
                   } AUI_OVERRIDE_STYLE { MinSize { 100_dp, {} } },
               },
               SpacerExpanding{},
               playButton(),
               Centered {
                   Horizontal {
                       Vertical {
                           Button {
                               .content = Horizontal {
                                   Icon { ":svg/plus.svg" },
                                   SpacerFixed { 2_dp },
                                   Label { "Import version..." },
                               },
                               .onClick = [this] {
                                   ui::presentImportVersion(mState);
                               },
                           },
                           Button {
                               .content = Horizontal {
                                   Icon { ":svg/wrench.svg" },
                                   SpacerFixed { 2_dp },
                                   Label { "Edit profile..." },
                               },
                               .onClick = [this] {
                                   editCurrentProfile();
                               }
                           } AUI_LET {
                               connect(mState.profile.selected, [it](const _<GameProfile>& p){
                                   it->setEnabled(p != nullptr);
                               });
                           },
                       } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
                       Vertical {
                           Button {
                               .content = Horizontal {
                                   Icon { ":svg/cog.svg" },
                                   SpacerFixed { 2_dp },
                                   Label { "Settings" },
                               },
                               .onClick = [this] { showLauncherSettings(); },
                           },
                           Button {
                               .content = Horizontal {
                                   Icon { ":svg/dir.svg" },
                                   SpacerFixed { 2_dp },
                                   Label { "Game dir" },
                               },
                               .onClick = [this] { openGameDir(); },
                           },
                       } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
                   } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
               },
           } AUI_OVERRIDE_STYLE { Expanding{}, Padding { 8_dp }, LayoutSpacing { 8_dp } },

        }
    );
    showProfileLoading();
    auto state = _new<State>(mState);
    mTask = AUI_THREADPOOL {
        AUI_DEFER {
            AUI_UI_THREAD {
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
_<AView> MainWindow::playButton() {
    // Play button / download panel
    return Stacked {
        // downloading panel
        mDownloadingPanel = Vertical {
            Horizontal {
                _new<ASpinnerV2>(),
                mStatusLabel = _new<ALabel>("Running...") AUI_LET {
                    it->setCustomStyle({
                        FontSize { 12_pt },
                        TextColor { 0_rgb },
                    });
                },

                SpacerExpanding{},

                mDownloadedLabel = _new<ALabel>() << ".secondary",
                _new<ALabel>("/") << ".secondary",
                mTotalLabel = _new<ALabel>() << ".secondary",
            } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
            mTargetFileLabel = _new<ALabel>() << ".secondary" AUI_OVERRIDE_STYLE { ATextOverflow::ELLIPSIS, Expanding {1, 0} },
        } AUI_LET {
            it->setVisibility(Visibility::GONE);
            it << "#downloading_panel";
        },

        // download button
        mPlayButton = Button {
            .content = Horizontal {
                Icon { ":svg/play.svg" },
                SpacerFixed { 2_dp },
                Label { "Play" },
              },
              .onClick = [this] { onPlayButtonClicked(); },
              .isDefault = true,
          }() << "#play" << ".btn" << ".btn_default",
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
            ui::presentImportVersion(mState);
            return;
        }
        AMessageBox::show(this, "Hacker's MC Launcher", "Please select a profile.");
        return;
    }

    showDownloadingPanel();
    mDownloadedLabel->setText("0");
    mTotalLabel->setText("0");
    mTargetFileLabel->setText("");
    mTask = AUI_THREADPOOL_X [this, account = std::move(account), profile = std::move(profile)] {
        try {
            auto game = _new<GameProcess>();
            ALogger::global().onLogged([game](const AString& prefix, const AString& message, ALogger::Level level) {
                AUI_UI_THREAD {
                    game->stdoutBuffer.writeScope() << message;
                };
            });
            if (*Settings::inst().showConsoleOnPlay) {
                AUI_UI_THREAD {
                    _new<GameConsoleWindow>(this, game)->show();
                };
            }

            *account->populateUuid();

            auto launcher = _new<Launcher>();
            connect(launcher->updateStatus, AUI_SLOT(mStatusLabel)::setText);
            connect(launcher->updateTargetFile, AUI_SLOT(mTargetFileLabel)::setText);

            connect(launcher->updateTotalDownloadSize, [&](size_t s) {
                mTotalLabel->setText(APrettyFormatter::sizeInBytes(s));
            });
            connect(launcher->updateDownloadedSize, [&](size_t s) {
                mDownloadedLabel->setText(APrettyFormatter::sizeInBytes(s));
            });

            auto process = launcher->play(
                    *account,
                    *profile,
                   false
            );

            game->process = process;

            connect(process->finished, [this, game] { // capture process in order to keep reference
                show();

                ALogger::global().onLogged(nullptr);

                if (!*Settings::inst().showConsoleOnPlay) {
                    GameConsoleWindow::handleGameExit(this, std::move(game));
                }
                game->process->finished.clearAllOutgoingConnectionsWith(this);
                game->process->stdOut.clearAllOutgoingConnectionsWith(this);
            });

            process->run(ASubProcessExecutionFlags::MERGE_STDOUT_STDERR);

            connect(process->stdOut, [game](const AByteBuffer& buffer) {
                auto line = AString::fromUtf8(buffer);
                while (!line.empty() && line.back() == '\n') {
                    line.pop_back();
                }
                ALogger::info("Minecraft") << line;
            });

            AUI_UI_THREAD {
                hide();
            };

        } catch (const AException& e) {
            ALogger::err("GameLauncher") << "Could not run game: " << e;
            auto msg = e.getMessage();
            AUI_UI_THREAD {
                AMessageBox::show(this, "Could not run game", msg, AMessageBox::Icon::CRITICAL);
            };
        }
        AUI_UI_THREAD {
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

    // check for new profiles every 5 secs when cursor moves.
    // this is the case when a user install Forge or OptiFine via respective installer.
    static milliseconds lastCheckTime = 0ms;
    if (high_resolution_clock::now().time_since_epoch() - lastCheckTime > 5s) {
        if (!mTask.isWaitNeeded()) {
            mTask = AUI_THREADPOOL {
                // load actual set of profiles
                decltype(auto) profilesOnDisk = LegacyLauncherJsonSource::getSetOfProfilesOnDisk();
                AUI_UI_THREAD_X [this, profilesOnDisk = std::move(profilesOnDisk)] {
                    const auto& profilesInMemory = mState.profilesUuidsSnapshot;
                    // and compare it with actual profiles
                    if (ranges::any_of(profilesOnDisk, [&](const AUuid& u) { return !profilesInMemory.contains(u); })) {
                        // found new game profile!
                        ALogger::info("Detected changes in launcher_profiles.json, reloading");
                        ALOG_DEBUG(LOG_TAG) << "Profiles on disk: " << profilesOnDisk;
                        ALOG_DEBUG(LOG_TAG) << "Profiles in memory: " << profilesInMemory;
                        emit reloadProfiles();
                    }
                };
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
    static auto a = aui::ptr::manage_shared(new MainWindow);
    ACleanup::afterEntry([&] {
        a = nullptr;
    });
    return *a;
}

void MainWindow::onCloseButtonClicked() {
    AWindow::onCloseButtonClicked();
    LegacyLauncherJsonSource::save(mState);
}

void MainWindow::editCurrentProfile() {
    editProfile(mState.profile.selected);
}

void MainWindow::editProfile(_<GameProfile> profile) {
    if (profile == nullptr) {
        return;
    }
    _new<GameProfileWindow>(std::move(profile))->show();
}
