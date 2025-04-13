//
// Created by alex2772 on 4/15/21.
//

#include <AUI/Common/AMap.h>
#include <AUI/Traits/platform.h>
#include <AUI/Traits/strings.h>
#include <Model/Settings.h>
#include <AUI/IO/AStringStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Logging/ALogger.h>
#include "VariableHelper.h"
#include "AuthTricks.h"
#include <AUI/Util/ARandom.h>

AString VariableHelper::getVariableValue(const Context& c, const AString& name)
{
    static constexpr auto CLASSPATH_SEPARATOR = aui::platform::current::is_unix() ? ':' : ';';
    static AMap<AString, std::function<AString(const Context&)>> m = {
            {
                    "os.name",
                    [](const Context& c) -> AString
                    {

                        return aui::platform::current::name();
                    }
            },
            {
                    "os.version",
                    [](const Context& c) -> AString
                    {
                        return "unknown";
                    }
            },
            {
                    "os.arch",
                    [](const Context& c) -> AString
                    {
                        return "x86";
                    }
            },
            {
                    "natives_directory",
                    [](const Context& c) -> AString
                    {
                        if (c.profile) {
                            return *Settings::inst().gameDir / "bin" / c.profile->getUuid().toRawString();
                        }
                        return {};
                    }
            },
            {
                    "launcher_name",
                    [](const Context& c) -> AString
                    {
                        return "hackers-mc-launcher";
                    }
            },
            {
                    "launcher_version",
                    [](const Context& c) -> AString
                    {
                        return HACKERS_MC_VERSION;
                    }
            },
            {
                    "classpath",
                    [](const Context& c) -> AString
                    {
                        if (c.profile) {
                            const auto& name = *c.profile->name;
                            AString classpath;
                            for (auto& e : c.profile->getClasspath()) {
                                if (VariableHelper::checkRules(c, e.conditions)) {
                                    classpath += e.name;
                                    classpath += CLASSPATH_SEPARATOR;
                                }
                            }
                            return classpath + "versions/{}/{}.jar"_format(name, name);
                        }
                        return {};
                    }
            },
            {
                    "classpath_separator",
                    [](const Context& c) -> AString
                    {
                        return AString(1, CLASSPATH_SEPARATOR);
                    }
            },
            {
                    "auth_player_name",
                    [](const Context& c) -> AString
                    {
                        if (c.user) {
                            return c.user->username;
                        }
                        return {};
                    }
            },
            {
                    "auth_uuid",
                    [](const Context& c) -> AString
                    {
                        if (c.user) {
                            if (c.user->uuid == AUuid()) {
                                try {
                                    const_cast<AUuid&>(c.user->uuid) = auth_tricks::usernameToUuid(c.user->username);
                                    ALogger::info("VariableHelper") << "Mapped online user " << c.user->username << " uuid to " << c.user->uuid.toString();
                                } catch (const AException& e) {
                                    return "null";
                                }
                            }
                            return c.user->uuid.toRawString();
                        }
                        return {};
                    }
            },
            {
                    "auth_access_token",
                    [](const Context& c) -> AString
                    {
                        return "null";
                    }
            },
            {
                    "user_type",
                    [](const Context& c) -> AString
                    {
                        return "msa";
                    }
            },
            {
                    "version_type",
                    [](const Context& c) -> AString
                    {
                        return "release";
                    }
            },
            {
                    "version_name",
                    [](const Context& c) -> AString
                    {
                        if (c.profile) {
                            return c.profile->name;
                        }
                        return {};
                    }
            },
            {
                    "game_directory",
                    [](const Context& c) -> AString
                    {
                        return Settings::inst().gameDir;
                    }
            },
            {
                    "library_directory",
                    [](const Context& c) -> AString
                    {
                        return *Settings::inst().gameDir / "libraries";
                    }
            },
            {
                    "assets_root",
                    [](const Context& c) -> AString
                    {
                        return *Settings::inst().gameDir / "assets";
                    }
            },
            {
                    "assets_index_name",
                    [](const Context& c) -> AString
                    {
                        if (c.profile) {
                            return c.profile->getAssetsIndex();
                        }
                        return {};
                    }
            },
            {
                    "user_properties",
                    [](const Context& c) -> AString
                    {
                        return "{}";
                    }
            },
            {
                    "has_custom_resolution",
                    [](const Context& c) -> AString
                    {
                        return "true";
                    }
            },
            {
                    "resolution_width",
                    [](const Context& c) -> AString
                    {
                        return AString::number(*Settings::inst().width);
                    }
            },
            {
                    "resolution_height",
                    [](const Context& c) -> AString
                    {
                        return AString::number(*Settings::inst().height);
                    }
            },
            {
                    "is_fullscreen",
                    [](const Context& c) -> AString
                    {
                        return AString::number(*Settings::inst().isFullscreen);
                    }
            },
            {
                    "is_demo_user",
                    [](const Context& c) -> AString
                    {
                        return "false";
                    }
            },
            {
                    "auth_xuid",
                    [](const Context& c) -> AString
                    {
                        //return auth_tricks::xuid(c);
                        return "2535459390466924";
                    }
            },
            {
                    "clientid",
                    [](const Context& c) -> AString
                    {
                        return "N2U3M2FiOGYtNzQ5NS00N2RhLWIwNzgtMjk4NmY0NWZmODkx";
                        //return auth_tricks::clientId(c);
                    }
            },
    };

    
    if (auto co = m.contains(name)) {
        return co->second(c);
    }
    ALogger::warn("VariableHelper") << "Unknown variable: " << name;
    return "${" + name + "}";
}

bool VariableHelper::checkRules(const Context& c, const Rules& rules) {
    if (rules.empty()) {
        return true;
    }
    LauncherRule::Action action = LauncherRule::Action::DISALLOW;
    for (auto& v : rules) {
        bool t = true;
        for (auto& cond : v.conditions) {
            if (getVariableValue(c, cond.first) != cond.second) {
                t = false;
                break;
            }
        }
        if (t) {
            action = v.action;
        }
    }
    return action == LauncherRule::Action::ALLOW;
}

AString VariableHelper::parseVariables(const Context& c, const AString& s) {
    std::string r;
    r.reserve(s.length());

    ATokenizer tokenizer(_new<AStringStream>(s));
    try {
        for (;;) {
            tokenizer.readStringUntilUnescaped(r, '$');
            if (tokenizer.readChar() == '{') {
                auto name = tokenizer.readStringUntilUnescaped('}');
                r += getVariableValue(c, name).toStdString();
            } else {
                r += tokenizer.getLastCharacter();
            }
        }
    } catch (...) {

    }
    return r;
}
