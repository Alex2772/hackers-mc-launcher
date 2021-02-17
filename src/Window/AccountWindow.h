#pragma once

#include <Model/User.h>
#include <AUI/Platform/AWindow.h>

class AccountWindow: public AWindow {
public:
    AccountWindow(User* user);
};

