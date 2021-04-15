//
// Created by alex2 on 15.04.2021.
//

#include "LauncherSettingsWindow.h"
#include "MainWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATabView.h>
#include <AUI/View/APathChooserView.h>
#include <AUI/Json/AJson.h>

struct Settings {
    AString game_folder;
    AString azaza;

    AJSON_FIELDS(game_folder, azaza)
};

LauncherSettingsWindow::LauncherSettingsWindow() :
        AWindow("Settings", 400_dp, 300_dp, Autumn::get<MainWindow>().get(), WS_DIALOG) {
    setContents(
        Vertical {
            _new<ATabView>() let {
                it->addTab(
                    Vertical{
                        _form({
                            {"Game folder:"_as, _new<APathChooserView>()},
                        })
                    }, "Launcher"
                );
            },
            _new<ASpacer>(),
            Horizontal {
                _new<ASpacer>(),
                _new<AButton>("OK") let { it->setDefault(); },
                _new<AButton>("Cancel").connect(&AView::clicked, me::close),
            }
        }
    );
}
