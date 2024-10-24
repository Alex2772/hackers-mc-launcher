//
// Created by alex2 on 15.04.2021.
//

#pragma once


#include <AUI/Model/AListModel.h>
#include <Model/GameProfile.h>

class GameProfilesRepository {
private:
    _<AListModel<GameProfile>> mModel;

    ASet<AUuid> mCurrentlyLoadedSetOfProfiles; // used for profile auto-reload feature

public:
    GameProfilesRepository();

    [[nodiscard]] _<AListModel<GameProfile>>& getModel() {
        return mModel;
    }

    static GameProfilesRepository& inst();

    void addGameProfile(const GameProfile& user);
    void removeGameProfile(const AUuid& uuid);

    ASet<AUuid>& getCurrentlyLoadedSetOfProfiles() {
        return mCurrentlyLoadedSetOfProfiles;
    }
};


