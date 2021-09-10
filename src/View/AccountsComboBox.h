#pragma once


#include <AUI/View/AComboBox.h>

class AccountsComboBox: public AComboBox {
public:
    AccountsComboBox(const _<IListModel<AString>>& model);

protected:
    void onComboBoxWindowCreated() override;

    void updateText() override;
};


