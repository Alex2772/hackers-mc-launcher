//
// Created by alex2772 on 4/15/21.
//

#include <AUI/Common/AVariant.h>
#include <AUI/Common/AMap.h>
#include <AUI/Traits/platform.h>
#include <Model/Settings.h>
#include <AUI/Autumn/Autumn.h>
#include <AUI/IO/StringStream.h>
#include <AUI/Util/ATokenizer.h>
#include "VariableHelper.h"

AVariant VariableHelper::getVariableValue(const Context& c, const AString& name)
{
    static AMap<AString, std::function<AVariant(const Context&)>> m = {
            {
                    "os.name",
                    [](const Context& c) -> AVariant
                    {

                        return aui::platform::current::name();
                    }
            },
            {
                    "os.version",
                    [](const Context& c) -> AVariant
                    {
                        return "unknown";
                    }
            },
            {
                    "os.arch",
                    [](const Context& c) -> AVariant
                    {
                        return "x86";
                    }
            },
            {
                    "natives_directory",
                    [](const Context& c) -> AVariant
                    {
                        if (c.profile) {
                            return Settings::inst().game_folder["bin"][c.profile->getUuid().toRawString()];
                        }
                        return {};
                    }
            },
            {
                    "launcher_name",
                    [](const Context& c) -> AVariant
                    {
                        return "hackers-mc-launcher";
                    }
            },
            {
                    "launcher_version",
                    [](const Context& c) -> AVariant
                    {
                        return HACKERS_MC_VERSION;
                    }
            },
            {
                    "classpath",
                    [](const Context& c) -> AVariant
                    {
                        if (c.profile) {
                            return c.profile->getClasspath().join(aui::platform::current::is_unix() ? ':' : ';');
                        }
                        return {};
                    }
            },
            {
                    "auth_player_name",
                    [](const Context& c) -> AVariant
                    {
                        if (c.user) {
                            return c.user->username;
                        }
                        return {};
                    }
            },
            {
                    "auth_uuid",
                    [](const Context& c) -> AVariant
                    {
                        if (c.user) {
                            return c.user->uuid.toRawString();
                        }
                        return {};
                    }
            },
            {
                    "auth_access_token",
                    [](const Context& c) -> AVariant
                    {
                        return "null";
                    }
            },
            {
                    "user_type",
                    [](const Context& c) -> AVariant
                    {
                        return "legacy";
                    }
            },
            {
                    "version_type",
                    [](const Context& c) -> AVariant
                    {
                        return "release";
                    }
            },
            {
                    "version_name",
                    [](const Context& c) -> AVariant
                    {
                        if (c.profile) {
                            return c.profile->getName();
                        }
                        return {};
                    }
            },
            {
                    "game_directory",
                    [](const Context& c) -> AVariant
                    {
                        return Settings::inst().game_folder;
                    }
            },
            {
                    "assets_root",
                    [](const Context& c) -> AVariant
                    {
                        return Settings::inst().game_folder["assets"];
                    }
            },
            {
                    "assets_index_name",
                    [](const Context& c) -> AVariant
                    {
                        if (c.profile) {
                            return c.profile->getAssetsIndex();
                        }
                        return {};
                    }
            },
            {
                    "user_properties",
                    [](const Context& c) -> AVariant
                    {
                        return "{}";
                    }
            },
            {
                    "has_custom_resolution",
                    [](const Context& c) -> AVariant
                    {
                        return true;
                    }
            },
            {
                    "resolution_width",
                    [](const Context& c) -> AVariant
                    {
                        return Settings::inst().width;
                    }
            },
            {
                    "resolution_height",
                    [](const Context& c) -> AVariant
                    {
                        return Settings::inst().height;
                    }
            },
            {
                    "is_fullscreen",
                    [](const Context& c) -> AVariant
                    {
                        return Settings::inst().is_fullscreen;
                    }
            },
            {
                    "is_demo_user",
                    [](const Context& c) -> AVariant
                    {
                        return false;
                    }
            },
    };

    
    if (auto co = m.contains(name)) {
        return co->second(c);
    }
    return "null";
}

bool VariableHelper::checkConditions(const Context& c, const AVector<std::pair<AString, AVariant>>& conditions) {
    for (auto& v : conditions) {
        if (getVariableValue(c, v.first) != v.second) {
            return false;
        }
    }
    return true;
}

AString VariableHelper::parseVariables(const Context& c, const AString& s) {
    std::string r;
    r.reserve(s.length());

    ATokenizer tokenizer(_new<StringStream>(s));
    try {
        for (;;) {
            tokenizer.readStringUntilUnescaped(r, '$');
            if (tokenizer.readChar() == '{') {
                auto name = tokenizer.readStringUntilUnescaped('}');
                r += getVariableValue(c, name).toString().toStdString();
            } else {
                r += tokenizer.getLastCharacter();
            }
        }
    } catch (...) {

    }
    return r;
}
