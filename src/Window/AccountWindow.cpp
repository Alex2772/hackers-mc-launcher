//
// Created by alex2772 on 2/17/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/AButton.h>
#include <Repository/UsersRepository.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Traits/strings.h>
#include "AccountWindow.h"

AccountWindow::AccountWindow(User* user):
    AWindow(user == nullptr ? "New user" : "Edit user", 200, 100, AWindow::current(), WS_DIALOG)
{

    if (user) {
        mBinding->setModel(*user);
    }

    setContents(
        Vertical {
            _form({
                {"Username"_as, mUsername = _new<ATextField>() let { it->focus(); it && mBinding(&User::username); }},
                {{},            _new<ACheckBox>("Online account on minecraft.net") && mBinding(&User::isOnlineAccount)},
                {"Password"_as, _new<ATextField>() && mBinding(&User::token) && mBinding(&User::isOnlineAccount, &ATextField::setEnabled) },
                }),
            Horizontal {
                user ? (_new<AButton>("Delete user").connect(&AView::clicked, this, [&, user] {
                    if (AMessageBox::show(this,
                                      "Delete user",
                                      "Do you really want to delete user \"{}\"? "
                                      "The operation is unrecoverable!"_as.format(user->username),
                                      AMessageBox::Icon::INFO,
                                      AMessageBox::Button::YES_NO) == AMessageBox::ResultButton::YES) {
                        close();
                        emit deleteUser();
                    }
                })) : nullptr,
                _new<ASpacer>(),
                (user ?
                    _new<AButton>("OK").connect(&AView::clicked, this, [&, user] {
                        *user = mBinding->getModel();
                        emit finished();
                        close();
                    })
                        :
                    _new<AButton>("Create").connect(&AView::clicked, this, [&] {
                        UsersRepository::inst().addUser(mBinding->getModel());
                        close();
                    })) let { it->setDefault(); },
                _new<AButton>("Cancel").connect(&AView::clicked, me::close)
            }
        }
    );
    pack();
}
