//
// Created by Alex2772 on 9/10/2021.
//

#include <range/v3/all.hpp>
#include <AUI/View/AForEachUI.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/View/AText.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/AButton.h>
#include "GameProfilesView.h"
#include "AUI/Action/AMenu.h"
#include "Window/ImportVersionWindow.h"

GameProfilesView::GameProfilesView(State::Profiles& state): mState(state)
{
    setContents(Vertical {
        AUI_DECLARATIVE_FOR(profile, *mState.list, AWordWrappingLayout) {
            auto item = Vertical {
                    Stacked{
                        _new<ADrawableView>(":profile_icons/default.png"_url),
                    } << ".version_item_wrap",
                    AText::fromString(profile->name, { WordBreak::BREAK_ALL }),
            } << ".version_item";
            auto it = item.get();

            connect(item->clicked, [this, profile] {
                mState.selected = profile;
            });
            connect(mState.selected, [item, profile](const _<GameProfile>& selected) {
                item->setAssName(".version_item_selected", profile == selected);
            });
            connect(item->clickedRightOrLongPressed, item, [this, profile] {
                AMenu::show(AMenuModel {
                    AMenuItem {
                        .name = "Remove",
                        .onAction = [this, profile] {
                            mState.list.writeScope()->removeFirst(profile);
                        },
                    },
                });
            });
            return item;
        }
    });
}
