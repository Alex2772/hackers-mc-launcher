//
// Created by alex2772 on 4/16/21.
//

#include <AUI/IO/FileInputStream.h>
#include <AUI/IO/FileOutputStream.h>
#include <Util.h>
#include <AUI/Util/kAUI.h>
#include "Settings.h"

const auto SETTINGS_PATH = Util::getSettingsDir().file("settings.json");

Settings& Settings::inst() {
    static Settings s;
    do_once {
        try {
            s.readJson(AJson::read(_new<FileInputStream>(SETTINGS_PATH)));
        } catch (...) {

        }
    }
    return s;
}

void Settings::save() {
    inst().initEmptyFields();
    AJson::write(_new<FileOutputStream>(SETTINGS_PATH), inst().toJson());
}

Settings::Settings() {
    initEmptyFields();
}

void Settings::initEmptyFields() {
    if (game_folder.empty())
        game_folder = APath::getDefaultPath(APath::APPDATA).file(".minecraft");
}

void Settings::reset() {
    inst() = {};
    save();
}

