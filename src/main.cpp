//
// Created by alex2772 on 2/15/21.
//

#include <AUI/Platform/Entry.h>
#include <Window/MainWindow.h>
#include <AUI/Common/Plugin.h>

AUI_ENTRY {
    aui::importPlugin("image");
    _new<MainWindow>()->show();
    return 0;
};