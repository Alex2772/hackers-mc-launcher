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
#include "Model/State.h"

class ImportVersionWindow: public AWindow {
public:
    ImportVersionWindow(State& state);

private:
    State& mState;
    _<AListView> mMinecraftRepoList;
    _<IListModel<Version>> mVersionModel;
    AProperty<VersionType> mVersionTypeValue = VersionType::RELEASE;
    AProperty<AString> mSearchTextField;
    AAsyncHolder mAsync;

    void doImportFromMinecraftRepo();
    void showChooseFileDialog();

};

