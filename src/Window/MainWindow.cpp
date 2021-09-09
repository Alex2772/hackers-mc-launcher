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
#include "UserWindow.h"
#include "ImportVersionWindow.h"
#include "LauncherSettingsWindow.h"
#include "GameProfileWindow.h"
#include <chrono>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AImageView.h>

using namespace ass;

MainWindow::MainWindow():
    AWindow("Hacker's Minecraft Launcher", 600_dp, 180_dp)
{
    setContents(
        Vertical {
            (_new<AScrollArea>() let {
                it->getContentContainer()->setLayout(_new<AVerticalLayout>());
                it->getContentContainer()->addView(ui_for(profile, GameProfilesRepository::inst().getModel(), AWordWrappingLayout) {
                    return Vertical {
                        Stacked{
                            _new<AImageView>(":profile_icons/default.png"_url),
                        } << ".version_item_wrap",
                        _new<ALabel>(profile.getName()),
                    } << ".version_item";
                });
            }) with_style { MinSize { 300_dp } },
            Horizontal {
                Vertical {
                    mUsersListView = _new<AComboBox>(AAdapter::make<User>(UsersRepository::inst().getModel(), [](const User& u) {
                        return u.username;
                    })),

                },
                Horizontal {
                    mPlayButton = _new<AButton>().connect(&AButton::clicked, this, [&] {
                        mPlayButton->disable();
                        mDownloadingPanel->setVisibility(Visibility::VISIBLE);
                        mDownloadedLabel->setText("0");
                        mTotalLabel->setText("0");
                        mTargetFileLabel->setText("");
                        async {
                            auto launcher = _new<Launcher>();
                            connect(launcher->updateStatus, slot(mStatusLabel)::setText);
                            connect(launcher->updateTargetFile, slot(mTargetFileLabel)::setText);
                            connect(launcher->errorOccurred, [&](const AString& message) {
                                mDownloadingPanel->setVisibility(Visibility::GONE);
                                mPlayButton->enable();
                                AMessageBox::show(this, "Could not run game", message, AMessageBox::Icon::CRITICAL);
                            });
                            connect(launcher->updateTotalDownloadSize, [&](size_t s) {
                                mTotalLabel->setText(APrettyFormatter::sizeInBytes(s));
                            });
                            connect(launcher->updateDownloadedSize, [&](size_t s) {
                                mDownloadedLabel->setText(APrettyFormatter::sizeInBytes(s));
                            });
                            /*
                            launcher->play(
                                    UsersRepository::inst().getModel()->at(mUsersListView->getSelectionModel().one().getRow()),
                                    GameProfilesRepository::inst().getModel()->at(mGameProfilesListView->getSelectionModel().one().getRow()),
                                    true
                                    );*/
                            mPlayButton->enable();
                            mDownloadingPanel->setVisibility(Visibility::GONE);
                        };
                    }) << "#play" let { it->setDefault(); },
                },
                _new<AButton>().connect(&AView::clicked, this, [&] {
                    _new<LauncherSettingsWindow>()->show();
                }) << "#settings"
            },



            // downloading panel
            mDownloadingPanel = Vertical {
                Horizontal {
                    mStatusLabel = _new<ALabel>("Running...") let {
                        it->setCustomAss({
                            FontSize { 20_pt },
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
            }
        }
    );


}

void MainWindow::onMouseMove(glm::ivec2 pos) {
    AWindow::onMouseMove(pos);
    checkForDiskProfileUpdates();
}

void MainWindow::showUserConfigureDialogFor(unsigned int index) {
    _new<UserWindow>(&UsersRepository::inst().getModel()->at(index)) let {
        connect(it->finished, this, [&, index] {
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
        async {
            // load actual set of profiles
            ASet<AUuid> actualProfiles = LegacyLauncherJsonSource::getSetOfProfilesOnDisk();
            ASet<AUuid> loadedProfiles;

            // fill "loadedProfiles"
            for (GameProfile& p : GameProfilesRepository::inst().getModel()) { loadedProfiles << p.getUuid(); }

            // and compare it with actual profiles
            if (actualProfiles != loadedProfiles) {
                // found new game profile!
                ALogger::info("Detected changes in launcher_profiles.json, reloading");
                ui_thread {
                    LegacyLauncherJsonSource::reload();
                };
            }
        };

        lastCheckTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    }

}

