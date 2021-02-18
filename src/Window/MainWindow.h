#pragma once

#include <AUI/Platform/AWindow.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>

class MainWindow: public AWindow {
private:
    _<AButton> mUserConfigureButton;
    _<AListView> mUsersListView;

    void showUserConfigureDialogFor(unsigned index);
public:

    MainWindow();
};

