//
// Created by Alex2772 on 9/10/2021.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AImageView.h>
#include "GameProfilesView.h"

GameProfilesView::GameProfilesView(const _<IListModel<GameProfile>>& model):
    mModel(model)
{
    setContents(Vertical {
        ui_for(profile, model, AWordWrappingLayout) {
            auto item = Vertical {
                    Stacked{
                        _new<AImageView>(":profile_icons/default.png"_url),
                    } << ".version_item_wrap",
                    _new<ALabel>(profile.getName()),
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
