//
// Created by alex2772 on 4/13/25.
//

#include "DetailsPage.h"
#include "View/Common.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/Platform/AMessageBox.h>

using namespace declarative;

static void stubAction() {
    AMessageBox::show(
        dynamic_cast<AWindow*>(AWindow::current()), "Hacker's MC",
        "Hah, you thought it was implemented?! You are naive...");
}
static void stubButton(const _<AView>& button) {
    AObject::connect(button->clicked, AObject::GENERIC_OBSERVER, [] { stubAction(); });
}

DetailsPage::DetailsPage(GameProfile& profile) : mProfile(profile) {
    setContents(Vertical {
      _form(
          { { Button {
                _new<ADrawableView>(":profile_icons/default.png"_url) AUI_LET { stubButton(it); },
              },
              Vertical {
                _new<ATextField>() && profile.name,
                description("Display name. You can use whatever symbols you want."),
              } },
            { "UUID:",
              Vertical {
                _new<ATextField>() AUI_LET {
                        it->setText(profile.getUuid().toString());
                        it->setEditable(false);
                    },
                description("Identifies profile within this launcher and potentially in others.") } } }),

    });
}
