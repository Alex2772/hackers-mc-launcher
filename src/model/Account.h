//
// Created by alex2772 on 2/17/21.
//

#pragma once

#include <AUI/Common/AString.h>
#include <AUI/Common/AUuid.h>
#include <AUI/Common/AProperty.h>
#include <AUI/Validation/AValidator.h>
#include <AUI/Thread/AFuture.h>


struct Account {
    AUuid uuid;
    AProperty<AString> username;
    AProperty<bool> isOnlineAccount = false;
    AProperty<AString> token;

    AFuture<> populateUuid();
};


struct UsernameValidator {
    bool operator()(const AString& s) {
        using namespace aui::valid;
        return string::latin_numeric()(s) && in_range<4, 32>()(s.length());
    }
};