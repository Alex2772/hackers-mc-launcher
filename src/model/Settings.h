#pragma once


#include <AUI/Json/AJson.h>
#include <AUI/Common/AProperty.h>

struct Settings {
public:
    AProperty<APath> gameDir;
    AProperty<int> width = 854;
    AProperty<int> height = 500;
    AProperty<bool> isFullscreen = false;
    AProperty<bool> showConsoleOnPlay = false;
    AProperty<bool> autoUpdate = true;

    static Settings& inst();

    static void save();
    static void reset();

private:
    void initEmptyFields();
};
AJSON_FIELDS(Settings,
             (gameDir, "game_dir")
             (width, "width")
             (height, "height")
             (isFullscreen, "is_fullscreen")
             (showConsoleOnPlay, "show_console_on_play")
             (autoUpdate, "auto_update")
             )