//
// Created by alex2772 on 4/20/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATabView.h>
#include <AUI/Util/ADataBinding.h>
#include <AUI/View/ATextField.h>
#include <AUI/Platform/AMessageBox.h>
#include "GameProfileWindow.h"
#include "MainWindow.h"
#include "Window/GameProfilePages/DetailsPage.h"

using namespace declarative;

GameProfileWindow::GameProfileWindow(_<GameProfile> targetGameProfile):
    AWindow("Game profile", 500_dp, 300_dp, &MainWindow::inst(), WindowStyle::MODAL),
    mProfile(*targetGameProfile),
    mTargetGameProfile(targetGameProfile),
    mPageRoot(_new<AViewContainer>())
{
    using namespace declarative;


    setContents(
        Vertical {
            Horizontal::Expanding {
                Vertical {
                    tab("Version", _new<DetailsPage>(mProfile)) let { mPage = it; },
                    tab("Mods", Centered { Label { "TODO. Sorry!" } }),
                    tab("Notes", Centered { Label { "TODO. Sorry!" } }),
                    tab("Servers", Centered { Label { "TODO. Sorry!" } }),
                    tab("Resource packs", Centered { Label { "TODO. Sorry!" } }),
                    tab("Shader packs", Centered { Label { "TODO. Sorry!" } }),
                    tab("Worlds", Centered { Label { "TODO. Sorry!" } }),
                } with_style { Padding{1_px}, LayoutSpacing { 1_px } },
                Vertical::Expanding {
                    mPageRoot,
                },
            },
            Horizontal {
                mResetButton = _new<AButton>("Reset").connect(&AButton::clicked, this, [this] {
                    mProfile = *mTargetGameProfile;
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
                SpacerExpanding{},
                _new<AButton>("OK").connect(&AButton::clicked, this, [this] {
                    *mTargetGameProfile = std::move(mProfile);
                }) let { it->setDefault(); },
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
            }
        }
    );
}

_<AView> GameProfileWindow::tab(AString name, _<AView> contents) {
    return Label { std::move(name) } with_style { Padding { 8_dp, 16_dp }, Margin { 0 } } let {
        connect(it->clicked, [this, it] {
            mPage = it;
        });
        connect(mPage, [this, it, contents = std::move(contents)] {
            it->setAssName(".background_accent", mPage == it);
            if (mPage == it) {
                ALayoutInflater::inflate(mPageRoot, contents);
            }
        });
    };
}
