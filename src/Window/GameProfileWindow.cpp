//
// Created by alex2772 on 4/20/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATabView.h>
#include <AUI/Util/ADataBinding.h>
#include <AUI/View/ATextField.h>
#include <AUI/Platform/AMessageBox.h>
#include <Repository/GameProfilesRepository.h>
#include "GameProfileWindow.h"
#include "MainWindow.h"

GameProfileWindow::GameProfileWindow(GameProfile& targetGameProfile):
    AWindow("Game profile", 500_dp, 300_dp, Autumn::get<MainWindow>().get(), WindowStyle::DIALOG),
    mTargetGameProfile(targetGameProfile)
{
    auto binding = _new<ADataBinding<GameProfile>>(targetGameProfile);
    setContents(
        Vertical {
            _new<ATabView>() let {
                // COMMON TAB ============================================================================================
                it->addTab(
                    Vertical {
                        _form({
                            {"Profile name:"_as, _new<ATextField>() && binding(&GameProfile::mName)},
                            {"Main class:"_as, _new<ATextField>() && binding(&GameProfile::mMainClass)},
                        }),
                    }, "Common"
                );

            },
            _new<ASpacer>(),
            Horizontal {
                mResetButton = _new<AButton>("Reset to defaults").connect(&AButton::clicked, this, [this, binding] {
                    // TODO
                    binding->setModel(mTargetGameProfile);
                    mResetButton->setDisabled();
                }),
                _new<AButton>("Delete").connect(&AButton::clicked, this, [this, binding] {
                    auto result = AMessageBox::show(this,
                                                    "Delete profile",
                                                    "This operation is unrecoverable. Do you wish to continue?",
                                                    AMessageBox::Icon::WARNING,
                                                    AMessageBox::Button::YES_NO);
                    if (result == AMessageBox::ResultButton::YES) {
                        GameProfilesRepository::inst().removeGameProfile(binding->getModel().getUuid());
                        close();
                    }
                }),
                _new<ASpacer>(),
                _new<AButton>("OK").connect(&AButton::clicked, this, [this, binding] {
                    mTargetGameProfile = binding->getModel();
                }) let { it->setDefault(); },
                _new<AButton>("Cancel").connect(&AView::clicked, this, [this, binding] { /*
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
