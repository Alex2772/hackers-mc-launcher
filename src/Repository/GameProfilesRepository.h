//
// Created by alex2 on 15.04.2021.
//

#pragma once


#include <Model/GameProfile.h>

class GameProfilesRepository {
private:
    _<AListModel<GameProfile>> mModel;

public:
    GameProfilesRepository();

    [[nodiscard]] const _<AListModel<GameProfile>>& getModel() const {
        return mModel;
    }

    static GameProfilesRepository& inst();

    void addGameProfile(const GameProfile& user);
};

