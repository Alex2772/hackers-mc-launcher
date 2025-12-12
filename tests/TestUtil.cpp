//
// Created by Alex2772 on 12/24/2021.
//

#include <AUI/Util/ARandom.h>
#include "TestUtil.h"
#include "source/LegacyLauncherJsonSource.h"
#include "window/MainWindow.h"
#include "model/Settings.h"

void TestUtil::prepareApp() {
    // fake game dir
    Settings::inst().gameDir = APath("fake_game_dir").makeDirs().absolute();

//    (Settings::inst().gameDir / "libraries").removeFileRecursive();
    AUI_DO_ONCE {
        (Settings::inst().gameDir / "versions").removeFileRecursive();
    }
}

void TestUtil::prepareMainWindow() {
    MainWindow::inst().show();
}

const APath& TestUtil::testDataDir() {
    static const auto path = [] {
        auto p = APath(__FILE__).parent() / "data";
        ALogger::info("TestUtil") << "Note: test data dir" << p;
        if (!p.isDirectoryExists()) {
            throw AException("launchCustom: missing test data dir: {}"_format(p));
        }
        return p;
    }();
    return path;
}

