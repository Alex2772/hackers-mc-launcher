#pragma once


#include <AUI/Json/AJson.h>
#include <AUI/Common/AProperty.h>

struct Settings {
public:
    AProperty<APath> gameDir;
    AProperty<int> width = 854;
    AProperty<int> height = 500;
    AProperty<bool> isFullscreen = false;

    bool operator==(const Settings& rhs) const {
        return std::tie(gameDir, width, height, isFullscreen) ==
               std::tie(rhs.gameDir, rhs.width, rhs.height, rhs.isFullscreen);
    }

    bool operator!=(const Settings& rhs) const {
        return !(rhs == *this);
    }

    static Settings& inst();

    static void save();
    static void reset();

    Settings();

private:
    void initEmptyFields();
};
AJSON_FIELDS(Settings,
             (gameDir, "game_dir")
             (width, "width")
             (height, "height")
             (isFullscreen, "is_fullscreen"))