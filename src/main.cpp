//
// Created by alex2772 on 2/15/21.
//

#include <AUI/Platform/Entry.h>
#include <Window/MainWindow.h>
#include <AUI/Common/Plugin.h>
#include <Source/LegacyLauncherJsonSource.h>
#include <AUI/Util/ARandom.h>
#include <Model/Settings.h>
#include <AUI/Logging/ALogger.h>

AUI_ENTRY {
    Autumn::put(_new<ARandom>());
    auto w = _new<MainWindow>();
    Autumn::put(w);
    w->show();
    return 0;
};