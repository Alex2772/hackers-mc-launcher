//
// Created by alex2772 on 3/2/21.
//

#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AComboBox.h>
#include <AUI/View/AText.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Json/AJson.h>
#include <AUI/Model/AModels.h>
#include <AUI/View/ACheckBox.h>
#include <Model/GameProfile.h>
#include "ImportVersionWindow.h"
#include "Source/LegacyLauncherJsonSource.h"

#include <AUI/Util/UIBuildingHelpers.h>
#include <Repository/GameProfilesRepository.h>
#include <AUI/Util/ARandom.h>
#include <Model/Settings.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/View/ATextField.h>

using namespace ass;


struct Version {
    AString id;
    AString url;
    VersionType type;
};

ImportVersionWindow::ImportVersionWindow():
    AWindow("Import version", 500_dp, 400_dp, dynamic_cast<AWindow*>(AWindow::current()), WindowStyle::MODAL)
{
    connect(mReleaseTypeGroup.selectionChanged, [&](int d) {
        mVersionTypeValue = (VersionType)d;
        emit invalidateSearch;
    });
    _<AView> minecraftRepoListWrap = Horizontal {
        mMinecraftRepoList = _new<AListView>() let {
            it->setCustomAss({ Expanding{} });
            connect(it->itemDoubleClicked, me::doImportFromMinecraftRepo);
        },
        Vertical {
            _new<ALabel>("Search:"),
            mSearchTextField = _new<ATextField>().connect(&ATextField::textChanging, [&]{ emit invalidateSearch; }) let { it->focus(); },
            _new<ALabel>("Filter:"),
            mReleaseTypeGroup.addRadioButton(_new<ARadioButton>("Releases"), int(VersionType::RELEASE)),
            mReleaseTypeGroup.addRadioButton(_new<ARadioButton>("Snapshots"), int(VersionType::SNAPSHOT)),
            mReleaseTypeGroup.addRadioButton(_new<ARadioButton>("Betas"), int(VersionType::OLD_BETA)),
            mReleaseTypeGroup.addRadioButton(_new<ARadioButton>("Alphas"), int(VersionType::OLD_ALPHA)),
        } with_style { MinSize { 200_dp, {} } },
    } let {
        it->setExpanding();
        it->addAssName(".import_version_offset");
    };

    mReleaseTypeGroup.setSelectedId(int(VersionType::RELEASE));

    auto importFromFile = Vertical{
            AText::fromString("You can import a profile packed into the zip file sent you by your friend or "
                              "downloaded from the internet."),
            Centered { _new<AButton>("Choose file") } with_style { Padding { 8_dp } },
    } << ".import_version_offset";

    setContents(Vertical {
        _new<ALabel>("Import version") << ".title",
        AText::fromString("Please choose where do you want import version from:"),

        _new<ARadioButton>("Official Minecraft repository") let {
            mRadioGroup->addRadioButton(it);
            connect(it->checked, minecraftRepoListWrap, &AView::setEnabled);
        },
        minecraftRepoListWrap,

        _new<ARadioButton>("Zip archive") let {
            mRadioGroup->addRadioButton(it);
            connect(it->checked, importFromFile, &AView::setEnabled);
        },
        importFromFile,

        Horizontal {
            _new<ASpacer>(),
            mImportButton = _new<AButton>("Import").connect(&AView::clicked, this, [&] {
                switch (mRadioGroup->getSelectedId()) {
                    case 0: // official repo
                        doImportFromMinecraftRepo();
                        break;
                }
            }) let { it->setDefault(); },
            _new<AButton>("Cancel").connect(&AView::clicked, me::close)
        }
    });

    mRadioGroup->uncheckAll();
    mRadioGroup->setSelectedId(0);

    minecraftRepoListWrap->setDisabled();

    mVersionModel = _new<AListModel<Version>>();

    async {
        auto versionManifest = AJson::read(_new<ACurl>("https://launchermeta.mojang.com/mc/game/version_manifest.json"));
        for (auto& version : versionManifest["versions"].asArray()) {
            mVersionModel->push_back({
                version["id"].asString(),
                version["url"].asString(),
                AEnumerate<VersionType>::byName(version["type"].asString()),
            });
        }

        ui_thread {
            minecraftRepoListWrap->setEnabled();
            auto filterModel = AModels::filter(mVersionModel, [&](const Version& v) {
                if (v.type != mVersionTypeValue) return false;

                auto filterString = mSearchTextField->getText();
                if (!filterString.empty()) {
                    if (!v.id.contains(filterString)) {
                        return false;
                    }
                }

                return true;
            });
            connect(invalidateSearch, slot(filterModel)::invalidate);
            mMinecraftRepoList->setModel(AModels::adapt<AString>(filterModel, [](const Version& v) { return v.id; }));
        };
    };
}

void ImportVersionWindow::doImportFromMinecraftRepo() {
    mImportButton->setDisabled();
    for (const auto& row : mMinecraftRepoList->getSelectionModel()) {
        Version version = mVersionModel->listItemAt(row.getIndex().getRow());

        async {
            try {
                GameProfile p;
                auto file = Settings::inst().game_dir["versions"][version.id][version.id + ".json"];
                file.parent().makeDirs();
                ALogger::info("Importing {}"_as.format(version.url));
                AFileOutputStream(file) << ACurl(version.url);
                GameProfile::fromJson(p, Autumn::get<ARandom>()->nextUuid(), version.id, AJson::read(_new<AFileInputStream>(file)).asObject());
                p.save();
                ui_thread {
                    GameProfilesRepository::inst().addGameProfile(p);
                };
                LegacyLauncherJsonSource::save();
            } catch (const AException& e) {
                AMessageBox::show(this, "Could not import version", e.getMessage(), AMessageBox::Icon::CRITICAL);
            }
            close();
        };
    }
}
