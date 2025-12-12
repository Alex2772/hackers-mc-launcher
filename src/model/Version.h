#pragma once


#include <AUI/Common/AString.h>
#include "GameProfile.h"
#include <AUI/Reflect/AEnumerate.h>

AUI_ENUM_FLAG(VersionType) {
    NONE = 0,
    RELEASE = 0b1,
    SNAPSHOT = 0b10,
    OLD_BETA = 0b100,
    OLD_ALPHA = 0b1000,
};

AUI_ENUM_VALUES(VersionType,
            VersionType::NONE,
            VersionType::RELEASE,
            VersionType::SNAPSHOT,
            VersionType::OLD_BETA,
            VersionType::OLD_ALPHA);

struct Version {
    AString id;
    AString url;
    VersionType type;

    static AVector<Version> fetchAll();

    GameProfile import() const;
};


