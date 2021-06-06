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
    _<ALabel> mDownloadedLabel;
    _<ALabel> mTotalLabel;
    _<ALabel> mTargetFileLabel;

    void showUserConfigureDialogFor(unsigned index);
    void showGameProfileConfigureDialogFor(unsigned index);

    void checkForDiskProfileUpdates();

public:

    MainWindow();

    void onMouseMove(glm::ivec2 pos) override;
};

