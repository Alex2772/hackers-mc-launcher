//
// Created by alex2772 on 4/20/21.
//
#include <range/v3/all.hpp>
#include <Model/Settings.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Util/ARandom.h>
#include "LegacyLauncherJsonSource.h"
#include "Model/GameProfile.h"
#include "Model/State.h"

static constexpr auto LOG_TAG = "LegacyLauncherJsonSource";

bool LegacyLauncherJsonSource::ourDoSave = true;

APath LegacyLauncherJsonSource::getVersionsJsonFilePath() {
    if (auto h = getVersionsJsonFilePathHackers(); h.isRegularFileExists()) {
        return h;
    }
    return *Settings::inst().gameDir / "launcher_profiles.json";
}

APath LegacyLauncherJsonSource::getVersionsJsonFilePathHackers() {
    return *Settings::inst().gameDir / "launcher_profiles.hck.json";
}

static void loadProfile(State::Profiles& profiles, ASet<AString>& profilesLoadedFromConfig, const AUuid& uuid, const AString& name) {
    try {
        if (name.startsWith("latest-")) {
            return;
        }
        auto p = _new<GameProfile>();
        GameProfile::fromName(*p, uuid, name);
        profiles.list.raw << std::move(p);
        ALogger::info(LOG_TAG) << ("Imported profile: " + name);
        profilesLoadedFromConfig << std::move(name);
    } catch (const AException& e) {
        ALogger::err(LOG_TAG)
            << ("ProfileLoading") << "Unable to load game profile " << name
            << " from launcher_profiles.json: " << e.getMessage();
    }
}

void LegacyLauncherJsonSource::load(State& state) {
    ASet<AString> profilesLoadedFromConfig;
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
        if (getVersionsJsonFilePath().isRegularFileExists()) {
            auto config = AJson::fromStream(AFileInputStream(getVersionsJsonFilePath()));

            // try to load users
            try {
                for (auto &entry: config["authenticationDatabase"].asObject()) {
                    Account account{entry.first, entry.second["username"].asString()};
                    if (UsernameValidator()(account.username)) {
                        *state.accounts.current = std::move(account);
                    }
                }
            } catch (const AException &e) {
                ALogger::warn(LOG_TAG) << "Unable to load users from launcher_profiles.json: " << e.getMessage();
            }

            // try to load game profiles
            const auto &profiles = config["profiles"].asObject();

            try {
                for (const auto &[rawUuid, obj]: profiles) {
                    AString name = "unknown";
                    // optifine fix
                    try {
                        name = obj["lastVersionId"].asString();
                    } catch (...) {
                        name = obj["name"].asString();
                    }

                    auto uuid = safeUuid(rawUuid);

                    loadProfile(state.profile, profilesLoadedFromConfig, uuid, name);
                }
            } catch (const AException &e) {
                ALogger::warn(LOG_TAG) << "Unable to load users from launcher_profiles.json: " << e.getMessage();
            }

            if (config.contains("selectedProfile")) {
                auto selectedProfile = AUuid::fromString(config["selectedProfile"].asString());
                if (auto it = ranges::find(*state.profile.list, selectedProfile,
                                           [](const _<GameProfile> &p) { return p->getUuid(); }); it !=
                                                                                                  state.profile.list->end()) {
                    state.profile.selected = *it;
                }
            }
        }

    } catch (const AException& e) {
        ALogger::warn(LOG_TAG) << "Could not load launcher_profiles.json: " << e.getMessage();
    }

    try {
        // the newest official minecraft launcher also loads profiles which are not listed in launcher_profiles.json
        ARandom r;
        for (const auto& profileDir : (*Settings::inst().gameDir / "versions").listDir(AFileListFlags::DIRS)) {
            auto filename = profileDir.filename();
            if (!profilesLoadedFromConfig.contains(filename)) {
                ALogger::info(LOG_TAG)
                    << "Loading profile " << filename << " which is not listed in launcher_profiles.json";
                auto uuid = r.nextUuid();
                loadProfile(state.profile, profilesLoadedFromConfig, uuid, filename);
            }
        }
    } catch (const AException& e) {
        ALogger::warn(LOG_TAG) << "Could not load version from versions/ dir" << e.getMessage();
    }
    state.profilesUuidsSnapshot = getSetOfProfilesOnDisk();
}

void LegacyLauncherJsonSource::save(const State& state) {
    if (!ourDoSave) {
        return;
    }
    try {
        AJson config = { {
          { "clientToken", "" },   // client token is not supported
          { "launcherVersion",
            {
              { "format", 21 },
              { "name", "hackers-mc" },
              { "profilesFormat", 2 },
            } },
          { "authenticationDatabase",
            [&] {   // users
                AJson::Object result;
                for (const _<Account>& u : std::array{ state.accounts.current }) {
                    AJson::Object user;
                    user["username"] = u->username;
                    result[u->uuid.toRawString()] = user;
                }
                return result;
            }() },
          { "profiles",
            [&] {   // game profiles
                AJson::Object profiles;
                for (const auto& p : *state.profile.list) {
                    profiles[p->getUuid().toRawString()] = AJson { { "name", p->name } };
                }
                return profiles;
            }() },
        } };
        if (state.profile.selected != nullptr) {
            config["selectedProfile"] = (*state.profile.selected)->getUuid().toRawString();
        }
        AFileOutputStream(getVersionsJsonFilePathHackers()) << config;
    } catch (const AException& e) {
        ALogger::warn(LOG_TAG) << "Could not save launcher_profiles.json: " << e.getMessage();
    }
}

void LegacyLauncherJsonSource::reload(State& state) {
    ourDoSave = false;
//    state.accounts.list.raw.clear();
    state.profile.list.raw.clear();
    load(state);
    ourDoSave = true;
}

ASet<AUuid> LegacyLauncherJsonSource::getSetOfProfilesOnDisk() {
    ASet<AUuid> s;
    try {
        auto f = getVersionsJsonFilePath();
        if (!f.isRegularFileExists()) {
            return s;
        }
        auto config = AJson::fromStream(AFileInputStream(f));

        // try to load game profiles
        try {
            for (auto& entry : config["profiles"].asObject()) {
                s << safeUuid(entry.first);
            }
        } catch (const AException& e) {
            ALogger::warn(LOG_TAG) << "Unable to load users from launcher_profiles.json: " << e.getMessage();
        }

    } catch (const AException& e) {
        ALogger::warn(LOG_TAG) << "Could not load launcher_profiles.json: " << e.getMessage();
    }
    return s;
}

AUuid LegacyLauncherJsonSource::safeUuid(const AString& uuid) { return AUuid::fromString(uuid); }
