#pragma once

#include <AUI/Reflect/AEnumerate.h>

enum class Modloader {
    NONE,
    FORGE = 1 << 0,
    NEOFORGE = 1 << 1,
    FABRIC = 1 << 2,
    SPONGE = 1 << 3,
};

AUI_ENUM_VALUES(Modloader, Modloader::NONE, Modloader::FORGE, Modloader::NEOFORGE, Modloader::FABRIC)
