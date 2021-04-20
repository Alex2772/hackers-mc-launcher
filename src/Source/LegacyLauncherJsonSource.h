#pragma once


#include <AUI/IO/APath.h>

/**
 * Load data from launcher_profiles.json
 */
class LegacyLauncherJsonSource {
private:
    static APath getVersionsJsonFilePath();
public:
    static void load();
    static void save();
};

