//
// Created by alex2 on 15.04.2021.
//

#pragma once


#include <AUI/Platform/AWindow.h>
#include <AUI/View/AButton.h>
#include "Model/Settings.h"

class LauncherSettingsWindow: public AWindow {
private:
    Settings mSettings = Settings::inst();
    _<AButton> mResetButton;

    void clearGameDir();

    AFuture<> mTask;
    _<AView> mClearGameDirButton;
    _<AView> mClearGameDirSpinner;

public:

    LauncherSettingsWindow();
};


