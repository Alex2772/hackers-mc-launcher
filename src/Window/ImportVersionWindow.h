#pragma once


#include <AUI/Platform/AWindow.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AButton.h>
#include <AUI/Model/AListModel.h>
#include <AUI/View/ATextField.h>
#include <AUI/Util/ABitField.h>
#include "Model/Version.h"


class ImportVersionWindow: public AWindow {
public:
    ImportVersionWindow();

private:
    _<ARadioButton::Group> mRadioGroup = _new<ARadioButton::Group>();
    _<AListView> mMinecraftRepoList;
    _<AButton> mImportButton;
    _<IListModel<Version>> mVersionModel;
    _<ATextField> mSearchTextField;
    VersionType mVersionTypeValue = VersionType::NONE;
    ARadioButton::Group mReleaseTypeGroup;
    AFuture<> mImportTask;

    void doImportFromMinecraftRepo();

    emits<> invalidateSearch;
};

