#include <range/v3/all.hpp>
#include "State.h"

ASet<AUuid> State::Profiles::uuids() const {
    return *list | ranges::views::transform([](const _<GameProfile>& profile) {
        return profile->getUuid();
    }) | ranges::to<ASet<AUuid>>();
}
