#pragma once


#include <AUI/Platform/AWindow.h>
#include <Model/GameProfile.h>
#include <AUI/View/AButton.h>

class GameProfileWindow: public AWindow {
private:
    _<AButton> mResetButton;
    GameProfile& mTargetGameProfile;

public:
    explicit GameProfileWindow(GameProfile& targetGameProfile);
};

