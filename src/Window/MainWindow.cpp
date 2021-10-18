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
#include <chrono>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AImageView.h>
#include <View/AccountsComboBox.h>
#include <View/GameProfilesView.h>
#include <AUI/View/AHDividerView.h>

using namespace ass;

MainWindow::MainWindow():
    AWindow("Hacker's Minecraft Launcher", 600_dp, 180_dp)
{
    setContents(
        Vertical {
            (_new<AScrollArea>() let {
                //it->getContentContainer()->setLayout(_new<AVerticalLayout>());
                //it->getContentContainer()->addView(mGameProfilesView = _new<GameProfilesView>(GameProfilesRepository::inst().getModel()));
            }) with_style { MinSize { 300_dp } },
            _new<AHDividerView>(),
            Stacked {
                Horizontal {
                    Vertical {
                        _new<ALabel>("Account:"),
                        mUsersListView = _new<AccountsComboBox>(AAdapter::make<Account>(UsersRepository::inst().getModel(), [](const Account& u) {
                            return u.username;
                        })) with_style { MinSize { 100_dp, {} } },
                    },
                    _new<ASpacer>(),
                    _new<AButton>().connect(&AView::clicked, this, [&] {
                        _new<LauncherSettingsWindow>()->show();
                    }) << "#settings",
                } with_style { Expanding{} },

                // Play button / download panel
                Stacked {
                    // downloading panel
                    mDownloadingPanel = Vertical {
                        Horizontal {
                            mStatusLabel = _new<ALabel>("Running...") let {
                                it->setCustomAss({
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
                    mPlayButton = _new<AButton>("Play").connect(&AButton::clicked, me::onPlayButtonClicked) << "#play" let { it->setDefault(); },
                },
            },
        }
    );


}

void MainWindow::onPlayButtonClicked() {
    showDownloadingPanel();
    mDownloadedLabel->setText("0");
    mTotalLabel->setText("0");
    mTargetFileLabel->setText("");
    asyncX [this] {
        auto launcher = _new<Launcher>();
        connect(launcher->updateStatus, slot(mStatusLabel)::setText);
        connect(launcher->updateTargetFile, slot(mTargetFileLabel)::setText);
        connect(launcher->errorOccurred, [&](const AString& message) {
            showPlayButton();
            AMessageBox::show(this, "Could not run game", message, AMessageBox::Icon::CRITICAL);
        });
        connect(launcher->updateTotalDownloadSize, [&](size_t s) {
            mTotalLabel->setText(APrettyFormatter::sizeInBytes(s));
        });
        connect(launcher->updateDownloadedSize, [&](size_t s) {
            mDownloadedLabel->setText(APrettyFormatter::sizeInBytes(s));
        });
        AThread::sleep(1000);

        launcher->play(
                UsersRepository::inst().getModel()->at(mUsersListView->getSelectedId()),
                GameProfilesRepository::inst().getModel()->at(mGameProfilesView->getSelectedProfileIndex()),
                true
                );
        mPlayButton->enable();
        showPlayButton();
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


