#pragma once


#include <AUI/Json/AJson.h>

struct Settings {
public:
    APath game_folder;
    APath java_executable = findJava();
    uint16_t width = 854;
    uint16_t height = 500;
    bool is_fullscreen = false;
    AJSON_FIELDS(game_folder, width, height, is_fullscreen)

    bool operator==(const Settings& rhs) const {
        return std::tie(game_folder, width, height, is_fullscreen) ==
               std::tie(rhs.game_folder, rhs.width, rhs.height, rhs.is_fullscreen);
    }

    bool operator!=(const Settings& rhs) const {
        return !(rhs == *this);
    }

    static Settings& inst();

    static void save();
    static void reset();

private:
    Settings();
    void initEmptyFields();
    static APath findJava();
};