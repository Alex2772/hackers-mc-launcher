//
// Created by alex2772 on 2/17/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/AButton.h>
#include <Repository/UsersRepository.h>
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
                {"Username"_as, mUsername = _new<ATextField>() let { it->focus(); it && mBinding->link(&User::username, &ATextField::textChanging, &ATextField::setText); }},
                {{},            _new<ACheckBox>("Online account on minecraft.net")},
                {"Password"_as, _new<ATextField>() let { it && mBinding->link(&User::token, &ATextField::textChanging, &ATextField::setText); }},
                }),
            Horizontal {
                _new<ASpacer>(),
                (user ?
                    _new<AButton>("OK").connect(&AView::clicked, this, [&, user] {
                        *user = mBinding->getModel();
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
