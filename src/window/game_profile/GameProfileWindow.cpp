//
// Created by alex2772 on 4/20/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATabView.h>
#include <AUI/Util/ADataBinding.h>
#include <AUI/View/ATextField.h>
#include <AUI/Platform/AMessageBox.h>
#include "GameProfileWindow.h"

#include "ModsPage.h"
#include "../GameProfileExportWindow.h"
#include "../MainWindow.h"
#include "AUI/View/ASpacerFixed.h"
#include "window/game_profile/DetailsPage.h"

using namespace declarative;

GameProfileWindow::GameProfileWindow(_<GameProfile> targetGameProfile):
    AWindow("Game profile", 500_dp, 300_dp, &MainWindow::inst(), WindowStyle::MODAL),
    mProfile(_new<GameProfile>(*targetGameProfile)),
    mTargetGameProfile(targetGameProfile),
    mPageRoot(_new<AViewContainer>())
{
    using namespace declarative;
    mPageRoot->setExpanding();


    setContents(
        Vertical {
            Horizontal::Expanding {
                Vertical {
                    tab("Version", [&] { return game_profile::detailsPage(mProfile); }) AUI_LET { mPage = it; },
                    tab("Mods", [&] { return game_profile::modsPage(mProfile); }),
                    tab("Notes", [] { return Centered { Label { "TODO. Sorry!" } }; }),
                    tab("Servers", [] { return Centered { Label { "TODO. Sorry!" } }; }),
                    tab("Resource packs", [] { return Centered { Label { "TODO. Sorry!" } }; }),
                    tab("Shader packs", [] { return Centered { Label { "TODO. Sorry!" } }; }),
                    tab("Worlds", [] { return Centered { Label { "TODO. Sorry!" } }; }),
                } AUI_OVERRIDE_STYLE { Padding{1_px}, LayoutSpacing { 1_px } },
                SpacerFixed { 8_dp },
                Vertical::Expanding {
                    mPageRoot,
                },
            },
            SpacerFixed { 8_dp },
            Horizontal {
                mResetButton = _new<AButton>("Reset").connect(&AButton::clicked, this, [this] {
                    *mProfile = *mTargetGameProfile;
                    mResetButton->setDisabled();
                }),
                _new<AButton>("Delete").connect(&AButton::clicked, this, [this] {
                    auto result = AMessageBox::show(this,
                                                    "Delete profile",
                                                    "This operation is unrecoverable. Do you wish to continue?",
                                                    AMessageBox::Icon::WARNING,
                                                    AMessageBox::Button::YES_NO);
                    if (result == AMessageBox::ResultButton::YES) {
                        // TODO
                        close();
                    }
                }),
                Button {
                    .content = Label { "Export..." },
                    .onClick = [=] {
                        _new<GameProfileExportWindow>(this, targetGameProfile)->show();
                    },
                },
                SpacerExpanding{},
                _new<AButton>("OK").connect(&AButton::clicked, this, [this] {
                    *mTargetGameProfile = std::move(*mProfile);
                    mTargetGameProfile->save();
                    close();
                }) AUI_LET { it->setDefault(); },
                _new<AButton>("Cancel").connect(&AView::clicked, this, [this] { /*
                    if (binding->getEditableModel() != Settings::inst()) {
                        auto result = AMessageBox::show(this,
                                                        "Unsaved settings",
                                                        "You have an unsaved changes. Do you wish to continue?",
                                                        AMessageBox::Icon::WARNING,
                                                        AMessageBox::Button::YES_NO);
                        if (result == AMessageBox::ResultButton::YES) {
                            close();
                        }
                    } else */ {
                        close();
                    }
                }),
            } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
        }
    );
}

_<AView> GameProfileWindow::tab(AString name, std::function<_<AView>()> contents) {
    return Label { std::move(name) } AUI_OVERRIDE_STYLE { Padding { 8_dp, 16_dp }, Margin { 0 } } AUI_LET {
        connect(it->clicked, [this, it] {
            mPage = it;
        });
        connect(mPage, [this, it, contents = aui::lazy<_<AView>>(std::move(contents))]() mutable  {
            it->setAssName(".background_accent", mPage == it);
            if (mPage == it) {
                ALayoutInflater::inflate(mPageRoot, *contents);
            }
        });
    };
}
