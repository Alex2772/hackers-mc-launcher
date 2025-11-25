#pragma once
#include "AUI/Platform/AWindow.h"
#include "Model/GameProfile.h"

class GameProfileExportWindow: public AWindow {
public:
    GameProfileExportWindow(AWindow* parent, _<GameProfile> profile);
};
