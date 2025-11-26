//
// Created by alex2772 on 2/15/21.
//

#include "MyUpdater.h"

#include <AUI/Platform/Entry.h>
#include <Window/MainWindow.h>
#include <AUI/Common/Plugin.h>
#include <Source/LegacyLauncherJsonSource.h>
#include <AUI/Util/ARandom.h>
#include <Model/Settings.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AMessageBox.h>
#include "Util.h"

using namespace std::chrono_literals;

AUI_ENTRY {
    MyUpdater::inst().handleStartup(args);
    MainWindow::inst().show();
    auto welcome = Util::launcherDir() / "WELCOME";
    if (!welcome.isRegularFileExists()) {
        welcome.touch();
        ATimer::scheduler().enqueue(1s, [] {
          // an honor to Legacy Launcher, formerly TLauncher Legacy.
          AMessageBox::show(dynamic_cast<AWindow*>(AWindow::current()),
                            "Welcome to Hacker's MC Launcher!",
                            "Launcher does not verify your account if you own a copy of Minecraft Java Edition to "
                            "play. By using this launcher, you agree that downloading and launching Minecraft on your "
                            "machine through this software is purely legit."
                            "\n\n"
                            "If not, we strongly encourage you to close Hacker's MC Launcher and purchase a legit "
                            "copy of Minecraft at minecraft.net. We promise, it's worth it!"
                            "\n\n"
                            "Thanks!"
          );
        });
    }
    return 0;
};