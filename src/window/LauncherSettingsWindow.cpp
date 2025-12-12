//
// Created by alex2 on 15.04.2021.
//

#include "LauncherSettingsWindow.h"
#include "MainWindow.h"
#include "MyUpdater.h"
#include "AUI/Reflect/for_each_field.h"
#include "AUI/View/AGroupBox.h"
#include "AUI/View/AProgressBar.h"
#include "AUI/View/Dynamic.h"

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATabView.h>
#include <AUI/View/AText.h>
#include <AUI/View/APathChooserView.h>
#include <AUI/Json/AJson.h>
#include <model/Settings.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/ANumberPicker.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/ASS/ASS.h>
#include <AUI/View/ASpinnerV2.h>
#include <AUI/Platform/APlatform.h>
#include <AUI/View/ASpacerFixed.h>

using namespace ass;
using namespace declarative;

static _<AView> updaterView() {
    if (std::any_cast<AUpdater::StatusIdle>(&*MyUpdater::inst().status)) {
        return Horizontal {
            Label { "No updates found. " },
            SpacerFixed { 4_dp },
            Button {
                .content = Label { "Check for updates..." },
                .onClick = [] { MyUpdater::inst().checkForUpdates(); }
            },
        };
    }

    if (std::any_cast<AUpdater::StatusCheckingForUpdates>(&*MyUpdater::inst().status)) {
        return Label { "Checking for updates..." };
    }

    if (auto downloading = std::any_cast<AUpdater::StatusDownloading>(&*MyUpdater::inst().status)) {
        return Vertical {
            Label { "Downloading..." },
            _new<AProgressBar>() & downloading->progress,
        };
    }

    if (auto downloading = std::any_cast<AUpdater::StatusWaitingForApplyAndRestart>(&*MyUpdater::inst().status)) {
        return Button {
            .content = Label { "Restart to apply updates" },
            .onClick = [] { MyUpdater::inst().applyUpdateAndRestart(); }
        };
    }

    return nullptr;
}

LauncherSettingsWindow::LauncherSettingsWindow() :
        AWindow("Settings", 400_dp, 400_dp, &MainWindow::inst(), WindowStyle::MODAL | WindowStyle::NO_RESIZE) {

    aui::reflect::for_each_field_value(mSettings, [this](auto&& field) {
        AObject::connect(field.changed, [this] { mDirty = true; });
    });

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
                                } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
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
                                        LayoutSpacing { 8_dp },
                                    } AUI_LET {
                                        AObject::connect(mSettings.isFullscreen, [it](bool v) {
                                            it->setEnabled(!v);
                                        });
                                    },
                                } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
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
                        }) AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
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
                        _new<ALabel>("Version " AUI_PP_STRINGIZE(AUI_CMAKE_PROJECT_VERSION)) AUI_LET {
                            it->setCustomStyle({
                                FontSize { 8_pt },
                                //Margin { 0, 0, 4_dp },
                                ATextAlign::CENTER,
                                TextColor { 0x444444_rgb },
                            });
                        },
                        _new<ALabel>("Built " __DATE__ " " __TIME__) AUI_LET {
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
                        } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },

                        GroupBox {
                            CheckBox {
                                .checked = AUI_REACT(mSettings.autoUpdate),
                                .onCheckedChange = [this](bool v) { mSettings.autoUpdate = v; },
                                .content = Label { "Auto update" },
                            },
                            Vertical {
                                experimental::Dynamic {
                                    .content = AUI_REACT(updaterView()),
                                },
                            },
                        } AUI_LET { it->setVisible(MyUpdater::isAvailable()); },
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
                    if (mDirty) {
                        auto result = AMessageBox::show(this,
                                                        "Unsaved settings",
                                                        "You have an unsaved changes. Do you wish to continue?",
                                                        AMessageBox::Icon::WARNING,
                                                        AMessageBox::Button::YES_NO);
                        if (result != AMessageBox::ResultButton::YES) {
                            return;
                        }
                    }
                    close();
                }),
            } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
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
