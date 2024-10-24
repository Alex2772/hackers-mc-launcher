#pragma once


#include <AUI/View/ADropdownList.h>

class AccountsComboBox: public ADropdownList {
public:
    AccountsComboBox(const _<IListModel<AString>>& model);

protected:
    void onComboBoxWindowCreated() override;

    void updateText() override;
};


