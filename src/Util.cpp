//
// Created by alex2772 on 3/1/21.
//

#include <AUI/Util/kAUI.h>
#include "Util.h"

APath Util::getSettingsDir() {
    auto path = APath::getDefaultPath(APath::APPDATA).file(".hackers-mc");
    do_once {
        path.makeDirs();
    };
    return path;
}
