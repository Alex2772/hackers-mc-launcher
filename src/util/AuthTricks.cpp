//
// Created by Alex2772 on 7/11/2022.
//

#include "AuthTricks.h"
#include <random>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Curl/ACurl.h>

AString auth_tricks::clientId(const VariableHelper::Context& c) {
    std::default_random_engine re(std::hash<AString>()(c.user->username));
    std::uniform_int_distribution<int> distribution(0, 255);

    AUuid uuid;

    for (auto& d : uuid.data()) {
        d = distribution(re);
    }

    return AByteBufferView(uuid.toString().toStdString()).toBase64String();
}

AString auth_tricks::xuid(const VariableHelper::Context& c) {
    char buf[32];
    std::sprintf(buf, "%016llu", std::hash<AString>()(c.user->username));
    return buf;
}

AUuid auth_tricks::usernameToUuid(const AString& username) {
    auto response = AJson::fromBuffer(ACurl::Builder("https://api.mojang.com/users/profiles/minecraft/" + username).runBlocking().body);
    return AUuid::fromString(response["id"].asString());
}
