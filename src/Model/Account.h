//
// Created by alex2772 on 2/17/21.
//

#pragma once

#include <AUI/Common/AString.h>
#include <AUI/Common/AUuid.h>


struct Account {
    AUuid uuid;
    AString username;
    bool isOnlineAccount = false;
    AString token;
};