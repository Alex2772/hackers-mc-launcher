//
// Created by alex2772 on 3/2/21.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AComboBox.h>
#include "ImportVersionWindow.h"

ImportVersionWindow::ImportVersionWindow():
    AWindow("Import version", 500_dp, 400_dp, AWindow::current(), WS_DIALOG)
{
    setContents(Vertical {
        _new<ALabel>("Import version") << ".title",
        _new<ALabel>("Please choose where do you want import version from:") let { it->setMultiline(true); },

        _new<ARadioButton>("Official Minecraft repository") let { mRadioGroup->addRadioButton(it); },
        _new<AListView>() let {
            it->setExpanding({2, 2});
            it->addAssName(".import_version_offset");
        },
        _new<ARadioButton>("Vanilla launcher profile") let { mRadioGroup->addRadioButton(it); },
        _new<AComboBox>() let {
            it->addAssName(".import_version_offset");
        },
        _new<ARadioButton>("Zip archive") let { mRadioGroup->addRadioButton(it); },
        Vertical {
            _new<ALabel>("You can import a profile packed into the zip file sent you by your friend or downloaded from "
                         "the internet.") let { it->setMultiline(true); },
            _new<AButton>("Choose file")
        } << ".import_version_offset",


        _new<ASpacer>(),

        Horizontal {
            _new<ASpacer>(),
            _new<AButton>("Import").connect(&AView::clicked, this, [&] {
                close();
            }) let { it->setDefault(); },
            _new<AButton>("Cancel").connect(&AView::clicked, me::close)
        }
    });

    mRadioGroup->setSelectedId(0);
}
