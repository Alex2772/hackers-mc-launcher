//
// Created by alex2772 on 2/15/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AComboBox.h>
#include <AUI/View/AListView.h>
#include <AUI/Model/AListModelAdapter.h>
#include <Repository/UsersRepository.h>
#include <Repository/GameProfilesRepository.h>
#include <Source/LegacyLauncherJsonSource.h>
#include <AUI/ASS/ASS.h>
#include <Launcher.h>
#include "MainWindow.h"
#include "UserWindow.h"
#include "ImportVersionWindow.h"
#include "LauncherSettingsWindow.h"
#include "GameProfileWindow.h"

using namespace ass;

MainWindow::MainWindow():
    AWindow("Hacker's Minecraft Launcher", 600_dp, 180_dp)
{
    setContents(
        Vertical {
            Horizontal {
                Vertical {
                    Horizontal {
                        _new<ALabel>("Username:"),
                        _new<ASpacer>(),
                        _new<AButton>("Add user") let {
                            it->addAssName(".plus");
                            it->setIcon(AImageLoaderRegistry::inst().loadDrawable(":svg/plus.svg"));
                            connect(it->clicked, this, [&] {
                                _new<UserWindow>(nullptr)->show();
                            });
                        },
                        mUserConfigureButton = _new<AButton>() let {
                            it->addAssName(".configure");
                            it->setDisabled();
                            connect(it->clicked, this, [&] {
                                showUserConfigureDialogFor(mUsersListView->getSelectionModel().one().getRow());
                            });
                        },
                    },
                    mUsersListView = _new<AListView>(AAdapter::make<User>(UsersRepository::inst().getModel(), [](const User& u) {
                        return u.username;
                    })) let {
                        connect(it->selectionChanged, this, [&](const AModelSelection<AString>& e) {
                            mUserConfigureButton->setEnabled(!e.empty());
                        });
                        connect(it->itemDoubleClicked, this, [&](unsigned i) {
                            showUserConfigureDialogFor(i);
                        });
                    },

                } << ".column",
                Vertical {
                    Horizontal {
                        _new<ALabel>("Version:"),
                        _new<ASpacer>(),
                        _new<AButton>("Import version").connect(&AView::clicked, this, [] {
                            _new<ImportVersionWindow>()->show();
                        }) let {
                            it->addAssName(".plus");
                            it->setIcon(AImageLoaderRegistry::inst().loadDrawable(":svg/plus.svg"));
                        },
                        mGameProfileConfigureButton = _new<AButton>() let {
                            it->addAssName(".configure");
                            it->setDisabled();

                            connect(it->clicked, this, [&] {
                                showGameProfileConfigureDialogFor(mUsersListView->getSelectionModel().one().getRow());
                            });
                        },
                    },
                    mGameProfilesListView = _new<AListView>(AAdapter::make<GameProfile>(GameProfilesRepository::inst().getModel(), [](const GameProfile& profile) {
                        return profile.getName();
                    })) let {
                        connect(it->selectionChanged, this, [&](const AModelSelection<AString>& e) {
                            mGameProfileConfigureButton->setEnabled(!e.empty());
                        });
                        connect(it->itemDoubleClicked, this, [&](unsigned i) {
                            showGameProfileConfigureDialogFor(i);
                        });
                    },
                } << ".column",
                Stacked {
                    Vertical {
                        Horizontal {
                            _new<ASpacer>(),
                            _new<AButton>().connect(&AView::clicked, this, [&] {
                                _new<LauncherSettingsWindow>()->show();
                            }) << "#settings",
                        } let {it->setExpanding({2, 2});},
                        _new<ASpacer>(),
                    } let {it->setExpanding({2, 2});},
                    mPlayButton = _new<AButton>().connect(&AButton::clicked, this, [&] {
                        mPlayButton->disable();
                        mDownloadingPanel->setVisibility(Visibility::VISIBLE);
                        async {
                            auto launcher = _new<Launcher>();
                            connect(launcher->statusLabel, slot(mStatusLabel)::setText);
                            launcher->play(
                                    UsersRepository::inst().getModel()->at(mUsersListView->getSelectionModel().one().getRow()),
                                    GameProfilesRepository::inst().getModel()->at(mGameProfilesListView->getSelectionModel().one().getRow())
                                    );
                        };
                    }) << "#play" let { it->setDefault(); },
                },
            },


            // downloading panel
            mDownloadingPanel = Vertical {
                Horizontal{
                    mStatusLabel = _new<ALabel>("Running...") let {
                        it->setCustomAss({
                            FontSize { 20_pt },
                            TextColor { 0_rgb },
                        });
                     },

                    _new<ASpacer>(),

                    _new<ALabel>("Жопа") let {
                        it->setCustomAss({
                            TextColor { 0x444444_rgb },
                        });
                    },
                },
            } let {
                it->setVisibility(Visibility::GONE);
            }
        }
    );


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

