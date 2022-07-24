#pragma once


#include <AUI/Platform/AWindow.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AButton.h>
#include <AUI/Model/AListModel.h>
#include <AUI/View/ATextField.h>
#include <AUI/Util/ABitField.h>
#include <AUI/Thread/AAsyncHolder.h>
#include "Model/Version.h"


class ImportVersionWindow: public AWindow {
public:
    ImportVersionWindow();

private:
    _<AListView> mMinecraftRepoList;
    _<IListModel<Version>> mVersionModel;
    _<ATextField> mSearchTextField;
    VersionType mVersionTypeValue = VersionType::NONE;
    ARadioButton::Group mReleaseTypeGroup;
    AAsyncHolder mAsync;

    void doImportFromMinecraftRepo();
    void showChooseFileDialog();

    emits<> invalidateSearch;

};

