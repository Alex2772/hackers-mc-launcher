//
// Created by alex2772 on 4/16/21.
//

#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <Util.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/platform.h>
#include "Settings.h"

const auto SETTINGS_PATH = Util::getSettingsDir().file("settings.json");

Settings& Settings::inst() {
    static Settings s;
    do_once {
        try {
            s = aui::from_json<Settings>(AJson::fromStream(AFileInputStream(SETTINGS_PATH)));
        } catch (...) {

        }
    }
    return s;
}

void Settings::save() {
    inst().initEmptyFields();
    AFileOutputStream(SETTINGS_PATH) << aui::to_json(inst());
}

Settings::Settings() {
    initEmptyFields();
}

void Settings::initEmptyFields() {
    if (gameDir.empty())
        gameDir = APath::getDefaultPath(APath::APPDATA).file(".minecraft");
}

void Settings::reset() {
    inst() = {};
    save();
}
