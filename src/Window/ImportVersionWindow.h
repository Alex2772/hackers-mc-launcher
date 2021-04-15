#pragma once


#include <AUI/Platform/AWindow.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/Model/AListModel.h>

struct Version;

class ImportVersionWindow: public AWindow {
private:
    _<ARadioButton::Group> mRadioGroup = _new<ARadioButton::Group>();
    _<AListView> mMinecraftRepoList;
    _<AButton> mImportButton;
    _<AListModel<Version>> mVersionModel;

    void doImportFromMinecraftRepo();

public:

    ImportVersionWindow();
};

