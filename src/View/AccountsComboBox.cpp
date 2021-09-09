//
// Created by Alex2772 on 9/9/2021.
//

#include "AccountsComboBox.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/View/ABasicListEditor.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <Window/MainWindow.h>
#include <Window/GameProfileWindow.h>
#include <Window/UserWindow.h>
#include <Repository/UsersRepository.h>

void AccountsComboBox::onComboBoxWindowCreated() {
    AComboBox::onComboBoxWindowCreated();
    getComboBoxWindow()->addView(Horizontal {
        _new<AButton>("Manage...")
                .connect(&AView::clicked, this, [this] {
                    destroyWindow();
                    ABasicListEditor::Builder(getModel())
                        .withNewButton([] {
                            _new<UserWindow>(nullptr)->show();
                        })
                        .withModifyButton([](const AModelIndex& index) {
                            Autumn::get<MainWindow>()->showUserConfigureDialogFor(index.getRow());
                        })
                        .buildWindow("Manage accounts", Autumn::get<MainWindow>().get())->show();
                }) let { it->setExpanding(); },
        _new<AButton>("New...").connect(&AView::clicked, this, [this] {
            destroyWindow();
            _new<UserWindow>(nullptr)->show();
        }) let { it->setExpanding(); },
    });
}

AccountsComboBox::AccountsComboBox(const _<IListModel<AString>>& model) : AComboBox(model) {
    if (getModel()->listSize() == 0) {
        setText("<no accounts>");
    }
}
