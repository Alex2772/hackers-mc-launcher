//
// Created by alex2 on 15.04.2021.
//

#include "LauncherSettingsWindow.h"
#include "MainWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATabView.h>
#include <AUI/View/APathChooserView.h>
#include <AUI/Json/AJson.h>
#include <Model/Settings.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/ANumberPicker.h>
#include <AUI/View/ACheckBox.h>


LauncherSettingsWindow::LauncherSettingsWindow() :
        AWindow("Settings", 400_dp, 300_dp, Autumn::get<MainWindow>().get(), WS_DIALOG) {
    auto binding = _new<ADataBinding<Settings>>(Settings::inst());
    setContents(
        Vertical {
            _new<ATabView>() let {
                it->addTab(
                    Vertical{
                        _form({
                            {"Game folder:"_as, _new<APathChooserView>() && binding(&Settings::game_folder)},
                            {
                                "Display:"_as,
                                Horizontal {
                                    _new<ANumberPicker>() && binding(&Settings::width),
                                    _new<ALabel>("x"),
                                    _new<ANumberPicker>() && binding(&Settings::height),
                                    _new<ACheckBox>("Fullscreen") && binding(&Settings::is_fullscreen),
                                }
                            },
                        }),
                    }, "Game"
                );
            },
            _new<ASpacer>(),
            Horizontal {
                _new<AButton>("Reset to defaults").connect(&AButton::clicked, this, [this, binding] {
                    Settings::reset();
                    binding->setModel(Settings::inst());
                    close();
                }),
                _new<ASpacer>(),
                _new<AButton>("OK").connect(&AButton::clicked, this, [this, binding] {
                    Settings::inst() = binding->getModel();
                    Settings::save();
                    close();
                }) let { it->setDefault(); },
                _new<AButton>("Cancel").connect(&AView::clicked, me::close),
            }
        }
    );
}
