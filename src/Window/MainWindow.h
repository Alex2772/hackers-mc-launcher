#pragma once

#include <AUI/Event/APointerMoveEvent.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/ADropdownList.h>
#include <View/GameProfilesView.h>
#include <AUI/View/ADropdownList.h>

class MainWindow: public AWindow {
public:

    void showUserConfigureDialogFor(unsigned index);
    void showGameProfileConfigureDialogFor(unsigned index);
    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;
    void onPlayButtonClicked();
    void showPlayButton();
    void showDownloadingPanel();

    static MainWindow& inst();

signals:
    emits<> reloadProfiles;

private:
    _<AButton> mUserConfigureButton;
    _<AButton> mGameProfileConfigureButton;
    _<AView> mPlayButton;
    _<ADropdownList> mUsersListView;
    _<GameProfilesView> mGameProfilesView;

    _<AViewContainer> mDownloadingPanel;
    _<ALabel> mStatusLabel;
    _<ALabel> mDownloadedLabel;
    _<ALabel> mTotalLabel;
    _<ALabel> mTargetFileLabel;

    AFuture<> mTask;

    _<AView> mSpinnerView;

    MainWindow();

    void checkForDiskProfileUpdates();
    void showProfileLoading();
    void hideProfileLoading();
    void openGameDir();
    void showLauncherSettings();
};

