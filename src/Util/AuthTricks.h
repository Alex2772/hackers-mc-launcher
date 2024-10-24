#pragma once


#include "VariableHelper.h"

namespace auth_tricks {
    AString clientId(const VariableHelper::Context& c);
    AString xuid(const VariableHelper::Context& c);
    AUuid usernameToUuid(const AString& username);
};


