#pragma once

#include "model/GameProfile.h"
#include <AUI/View/AViewContainer.h>

namespace declarative::game_profile {
/**
 * @brief Deep profile json editor view.
 */
_<AView> modsPage(_<GameProfile> profile);
}