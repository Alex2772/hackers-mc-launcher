#pragma once

#include <Model/Account.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/View/ATextField.h>
#include <AUI/Util/ADataBinding.h>

class AccountWindow: public AWindow {
private:
    _<ATextField> mUsername;


    _<ADataBinding<Account>> mBinding = _new<ADataBinding<Account>>();

public:

    AccountWindow(Account* user);

signals:
    emits<> finished;
    emits<> deleteUser;
};

