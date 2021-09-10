#pragma once

#include <AUI/Platform/AWindow.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AComboBox.h>
#include <View/GameProfilesView.h>

class MainWindow: public AWindow {
private:
    _<AButton> mUserConfigureButton;
    _<AButton> mGameProfileConfigureButton;
    _<AButton> mPlayButton;
    _<AComboBox> mUsersListView;
    _<GameProfilesView> mGameProfilesView;

    _<AViewContainer> mDownloadingPanel;
    _<ALabel> mStatusLabel;
    _<ALabel> mDownloadedLabel;
    _<ALabel> mTotalLabel;
    _<ALabel> mTargetFileLabel;

    void checkForDiskProfileUpdates();

public:

    MainWindow();

    void showUserConfigureDialogFor(unsigned index);
    void showGameProfileConfigureDialogFor(unsigned index);

    void onMouseMove(glm::ivec2 pos) override;

    void onPlayButtonClicked();

    void showPlayButton();

    void showDownloadingPanel();
};

