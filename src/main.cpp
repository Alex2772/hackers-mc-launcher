//
// Created by alex2772 on 2/15/21.
//

#include <AUI/Platform/Entry.h>
#include <Window/MainWindow.h>

AUI_ENTRY {
    _new<MainWindow>()->show();
    return 0;
};