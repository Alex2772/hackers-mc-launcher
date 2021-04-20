#pragma once

#include <AUI/Platform/AWindow.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>

class MainWindow: public AWindow {
private:
    _<AButton> mUserConfigureButton;
    _<AButton> mGameProfileConfigureButton;
    _<AButton> mPlayButton;
    _<AListView> mUsersListView;
    _<AListView> mGameProfilesListView;

    _<AViewContainer> mDownloadingPanel;
    _<ALabel> mStatusLabel;

    void showUserConfigureDialogFor(unsigned index);
    void showGameProfileConfigureDialogFor(unsigned index);
public:
    MainWindow();
};

