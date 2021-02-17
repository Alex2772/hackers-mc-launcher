//
// Created by alex2772 on 2/17/21.
//

#pragma once

#include <AUI/Common/AString.h>


struct User {
    AString username;
    bool isOnlineAccount = false;
    AString token;
};