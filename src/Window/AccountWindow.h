#pragma once

#include <Model/User.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/View/ATextField.h>
#include <AUI/Util/ADataBinding.h>

class AccountWindow: public AWindow {
private:
    _<ATextField> mUsername;


    _<ADataBinding<User>> mBinding = _new<ADataBinding<User>>();

public:

    AccountWindow(User* user);

signals:
    emits<> finished;
    emits<> deleteUser;
};
