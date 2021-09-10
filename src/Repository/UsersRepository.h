//
// Created by alex2 on 17.02.2021.
//

#pragma once

#include <AUI/Model/AListModel.h>
#include <Model/Account.h>

class UsersRepository {
private:
    _<AListModel<Account>> mModel;

public:
    UsersRepository();
    ~UsersRepository();

    _<AListModel<Account>>& getModel() {
        return mModel;
    }

    static UsersRepository& inst();

    void addUser(const Account& user);
};


