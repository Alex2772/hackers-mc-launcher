#pragma once

#include "GameProfile.h"
#include "Account.h"
#include <AUI/Common/AProperty.h>

struct State {
    struct Profiles {
        AProperty<AVector<_<GameProfile>>> list;
        AProperty<_<GameProfile>> selected;

        ASet<AUuid> uuids() const;
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
};