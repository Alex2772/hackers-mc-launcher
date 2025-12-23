#pragma once


#include <AUI/Platform/AWindow.h>
#include <model/GameProfile.h>
#include <AUI/View/AButton.h>

class GameProfileWindow: public AWindow {
public:
    explicit GameProfileWindow(_<GameProfile> targetGameProfile);
private:
    _<AButton> mResetButton;
    _<GameProfile> mProfile;
    _<GameProfile> mTargetGameProfile;
    _<AViewContainer> mPageRoot;

    AProperty<_<AView>> mPage;

    _<AView> tab(AString name, std::function<_<AView>()> contents);
};

