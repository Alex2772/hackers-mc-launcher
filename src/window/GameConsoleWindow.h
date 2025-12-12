#pragma once


#include <AUI/Platform/AWindow.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/View/AText.h>
#include <AUI/View/AScrollArea.h>
#include "model/GameProcess.h"

class GameConsoleWindow: public AWindow {
public:
    GameConsoleWindow(AWindow* parent, _<GameProcess> game);
    static void handleGameExit(AWindow* parent, _<GameProcess> game);
};


