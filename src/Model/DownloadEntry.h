#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Common/AVariant.h>
#include <AUI/Json/AJson.h>

struct LauncherRule {
    enum class Action {
        ALLOW,
        DISALLOW
    } action;
    AVector<std::pair<AString, AVariant>> conditions;

    AJSON_FIELDS(action, conditions)
};

namespace aui::json::conv {
    template<>
    struct conv<LauncherRule::Action> {
        static AJsonElement to_json(const LauncherRule::Action& rule) {
            switch (rule) {
                case LauncherRule::Action::ALLOW: return AJsonValue("allow");
                case LauncherRule::Action::DISALLOW: return AJsonValue("disallow");
            }
        }
        static LauncherRule::Action from_json(const AJsonElement& e) {
            if (e.asString() == "disallow") {
                return LauncherRule::Action::DISALLOW;
            }
            return LauncherRule::Action::ALLOW;
        }
    };
}

using Rules = AVector<LauncherRule>;

struct DownloadEntry {
    AString mLocalPath;
    AString mUrl;
    uint64_t mSize = 0;
    bool mExtract = false;
    AString mHash;
    Rules mConditions;
};

