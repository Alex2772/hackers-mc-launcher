//
// Created by alex2772 on 4/13/25.
//

#include "Common.h"
#include <AUI/View/AText.h>
#include <AUI/Util/UIBuildingHelpers.h>

using namespace declarative;
using namespace ass;

_<AView> declarative::description(const AString &text) {
    return AText::fromString(text) with_style { Opacity(0.5f) };
}