#pragma once

#include "Model/GameProfile.h"
#include <AUI/View/AViewContainer.h>

/**
 * @brief Deep profile json editor view.
 */
class DetailsPage : public AViewContainerBase {
public:
    DetailsPage(GameProfile& profile);

private:
    GameProfile& mProfile;
};
