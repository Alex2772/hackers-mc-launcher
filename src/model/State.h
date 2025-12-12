#pragma once

#include "GameProfile.h"
#include "Account.h"
#include <AUI/Common/AProperty.h>

struct State {
    struct Profiles {
        AProperty<AVector<_<GameProfile>>> list;
        AProperty<_<GameProfile>> selected;

        void notify() {
            list.notify();
            selected.notify();
        }
    } profile;

    struct Accounts {
//        AProperty<AVector<_<Account>>> list;
//        AProperty<_<Account>> selected;
        _<Account> current = _new<Account>();
    } accounts;

    /**
     * @brief Used to track updates in launcher_profiles.json.
     * @details
     * When using Forge or OptiFine installer, it populates launcher_profiles.json with new profiles. MainWindow
     * compares for that every time it acquires focus.
     */
    ASet<AUuid> profilesUuidsSnapshot;
};