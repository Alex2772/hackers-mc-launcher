//
// Created by Alex2772 on 9/10/2021.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/View/AText.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/AButton.h>
#include "GameProfilesView.h"
#include "Window/ImportVersionWindow.h"

GameProfilesView::GameProfilesView(const _<IListModel<GameProfile>>& model):
    mModel(model)
{
    setContents(Vertical {
        ui_for(profile, model, AWordWrappingLayout) {
            auto item = Vertical {
                    Stacked{
                        _new<ADrawableView>(":profile_icons/default.png"_url),
                    } << ".version_item_wrap",
                    AText::fromString(profile.getName(), { WordBreak::BREAK_ALL }),
            } << ".version_item";
            auto it = item.get();

            connect(item->clicked, item, [this, it, index] {
                *it << ".version_item_selected";
                onSelectedProfile(index);
                redraw();
                connect(selectionChanged, it, [this, it] {
                    it->removeAssName(".version_item_selected");
                    AObject::disconnect();
                });
            });
            return item;
        }
    });
}

void GameProfilesView::onSelectedProfile(size_t profileIndex) {
    mProfileIndex = profileIndex;
    emit selectionChanged;
}
