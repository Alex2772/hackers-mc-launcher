//
// Created by alex2 on 15.04.2021.
//

#include <AUI/Json/AJson.h>
#include <Source/LegacyLauncherJsonSource.h>
#include "GameProfilesRepository.h"

GameProfilesRepository::GameProfilesRepository():
    mModel(_new<AListModel<GameProfile>>())
{

    AObject::connect(mModel->dataChanged, mModel, []{LegacyLauncherJsonSource::save();});
    AObject::connect(mModel->dataRemoved, mModel, []{LegacyLauncherJsonSource::save();});
}

void GameProfilesRepository::addGameProfile(const GameProfile& user) {
    mModel << user;
    LegacyLauncherJsonSource::save();
}

GameProfilesRepository& GameProfilesRepository::inst() {
    static GameProfilesRepository r;
    return r;
}

