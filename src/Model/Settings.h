#pragma once


#include <AUI/Json/AJson.h>

struct Settings {
    static void reset();

public:
    APath game_folder;
    uint16_t width = 854;
    uint16_t height = 500;
    bool is_fullscreen = false;

    AJSON_FIELDS(game_folder, width, height)

    static Settings& inst();

    static void save();

private:
    Settings();
    void initEmptyFields();
};