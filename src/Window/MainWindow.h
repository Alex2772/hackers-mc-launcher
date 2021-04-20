#pragma once

#include <AUI/Platform/AWindow.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>

class MainWindow: public AWindow {
private:
    _<AButton> mUserConfigureButton;
    _<AButton> mGameProfileConfigureButton;
    _<AListView> mUsersListView;
    _<AListView> mGameProfilesListView;

    void showUserConfigureDialogFor(unsigned index);
    void showGameProfileConfigureDialogFor(unsigned index);
public:
    MainWindow();
};

