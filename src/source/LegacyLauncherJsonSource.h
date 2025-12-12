#pragma once

#include <AUI/Common/ASet.h>
#include <AUI/Common/AUuid.h>
#include <AUI/IO/APath.h>
#include "model/State.h"

/**
 * Load data from launcher_profiles.json
 */
class LegacyLauncherJsonSource {
public:
    /**
     * @brief Always returns hacker's launcher_profiles.hck.json is exists, launcher_profiles.json otherwise.
     * @details
     * The differentiation is needed in order to not screw up official Minecraft launcher.
     */
    static APath getVersionsJsonFilePath();

    /**
     * @brief Always returns hacker's launcher_profiles.hck.json.
     */
    static APath getVersionsJsonFilePathHackers();
    static void reload(State& state);
    static void load(State& state);
    static void save(const State& state);
    static ASet<AUuid> getSetOfProfilesOnDisk();

    /**
     * OptiFine places "Optifine" string instead of valid UUID, this function
     * workarounds this
     * @param uuid possibly invalid uuid
     * @return passed uuid or generated from string
     */
    static AUuid safeUuid(const AString &uuid);

private:
    static bool ourDoSave;
};
