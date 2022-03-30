//
// Created by alex2772 on 4/20/21.
//

#include <Model/Settings.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Logging/ALogger.h>
#include <Repository/UsersRepository.h>
#include <Repository/GameProfilesRepository.h>
#include <AUI/IO/AFileOutputStream.h>
#include "LegacyLauncherJsonSource.h"


bool LegacyLauncherJsonSource::ourDoSave = true;

APath LegacyLauncherJsonSource::getVersionsJsonFilePath() {
    return Settings::inst().gameDir.file("launcher_profiles.json");
}

void LegacyLauncherJsonSource::load() {
    try {
        /*
{
  "authenticationDatabase" : {
    "e681e8fcde71f860219102687b158192" : {
      "username" : "alex2772sc@gmail.com"
    }
  },
  "clientToken" : "",
  "launcherVersion" : {
    "format" : 21,
    "name" : "",
    "profilesFormat" : 2
  },
  "profiles" : {
    "20bad5e4ac74fef693ba34e5e840a2d2" : {
      "created" : "2020-04-06T01:55:00.204Z",
      "icon" : "Furnace",
      "lastUsed" : "2020-04-06T01:55:04.528Z",
      "lastVersionId" : "rd-132211",
      "name" : "",
      "type" : "custom"
    },
    ....
  },
  "selectedUser" : {
    "account" : "e681e8fcde71f860219102687b158192",
    "profile" : ""
  },
  "settings" : {
	...
  }
}
	 */
        auto config = AJson::fromStream(AFileInputStream(getVersionsJsonFilePath()));
        GameProfilesRepository::inst().getCurrentlyLoadedSetOfProfiles().clear();

        // try to load users
        try {
            for (auto& entry : config["authenticationDatabase"].asObject()) {
                Account account{entry.first, entry.second["username"].asString()};
                if (UsernameValidator()(account.username)) {
                    UsersRepository::inst().getModel() << account;
                }
            }
        } catch (const AException& e) {
            ALogger::warn("Unable to load users from launcher_profiles.json: " + e.getMessage());
        }

        // try to load game profiles
        try {
            for (auto& entry : config["profiles"].asObject()) {
                auto uuid = safeUuid(entry.first);
                GameProfilesRepository::inst().getCurrentlyLoadedSetOfProfiles() << uuid;
                AString name = "unknown";
                try {
                    // optifine fix
                    try {
                        name = entry.second["lastVersionId"].asString();
                    } catch (...) {
                        name = entry.second["name"].asString();
                    }
                    if (!name.startsWith("latest-")) {
                        GameProfile p;
                        GameProfile::fromName(p, uuid, name);
                        GameProfilesRepository::inst().getModel() << p;
                        ALogger::info("Imported profile: " + name);
                    }
                } catch (const AException& e) {
                    ALogger::err("ProfileLoading") << "Unable to load game profile " << name << " from launcher_profiles.json: " << e.getMessage();
                }
            }
        } catch (const AException& e) {
            ALogger::warn("Unable to load users from launcher_profiles.json: " + e.getMessage());
        }

    } catch (const AException& e) {
        ALogger::warn("Could not load launcher_profiles.json: " + e.getMessage());
    }
}

void LegacyLauncherJsonSource::save() {
    if (!ourDoSave) {
        return;
    }
    try {
        AJson config = {{
            {"clientToken", ""}, // client token is not supported
            {"launcherVersion", {
                {"format", 21},
                {"name", "hackers-mc"},
                {"profilesFormat", 2},
            }},
            {"authenticationDatabase", [] { // users
                AJson::Object result;
                for (const Account& u : UsersRepository::inst().getModel()) {
                    AJson::Object user;
                    user["username"] = u.username;
                    result[u.uuid.toRawString()] = user;
                }
                return result;
            }()},
            {"profiles", [] { // game profiles
                AJson::Object profiles;
                for (const GameProfile& p : GameProfilesRepository::inst().getModel()) {
                    profiles[p.getUuid().toRawString()] = AJson{
                        {"name", p.getName()}
                    };
                }
                return profiles;
            }()},
        }};
        AFileOutputStream(getVersionsJsonFilePath()) << config;
    } catch (const AException& e) {
        ALogger::warn("Could not save launcher_profiles.json: " + e.getMessage());
    }
}

void LegacyLauncherJsonSource::reload() {
    ourDoSave = false;
    UsersRepository::inst().getModel()->clear();
    GameProfilesRepository::inst().getModel()->clear();
    load();
    ourDoSave = true;
}

ASet<AUuid> LegacyLauncherJsonSource::getSetOfProfilesOnDisk() {
    ASet<AUuid> s;
    try {
        auto config = AJson::fromStream(AFileInputStream(getVersionsJsonFilePath()));

        // try to load game profiles
        try {
            for (auto& entry : config["profiles"].asObject()) {
                s << safeUuid(entry.first);
            }
        } catch (const AException& e) {
            ALogger::warn("Unable to load users from launcher_profiles.json: " + e.getMessage());
        }

    } catch (const AException& e) {
        ALogger::warn("Could not load launcher_profiles.json: " + e.getMessage());
    }
    return s;
}

AUuid LegacyLauncherJsonSource::safeUuid(const AString& uuid) {
    return AUuid::fromString(uuid);
}
