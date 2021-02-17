//
// Created by alex2772 on 2/17/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/AButton.h>
#include "AccountWindow.h"

AccountWindow::AccountWindow(User* user):
    AWindow(user == nullptr ? "New user" : "Edit user", 200, 100, AWindow::current(), WS_DIALOG)
{
    setContents(
        Vertical {
            _form({
                {"Username"_as, _new<ATextField>() let { it->focus(); }},
                {{},            _new<ACheckBox>("Online account on minecraft.net")},
                {"Password"_as, _new<ATextField>()},
                }),
            Horizontal {
                _new<ASpacer>(),
                (user ? _new<AButton>("OK") : _new<AButton>("Create")) let { it->setDefault(); },
                _new<AButton>("Cancel").connect(&AView::clicked, me::close)
            }
        }
    );
    pack();
}
