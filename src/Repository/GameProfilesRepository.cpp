//
// Created by alex2 on 15.04.2021.
//

#include "GameProfilesRepository.h"

GameProfilesRepository::GameProfilesRepository():
    mModel(_new<AListModel<GameProfile>>())
{

}

void GameProfilesRepository::addGameProfile(const GameProfile& user) {
    mModel << user;
}

GameProfilesRepository& GameProfilesRepository::inst() {
    static GameProfilesRepository r;
    return r;
}
