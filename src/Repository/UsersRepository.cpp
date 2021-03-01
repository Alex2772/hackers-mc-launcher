//
// Created by alex2 on 17.02.2021.
//

#include <AUI/Json/AJson.h>
#include <AUI/IO/FileInputStream.h>
#include <Util.h>
#include <AUI/IO/FileOutputStream.h>
#include <AUI/Logging/ALogger.h>
#include "UsersRepository.h"

UsersRepository::UsersRepository():
    mModel(_new<AListModel<User>>())
{
    try {
        auto json = AJson::read(_new<FileInputStream>(Util::getSettingsDir().file("users.json")));
        for (auto& user : json.asArray()) {
            mModel << User {user["username"].asString()};
        }
    } catch (const AException& e) {
        ALogger::info(e.getMessage());
    }
}

UsersRepository::~UsersRepository() {
    try {
        AJsonArray json;
        for (User& user : *mModel) {
            AJsonObject u;
            u["username"] = user.username;
            json << u;
        }
        AJson::write(_new<FileOutputStream>(Util::getSettingsDir().file("users.json")), json);
    } catch (const AException& e) {
        ALogger::info(e.getMessage());
    }
}

UsersRepository& UsersRepository::inst() {
    static UsersRepository ur;
    return ur;
}

void UsersRepository::addUser(const User& user) {
    mModel << user;
}
