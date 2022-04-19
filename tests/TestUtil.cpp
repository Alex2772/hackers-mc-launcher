//
// Created by Alex2772 on 12/24/2021.
//

#include <AUI/Autumn/Autumn.h>
#include <AUI/Util/ARandom.h>
#include "TestUtil.h"
#include "Source/LegacyLauncherJsonSource.h"
#include "Window/MainWindow.h"
#include "Model/Settings.h"

void TestUtil::prepareApp() {
    // fake game dir
    Settings::inst().gameDir = APath("fake_game_dir").makeDirs().absolute();

    (Settings::inst().gameDir / "libraries").removeFileRecursive();
    (Settings::inst().gameDir / "versions").removeFileRecursive();

    Autumn::put(_new<ARandom>());
}

void TestUtil::prepareMainWindow() {
    LegacyLauncherJsonSource::load();
    auto w = _new<MainWindow>();
    Autumn::put(w);
    w->show();
}
