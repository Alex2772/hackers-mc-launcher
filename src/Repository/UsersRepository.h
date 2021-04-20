//
// Created by alex2 on 17.02.2021.
//

#pragma once

#include <AUI/Model/AListModel.h>
#include <Model/User.h>

class UsersRepository {
private:
    _<AListModel<User>> mModel;

public:
    UsersRepository();
    ~UsersRepository();

    _<AListModel<User>>& getModel() {
        return mModel;
    }

    static UsersRepository& inst();

    void addUser(const User& user);
};


