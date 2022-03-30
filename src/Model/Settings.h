#pragma once


#include <AUI/Json/AJson.h>

struct Settings {
public:
    APath gameDir;
    int width = 854;
    int height = 500;
    bool isFullscreen = false;

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