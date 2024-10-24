#pragma once


#include <AUI/IO/APath.h>
#include <AUI/Common/ASet.h>
#include <AUI/Common/AUuid.h>

/**
 * Load data from launcher_profiles.json
 */
class LegacyLauncherJsonSource {
private:
    static bool ourDoSave;

    static APath getVersionsJsonFilePath();
public:
    static void reload();
    static void load();
    static void save();
    static ASet<AUuid> getSetOfProfilesOnDisk();

    /**
     * OptiFine places "Optifine" string instead of valid UUID, this function workarounds this
     * @param uuid possibly invalid uuid
     * @return passed uuid or generated from string
     */
    static AUuid safeUuid(const AString& uuid);
};

