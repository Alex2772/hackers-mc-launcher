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
    AComboBox::onComboBoxWindowCreated();

    comboWindow()->addView(Horizontal {
        _new<AButton>("Manage...")
                .connect(&AView::clicked, this, [this] {
                    destroyWindow();
                    ABasicListEditor::Builder(getModel())
                        .withNewButton([] {
                            _new<AccountWindow>(nullptr)->show();
                        })
                        .withModifyButton([](const AModelIndex& index) {
                            Autumn::get<MainWindow>()->showUserConfigureDialogFor(index.getRow());
                        })
                        .buildWindow("Manage accounts", Autumn::get<MainWindow>().get())->show();
                }) let { it->setExpanding(); },
        _new<AButton>("New...").connect(&AView::clicked, this, [this] {
            destroyWindow();
            _new<AccountWindow>(nullptr)->show();
        }) let { it->setExpanding(); },
    });
    comboWindow()->updateLayout();
}

AccountsComboBox::AccountsComboBox(const _<IListModel<AString>>& model) : AComboBox(model) {

    updateText();
}

void AccountsComboBox::updateText() {
    AComboBox::updateText();

    if (getModel()->listSize() == 0) {
        setText("<no accounts>");
    }
}

