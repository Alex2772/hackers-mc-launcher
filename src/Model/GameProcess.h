#pragma once

#include <AUI/Platform/AProcess.h>
#include <AUI/Common/AByteBuffer.h>

class GameConsoleWindow;

struct GameProcess {
    _<AChildProcess> process;
    GameConsoleWindow* consoleWindow = nullptr;
    AProperty<AStringVector> stdoutBuffer;
};