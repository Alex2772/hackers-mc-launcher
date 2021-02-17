//
// Created by alex2772 on 2/15/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AComboBox.h>
#include <AUI/View/AListView.h>
#include "MainWindow.h"
#include "AccountWindow.h"

MainWindow::MainWindow():
    AWindow("Hacker's Minecraft Launcher", 600_dp, 140_dp)
{
    mUsersModel = _new<AListModel<AString>>({
        "<no user>"
    });
    mProfilesModel = _new<AListModel<AString>>({
        "<latest>"
    });

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
                _new<AButton>() << ".configure",
            },
            _new<AListView>(mUsersModel),

        } << ".column",
        Vertical {
            Horizontal {
                _new<ALabel>("Version:"),
                _new<ASpacer>(),
                _new<AButton>("Import version") let {
                    it->addAssName(".plus");
                    it->setIcon(AImageLoaderRegistry::inst().loadDrawable(":svg/plus.svg"));
                },
                _new<AButton>() << ".configure",
            },
            _new<AListView>(mProfilesModel),
        } << ".column",
        Stacked{
            _new<AButton>() << "#play" let { it->setDefault(); },
        },
    });

}
