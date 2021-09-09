//
// Created by alex2772 on 4/16/21.
//

#include <AUI/IO/FileInputStream.h>
#include <AUI/IO/FileOutputStream.h>
#include <Util.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/platform.h>
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

APath Settings::findJava() {
    auto javaLocations = APath::find(aui::platform::current::is_windows() ? "java.exe" : "java",
                                     aui::platform::current::is_windows()
                                     ? AVector<APath>{"C:\\Program Files\\Java", "C:\\Program Files (x86)\\Java"}
                                     : AVector<APath>{"/usr/lib/jvm/jdk-16.0.2", "/usr/lib/jvm"},
                                     PathFinder::SINGLE | PathFinder::RECURSIVE);
    if (javaLocations.empty()) {
        return {};
    }
    return javaLocations.first();
}

