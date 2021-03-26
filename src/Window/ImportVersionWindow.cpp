//
// Created by alex2772 on 3/2/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AComboBox.h>
#include <AUI/View/AImageView.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Json/AJson.h>
#include <AUI/Model/AListModelAdapter.h>
#include "ImportVersionWindow.h"


struct Version {
    AString id;
};

ImportVersionWindow::ImportVersionWindow():
    AWindow("Import version", 500_dp, 400_dp, AWindow::current(), WS_DIALOG)
{
    auto minecraftRepoList = _new<AListView>() let {
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
            connect(it->checked, minecraftRepoList, &AView::setEnabled);
        },
        minecraftRepoList,

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
            _new<AButton>("Import").connect(&AView::clicked, this, [&] {
                close();
            }) let { it->setDefault(); },
            _new<AButton>("Cancel").connect(&AView::clicked, me::close)
        }
    });

    mRadioGroup->uncheckAll();
    mRadioGroup->setSelectedId(0);

    minecraftRepoList->setDisabled();

    _<AListModel<Version>> listModel = _new<AListModel<Version>>();

    async {
        auto versionManifest = AJson::read(_new<ACurl>("https://launchermeta.mojang.com/mc/game/version_manifest.json"));
        for (auto& version : versionManifest["versions"].asArray()) {
            auto id = version["id"].asString();
            listModel->push_back({id});
        }

        ui {
            minecraftRepoList->setEnabled();
            minecraftRepoList->setModel(AAdapter::make<Version>(listModel, [](const Version& v) { return v.id; }));
        };
    };
}
