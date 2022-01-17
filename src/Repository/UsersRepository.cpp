//
// Created by alex2 on 17.02.2021.
//

#include <AUI/Json/AJson.h>
#include <Util.h>
#include <AUI/Logging/ALogger.h>
#include <Source/LegacyLauncherJsonSource.h>
#include "UsersRepository.h"

UsersRepository::UsersRepository():
    mModel(_new<AListModel<Account>>())
{
    AObject::connect(mModel->dataChanged, mModel, []{LegacyLauncherJsonSource::save();});
    AObject::connect(mModel->dataRemoved, mModel, []{LegacyLauncherJsonSource::save();});
}

UsersRepository::~UsersRepository() {
}

UsersRepository& UsersRepository::inst() {
    static UsersRepository ur;
    return ur;
}

void UsersRepository::addUser(const Account& user) {
    mModel << user;
    LegacyLauncherJsonSource::save();
}
