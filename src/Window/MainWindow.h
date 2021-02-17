#pragma once

#include <AUI/Platform/AWindow.h>
#include <AUI/Model/AListModel.h>

class MainWindow: public AWindow {
private:
    _<AListModel<AString>> mUsersModel;
    _<AListModel<AString>> mProfilesModel;

public:

    MainWindow();
};

