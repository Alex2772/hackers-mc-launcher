//
// Created by alex2 on 17.02.2021.
//

#include "UsersRepository.h"

UsersRepository::UsersRepository():
    mModel(_new<AListModel<User>>())
{

}

UsersRepository& UsersRepository::inst() {
    static UsersRepository ur;
    return ur;
}

void UsersRepository::addUser(const User& user) {
    mModel << user;
}
