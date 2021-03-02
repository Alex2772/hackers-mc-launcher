//
// Created by alex2772 on 2/15/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AComboBox.h>
#include <AUI/View/AListView.h>
#include <AUI/Model/AListModelAdapter.h>
#include <Repository/UsersRepository.h>
#include "MainWindow.h"
#include "AccountWindow.h"
#include "ImportVersionWindow.h"

MainWindow::MainWindow():
    AWindow("Hacker's Minecraft Launcher", 600_dp, 140_dp)
{
    setContents(Horizontal {
        Vertical {
            Horizontal {
                _new<ALabel>("Username:"),
                _new<ASpacer>(),
                _new<AButton>("Add user") let {
                    it->addAssName(".plus");
                    it->setIcon(AImageLoaderRegistry::inst().loadDrawable(":svg/plus.svg"));
                    connect(it->clicked, this, [&] {
                        _new<AccountWindow>(nullptr)->show();
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
                _new<AButton>() << ".configure",
            },
            //_new<AListView>(mProfilesModel),
        } << ".column",
        Stacked{
            _new<AButton>() << "#play" let { it->setDefault(); },
        },
    });


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
