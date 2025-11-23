//
// Created by alex2772 on 2/17/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/AButton.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Traits/strings.h>
#include <AUI/Util/ARandom.h>
#include "AccountWindow.h"

AccountWindow::AccountWindow(State& state, _<Account> user):
    AWindow(user == nullptr ? "New account" : "Modify account", 200, 100, dynamic_cast<AWindow*>(AWindow::current()), WindowStyle::MODAL),
    mState(state)
{
    using namespace declarative;

    setContents(
        Vertical {
            _form({
                {"Username:"_as, _new<ATextField>() AUI_LET { it->focus(); it && mAccount.username; }},
//                {{},            CheckBoxWrapper { Label {"Online account on minecraft.net" } } && mBinding(&Account::isOnlineAccount)},
//                {"Password:"_as, _new<ATextField>() && mBinding(&Account::token) && mBinding(&Account::isOnlineAccount, &ATextField::setEnabled) },
                }),
            Horizontal {
                user ? (_new<AButton>("Delete account").connect(&AView::clicked, this, [&, user] {
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
                SpacerExpanding{},
                  _new<AButton>("OK").connect(&AView::clicked, this, [&, user] {
                      if (!UsernameValidator()(mAccount.username)) {
                          AMessageBox::show(this,
                                            "Username is invalid",
                                            "Please use latin or numeric characters",
                                            AMessageBox::Icon::INFO,
                                            AMessageBox::Button::OK);
                          return;
                      }
                      if (user) {
                        *user = std::move(mAccount);
                      } else {
                          *mState.accounts.current = std::move(mAccount);
                      }
                      emit positiveAction();
                      close();
                  }),
                _new<AButton>("Cancel").connect(&AView::clicked, me::close)
            }
        }
    );
    pack();
}
