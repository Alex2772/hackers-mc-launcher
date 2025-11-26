#pragma once


#include "AUI/Model/IMutableListModel.h"
#include "Model/State.h"
#include <AUI/View/AViewContainer.h>
#include <Model/GameProfile.h>

class GameProfilesView: public AViewContainer {
public:
    GameProfilesView(State::Profiles& state);

    static _<AView> item(_<GameProfile> profile);

private:
    State::Profiles& mState;

};


