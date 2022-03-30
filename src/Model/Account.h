//
// Created by alex2772 on 2/17/21.
//

#pragma once

#include <AUI/Common/AString.h>
#include <AUI/Common/AUuid.h>
#include <AUI/Validation/AValidator.h>


struct Account {
    AUuid uuid;
    AString username;
    bool isOnlineAccount = false;
    AString token;
};


struct UsernameValidator {
    bool operator()(const AString& s) {
        using namespace aui::valid;
        return string::latin_numeric()(s) && in_range<4, 32>()(s.length());
    }
};