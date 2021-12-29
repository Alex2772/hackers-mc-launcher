//
// Created by alex2772 on 3/2/21.
//

#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AComboBox.h>
#include <AUI/View/AText.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Json/AJson.h>
#include <AUI/Model/AListModelAdapter.h>
#include <AUI/View/ACheckBox.h>
#include <Model/GameProfile.h>
#include "ImportVersionWindow.h"

#include <AUI/Util/UIBuildingHelpers.h>
#include <Repository/GameProfilesRepository.h>
#include <AUI/Util/ARandom.h>
#include <Model/Settings.h>
#include <AUI/IO/FileInputStream.h>
#include <AUI/Platform/AMessageBox.h>

struct Version {
    AString id;
    AString url;
};

ImportVersionWindow::ImportVersionWindow():
    AWindow("Import version", 500_dp, 400_dp, dynamic_cast<AWindow*>(AWindow::current()), WindowStyle::MODAL)
{
    _<AView> minecraftRepoListWrap = Horizontal {/*
        Vertical {
            _new<ALabel>("Filter:"),
            _new<ACheckBox>("Snapshots"),
            _new<ACheckBox>("Releases"),
            _new<ACheckBox>("Betas")
        } let {
            it->setExpanding({0, 0});
        },
*/
        mMinecraftRepoList = _new<AListView>() let {
            it->setExpanding({10, 2});
            connect(it->itemDoubleClicked, me::doImportFromMinecraftRepo);
        },
    } let {
        it->setExpanding({2, 2});
        it->addAssName(".import_version_offset");
    };


    auto importFromFile = Vertical {
            AText::fromString("You can import a profile packed into the zip file sent you by your friend or "
                              "downloaded from the internet."),
            _new<AButton>("Choose file")
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
                version["url"].asString()
            });
        }

        ui_thread {
            minecraftRepoListWrap->setEnabled();
            mMinecraftRepoList->setModel(AAdapter::make<Version>(mVersionModel, [](const Version& v) { return v.id; }));
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
                _new<ACurl>(version.url) >> _new<FileOutputStream>(file);
                GameProfile::fromJson(p, Autumn::get<ARandom>()->nextUuid(), version.id, AJson::read(_new<FileInputStream>(file)).asObject());
                GameProfilesRepository::inst().addGameProfile(p);
                p.save();
            } catch (const AException& e) {
                AMessageBox::show(this, "Could not import version", e.getMessage(), AMessageBox::Icon::CRITICAL);
            }
            close();
        };
    }
}
