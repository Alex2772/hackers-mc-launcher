//
// Created by alex2 on 15.04.2021.
//

#include "LauncherSettingsWindow.h"
#include "MainWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATabView.h>
#include <AUI/View/AText.h>
#include <AUI/View/APathChooserView.h>
#include <AUI/Json/AJson.h>
#include <Model/Settings.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/ANumberPicker.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/ASS/ASS.h>
#include <AUI/View/ASpinner.h>
#include <AUI/Platform/APlatform.h>

using namespace ass;

LauncherSettingsWindow::LauncherSettingsWindow() :
        AWindow("Settings", 400_dp, 400_dp, &MainWindow::inst(), WindowStyle::MODAL | WindowStyle::NO_RESIZE) {
    using namespace declarative;

    auto binding = _new<ADataBinding<Settings>>(Settings::inst());

    _<AView> resolutionView;
    _<ACheckBoxWrapper> fullscreenCheckbox;

    setContents(
        Vertical {
            _new<ATabView>() let {
                // GAME TAB ============================================================================================
                it->addTab(
                    Vertical{
                        _form({
                            {
                                "Game dir:"_as,
                                Horizontal {
                                    _new<APathChooserView>() let {
                                        it->setExpanding();
                                        it && binding(&Settings::gameDir);
                                    },
                                    mClearGameDirButton = _new<AButton>("Clear game dir").connect(&AView::clicked, me::clearGameDir),
                                    mClearGameDirSpinner = _new<ASpinner>() let {
                                        it->setVisibility(Visibility::GONE);
                                    },
                                }
                            },
                            {
                                "Display:"_as,
                                Horizontal {
                                    resolutionView = Horizontal {
                                        _new<ANumberPicker>() && binding(&Settings::width),
                                        _new<ALabel>("x"),
                                        _new<ANumberPicker>() && binding(&Settings::height),
                                    },
                                    fullscreenCheckbox = CheckBoxWrapper { Label { "Fullscreen" } } && binding(&Settings::isFullscreen),
                                }
                            },
                        }),
                    }, "Game"
                );

                // ABOUT TAB ===========================================================================================
                it->addTab(
                    Vertical{
                        _new<ALabel>("Hacker's Minecraft Launcher") let {
                            it->setCustomStyle({
                                FontSize { 19_pt },
                                Margin { 8_dp, 0, 4_dp },
                                ATextAlign::CENTER,
                            });
                        },
                        _new<ALabel>("Version " HACKERS_MC_VERSION) let {
                            it->setCustomStyle({
                                FontSize { 8_pt },
                                //Margin { 0, 0, 4_dp },
                                ATextAlign::CENTER,
                                TextColor { 0x444444_rgb },
                            });
                        },
                        _new<ALabel>("Distributed under GNU General Public License v3") let {
                            it->setCustomStyle({
                                FontSize { 8_pt },
                                Margin { 0, 0, 4_dp },
                                ATextAlign::CENTER,
                                TextColor { 0x444444_rgb },
                            });
                        },
                        AText::fromString("Open source free Minecraft launcher that's designed to be independent of "
                                          "any third-party commercial organizations like Minecraft servers, hostings, "
                                          "launcher-based projects, even official Minecraft. Do not allow to feed "
                                          "yourself with terrible bullshit filled with annoying ads!"),
                        AText::fromString("The launcher is distributed under GNU General Public License (v3) and "
                                          "powered by open source software: zlib, minizip, Freetype, Boost, AUI."),
                        AText::fromString("Contributors: Alex2772"),
                        Horizontal {
                            SpacerExpanding{},
                            _new<AButton>("View GNU General Public License v3").connect(&AButton::clicked, this, [] {
                                APlatform::openUrl("https://www.gnu.org/licenses/gpl-3.0.html");
                            }),
                            _new<AButton>("Check for updates..."),
                        }
                    }, "About"
                );
            },
            SpacerExpanding{},
            Horizontal {
                mResetButton = _new<AButton>("Reset to defaults").connect(&AButton::clicked, this, [this, binding] {
                    Settings::reset();
                    binding->setModel(Settings::inst());
                    mResetButton->setDisabled();
                }),
                SpacerExpanding{},
                _new<AButton>("OK").connect(&AButton::clicked, this, [this, binding] {
                    Settings::inst() = binding->getModel();
                    auto s = Settings::inst();
                    Settings::save();
                    close();
                }) let { it->setDefault(); },
                _new<AButton>("Cancel").connect(&AView::clicked, this, [this, binding] {
                    if (binding->getEditableModel() != Settings::inst()) {
                        auto result = AMessageBox::show(this,
                                                        "Unsaved settings",
                                                        "You have an unsaved changes. Do you wish to continue?",
                                                        AMessageBox::Icon::WARNING,
                                                        AMessageBox::Button::YES_NO);
                        if (result == AMessageBox::ResultButton::YES) {
                            close();
                        }
                    } else {
                        close();
                    }
                }),
            }
        }
    );

    connect(binding->modelChanged, [&, binding] {
        mResetButton->enable();
    });
    connect(fullscreenCheckbox->checked, [this, resolutionView](bool g) { resolutionView->setVisibility(g ? Visibility::VISIBLE : Visibility::GONE); updateLayout(); });
}

void LauncherSettingsWindow::clearGameDir() {
    if (AMessageBox::show(this,
                          "Clear game dir?",
                          "Your saves, settings, mods, configs will be permanently removed!",
                          AMessageBox::Icon::WARNING, AMessageBox::Button::YES_NO) == AMessageBox::ResultButton::YES) {

        mClearGameDirSpinner->setVisibility(Visibility::VISIBLE);
        mClearGameDirButton->setVisibility(Visibility::GONE);

        mTask = async {
            Settings::inst().gameDir.removeFileRecursive().makeDirs();
            ui_thread {
                mClearGameDirSpinner->setVisibility(Visibility::GONE);
                mClearGameDirButton->setVisibility(Visibility::VISIBLE);
            };
        };
    }
}
