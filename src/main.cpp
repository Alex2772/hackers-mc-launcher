//
// Created by alex2772 on 2/15/21.
//

#include <AUI/Platform/Entry.h>
#include <Window/MainWindow.h>
#include <AUI/Common/Plugin.h>

AUI_ENTRY {
    aui::importPlugin("image");
    auto w = _new<MainWindow>();
    Autumn::put(w);
    w->show();
    return 0;
};