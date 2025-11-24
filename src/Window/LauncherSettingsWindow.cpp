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
#include <AUI/View/ASpinnerV2.h>
#include <AUI/Platform/APlatform.h>

using namespace ass;

LauncherSettingsWindow::LauncherSettingsWindow() :
        AWindow("Settings", 400_dp, 400_dp, &MainWindow::inst(), WindowStyle::MODAL | WindowStyle::NO_RESIZE) {
    using namespace declarative;

    setContents(
        Vertical {
            _new<ATabView>() AUI_LET {
                // GAME TAB ============================================================================================
                it->addTab(
                    Vertical{
                        _form({
                            {
                                "Game dir:"_as,
                                Horizontal {
                                    _new<APathChooserView>() AUI_LET {
                                        it->setExpanding();
                                        it && mSettings.gameDir;
                                    },
                                    mClearGameDirButton = _new<AButton>("Clear game dir").connect(&AView::clicked, me::clearGameDir),
                                    mClearGameDirSpinner = _new<ASpinnerV2>() AUI_LET {
                                        it->setVisibility(Visibility::GONE);
                                    },
                                } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
                            },
                            {
                                "Display:"_as,
                                Horizontal {
                                    CheckBox {
                                        .checked = AUI_REACT(mSettings.isFullscreen),
                                        .onCheckedChange = [this](bool g) {
                                            mSettings.isFullscreen = g;
                                        },
                                        .content = Label { "Fullscreen" },
                                    },
                                    Horizontal {
                                        _new<ANumberPicker>() && mSettings.width,
                                        _new<ALabel>("x"),
                                        _new<ANumberPicker>() && mSettings.height,
                                    } AUI_OVERRIDE_STYLE {
                                        LayoutSpacing { 4_dp },
                                    } AUI_LET {
                                        AObject::connect(mSettings.isFullscreen, [it](bool v) {
                                            it->setEnabled(!v);
                                        });
                                    },
                                } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
                            },
                            {
                                "Show game console:"_as,
                                Vertical {
                                    RadioButton {
                                        .checked = AUI_REACT(mSettings.showConsoleOnPlay == false),
                                        .onClick = [this] { mSettings.showConsoleOnPlay = false; },
                                        .content = Label { "When game crashes" },
                                    },
                                    RadioButton {
                                        .checked = AUI_REACT(mSettings.showConsoleOnPlay == true),
                                        .onClick = [this] { mSettings.showConsoleOnPlay = true; },
                                        .content = Label { "Always" },
                                    },
                                },
                            },
                        }) AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
                    }, "Game"
                );

                // ABOUT TAB ===========================================================================================
                it->addTab(
                    Vertical{
                        _new<ALabel>("Hacker's Minecraft Launcher") AUI_LET {
                            it->setCustomStyle({
                                FontSize { 19_pt },
                                Margin { 8_dp, 0, 4_dp },
                                ATextAlign::CENTER,
                            });
                        },
                        _new<ALabel>("Version " HACKERS_MC_VERSION) AUI_LET {
                            it->setCustomStyle({
                                FontSize { 8_pt },
                                //Margin { 0, 0, 4_dp },
                                ATextAlign::CENTER,
                                TextColor { 0x444444_rgb },
                            });
                        },
                        _new<ALabel>("Distributed under GNU General Public License v3") AUI_LET {
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
                            // _new<AButton>("Check for updates..."),
                        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
                    }, "About"
                );
            },
            SpacerExpanding{},
            Horizontal {
                mResetButton = _new<AButton>("Reset to defaults").connect(&AButton::clicked, this, [this] {
                    Settings::reset();
                    Settings::inst() = Settings{};
                    mResetButton->setDisabled();
                }),
                SpacerExpanding{},
                _new<AButton>("OK").connect(&AButton::clicked, this, [this] {
                    Settings::inst() = mSettings;
                    auto s = Settings::inst();
                    Settings::save();
                    close();
                }) AUI_LET { it->setDefault(); },
                _new<AButton>("Cancel").connect(&AView::clicked, this, [this] {
                    if (Settings::inst() != mSettings) {
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
            } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
        }
    );
}

void LauncherSettingsWindow::clearGameDir() {
    if (AMessageBox::show(this,
                          "Clear game dir?",
                          "Your saves, settings, mods, configs will be permanently removed!",
                          AMessageBox::Icon::WARNING, AMessageBox::Button::YES_NO) == AMessageBox::ResultButton::YES) {

        mClearGameDirSpinner->setVisibility(Visibility::VISIBLE);
        mClearGameDirButton->setVisibility(Visibility::GONE);

        mTask = AUI_THREADPOOL {
            mSettings.gameDir->removeFileRecursive().makeDirs();
            AUI_UI_THREAD {
                mClearGameDirSpinner->setVisibility(Visibility::GONE);
                mClearGameDirButton->setVisibility(Visibility::VISIBLE);
            };
        };
    }
}
