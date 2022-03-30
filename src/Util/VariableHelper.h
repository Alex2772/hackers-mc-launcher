#pragma once

#include <Model/GameProfile.h>
#include <Model/Account.h>

class VariableHelper {
public:
    struct Context {
        const Account* user = nullptr;
        const GameProfile* profile = nullptr;
    };

    static AString getVariableValue(const Context& c, const AString& name);
    static bool checkRules(const Context& c, const Rules& rules);
    static AString parseVariables(const Context& c, const AString& s);
};

