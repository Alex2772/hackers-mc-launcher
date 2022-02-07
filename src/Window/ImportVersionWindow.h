#pragma once


#include <AUI/Platform/AWindow.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/AListView.h>
#include <AUI/Model/AListModel.h>
#include <AUI/View/ATextField.h>
#include <AUI/Util/ABitField.h>

struct Version;

ENUM_FLAG(VersionType) {
    NONE = 0,
    RELEASE = 0b1,
    SNAPSHOT = 0b10,
    OLD_BETA = 0b100,
    OLD_ALPHA = 0b1000,
};

ENUM_VALUES(VersionType,
            VersionType::NONE,
            VersionType::RELEASE,
            VersionType::SNAPSHOT,
            VersionType::OLD_BETA,
            VersionType::OLD_ALPHA)



class ImportVersionWindow: public AWindow {
private:
    _<ARadioButton::Group> mRadioGroup = _new<ARadioButton::Group>();
    _<AListView> mMinecraftRepoList;
    _<AButton> mImportButton;
    _<AListModel<Version>> mVersionModel;
    _<ATextField> mSearchTextField;
    VersionType mVersionTypeValue = VersionType::NONE;
    ARadioButton::Group mReleaseTypeGroup;
    AFuture<> mImportTask;

    void doImportFromMinecraftRepo();

    emits<> invalidateSearch;

public:

    ImportVersionWindow();

};

