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

    const _<AListModel<User>>& getModel() const {
        return mModel;
    }

    static UsersRepository& inst();

    void addUser(const User& user);
};


