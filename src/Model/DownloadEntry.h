#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Common/AVariant.h>
#include <AUI/Json/AJson.h>

struct Rule {
    enum class Action {
        ALLOW,
        DISALLOW
    } action;
    AVector<std::pair<AString, AVariant>> conditions;

    AJSON_FIELDS(action, conditions)
};

namespace aui::json::conv {
    template<>
    struct conv<Rule::Action> {
        static AJsonElement to_json(const Rule::Action& rule) {
            switch (rule) {
                case Rule::Action::ALLOW: return AJsonValue("allow");
                case Rule::Action::DISALLOW: return AJsonValue("disallow");
            }
        }
        static Rule::Action from_json(const AJsonElement& e) {
            if (e.asString() == "disallow") {
                return Rule::Action::DISALLOW;
            }
            return Rule::Action::ALLOW;
        }
    };
}

using Rules = AVector<Rule>;

struct DownloadEntry {
    AString mLocalPath;
    AString mUrl;
    uint64_t mSize = 0;
    bool mExtract = false;
    AString mHash;
    Rules mConditions;
};

