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
};

