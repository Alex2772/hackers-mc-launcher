//
// Created by alex2772 on 4/3/25.
//
#include <range/v3/all.hpp>

#include "Account.h"
#include <AUI/Util/kAUI.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Json/AJson.h>
#include <AUI/Util/ARandom.h>

static constexpr auto LOG_TAG = "Account";


AFuture<> Account::populateUuid() {
    auto username = *this->username;
    return async {
        if (ranges::any_of(uuid.data(), [](auto b) { return b != 0; })) {
            return;
        }

        try {
            auto json = AJson::fromBuffer(ACurl::Builder("https://api.mojang.com/users/profiles/minecraft/{}"_format(username)).runAsync()->body);
            if (json.contains("id")) {
                uuid = AUuid::fromString(json["id"].asString());
                ALogger::err(LOG_TAG) << "Obtained official UUID of user " << username << ": " << uuid.toString();
                return;
            }
        } catch (const AException& e) {
            ALogger::err(LOG_TAG) << "Can't obtain official UUID of user " << username << ": " << e;
        }
        ARandom r;
        uuid = r.nextUuid();
    };
}
