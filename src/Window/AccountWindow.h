#pragma once

#include <Model/Account.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/View/ATextField.h>
#include <AUI/Util/ADataBinding.h>
#include "Model/State.h"

class AccountWindow: public AWindow {
public:
    AccountWindow(State& state, _<Account> user);

signals:
    emits<> positiveAction;
    emits<> deleteUser;

private:
    State& mState;
    Account mAccount;
};

