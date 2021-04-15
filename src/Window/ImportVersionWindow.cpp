//
// Created by alex2772 on 3/2/21.
//

#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AComboBox.h>
#include <AUI/View/AImageView.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Json/AJson.h>
#include <AUI/Model/AListModelAdapter.h>
#include <AUI/View/ACheckBox.h>
#include <Model/GameProfile.h>
#include "ImportVersionWindow.h"

#include <AUI/Util/UIBuildingHelpers.h>
#include <Repository/GameProfilesRepository.h>

struct Version {
    AString id;
    AString url;
};

ImportVersionWindow::ImportVersionWindow():
    AWindow("Import version", 500_dp, 400_dp, AWindow::current(), WS_DIALOG)
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


    auto vanillaLauncherProfile = _new<AComboBox>() let {
        it->addAssName(".import_version_offset");
    };

    auto importFromFile = Vertical {
            _new<ALabel>("You can import a profile packed into the zip file sent you by your friend or downloaded from "
                         "the internet.") let { it->setMultiline(true); },
            _new<AButton>("Choose file")
    } << ".import_version_offset";

    setContents(Vertical {
        _new<ALabel>("Import version") << ".title",
        _new<ALabel>("Please choose where do you want import version from:") let { it->setMultiline(true); },

        _new<ARadioButton>("Official Minecraft repository") let {
            mRadioGroup->addRadioButton(it);
            connect(it->checked, minecraftRepoListWrap, &AView::setEnabled);
        },
        minecraftRepoListWrap,

        _new<ARadioButton>("Vanilla launcher profile") let {
            mRadioGroup->addRadioButton(it);
            connect(it->checked, vanillaLauncherProfile, &AView::setEnabled);
        },
        vanillaLauncherProfile,
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

        ui {
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
                GameProfile::fromJson(p, version.id, AJson::read(_new<ACurl>(version.url)).asObject());
                GameProfilesRepository::inst().addGameProfile(p);
            } catch (...) {
            }
            close();
        };
    }
}
