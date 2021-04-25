#pragma once

#include <AUI/Common/AVariant.h>
#include <Model/GameProfile.h>
#include <Model/User.h>

class VariableHelper {
public:
    struct Context {
        const User* user = nullptr;
        const GameProfile* profile = nullptr;
    };

    static AVariant getVariableValue(const Context& c, const AString& name);
    static bool checkConditions(const Context& c, const AVector<std::pair<AString, AVariant>>& conditions);
    static AString parseVariables(const Context& c, const AString& s);
};

