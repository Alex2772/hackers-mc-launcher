//
// Created by Alex2772 on 9/9/2021.
//

#include "AccountsComboBox.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/View/ABasicListEditor.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <Window/MainWindow.h>
#include <Window/GameProfileWindow.h>
#include <Window/AccountWindow.h>
#include <Repository/UsersRepository.h>

void AccountsComboBox::onComboBoxWindowCreated() {
    ADropdownList::onComboBoxWindowCreated();

    comboWindow()->addView(Horizontal {
        _new<AButton>("Manage...")
                .connect(&AView::clicked, this, [this] {
                    destroyWindow();
                    ABasicListEditor::Builder(getModel())
                        .withNewButton([] {
                            _new<AccountWindow>(nullptr)->show();
                        })
                        .withModifyButton([](const AListModelIndex& index) {
                            MainWindow::inst().showUserConfigureDialogFor(index.getRow());
                        })
                        .buildWindow("Manage accounts", &MainWindow::inst())->show();
                }) let { it->setExpanding(); },
        _new<AButton>("New...").connect(&AView::clicked, this, [this] {
            destroyWindow();
            _new<AccountWindow>(nullptr)->show();
        }) let { it->setExpanding(); },
    });
}

AccountsComboBox::AccountsComboBox(const _<IListModel<AString>>& model) : ADropdownList(model) {

    updateText();
}

void AccountsComboBox::updateText() {
    ADropdownList::updateText();

    if (getModel()->listSize() == 0) {
        setText("<no accounts>");
    }
}

