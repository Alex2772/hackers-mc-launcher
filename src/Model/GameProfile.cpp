//
// Created by alex2772 on 4/15/21.
//

#include <Util/VariableHelper.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Logging/ALogger.h>
#include "GameProfile.h"
#include "DownloadEntry.h"
#include "Settings.h"

constexpr auto LOG_TAG = "GameProfile";

DownloadEntry downloadEntryFromJson(const AString& path, const AJson& v)
{
    return DownloadEntry{
            path, v["url"].asString(),
            uint64_t(v["size"].asInt()), false, v["sha1"].asString()
    };
}
/**
 * \brief Converts maven library name notation to path
 *
 *        for example:
 *					optifine:OptiFine:1.14.4_HD_U_F4
 *							 ->
 *        optifine/OptiFine/1.14.4_HD_U_F4/OptiFine-1.14.4_HD_U_F4.jar
 *
 *        Java coders so Java coders.
 *
 * \param name maven notation
 * \return meaningful path
 */
AString javaLibNameToPath(const AString& name)
{
    auto colonSplitted = name.split(':');
    if (colonSplitted.size() == 3)
    {
        colonSplitted[0].replaceAll('.', '/');
        return colonSplitted[0] + '/' + colonSplitted[1] + '/' + colonSplitted[2]
               + '/' + colonSplitted[1] + '-' + colonSplitted[2] + ".jar";
    }
    return "INVALID:" + name;
}

void GameProfile::fromJson(GameProfile& dst, const AUuid& uuid, const AString& name, const AJson& json) {
    dst.mUuid = uuid;

    bool cleanupNeeded = false;

    bool isHackersMcFormat = json.contains("hackers-mc");

    auto parseConditions = [](Rules& dst, const AJson& in)
    {
        // find positive conditions

        for (auto& r : in.asArray())
        {
            LauncherRule myRule;
            auto rule = r.asObject();
            myRule.action = rule["action"].asString() == "allow" ? LauncherRule::Action::ALLOW : LauncherRule::Action::DISALLOW;
            for (const auto& r : rule)
            {
                if (r.first == "features")
                {
                    myRule.conditions << r.second.asObject().map([](const std::pair<AString, AJson>& value) {
                        return std::pair<AString, AString>{value.first, AJson::toString(value.second) };
                    });
                }
                else if (r.first != "action")
                {
                    if (r.second.isObject())
                    {
                        myRule.conditions << r.second.asObject().map([&](const std::pair<AString, AJson>& value) {
                            return std::pair<AString, AString>(r.first + '.' + value.first, AJson::toString(value.second));
                        });
                    }
                    else
                    {
                        myRule.conditions << std::pair<AString, AString>{r.first, AJson::toString(r.second)};
                    }
                }
            }

            dst << std::move(myRule);
        }
    };
    if (isHackersMcFormat)
    {
        // hackers-mc format

        // Download entries
        for (auto& d : json["downloads"].asArray())
        {
            auto o = d.asObject();
            dst.mDownloads << downloadEntryFromJson(o["local"].asString(), o);
            dst.mDownloads.last().mExtract = o["extract"].asBool();
            if (o.contains("conditions")) {
                dst.mDownloads.last().mConditions = aui::from_json<Rules>(o["conditions"]);
            }
        }

        // game args
        for (auto& d : json["game_args"].asArray())
        {
            GameArg entry;
            entry.name = d["name"].asString();
            entry.value = d["value"].asString();

            try {
                entry.conditions = aui::from_json<Rules>(d["conditions"]);
            } catch (...) {}

            dst.mGameArgs << entry;
        }
        // java args
        for (auto& d : json["java_args"].asArray())
        {
            JavaArg entry;
            entry.name = d["name"].asString();

            try {
                entry.conditions = aui::from_json<Rules>(d["conditions"]);
            } catch (...) {}

            dst.mJavaArgs << entry;
        }

        // classpath
        for (auto& e : json["classpath"].asArray())
        {
            ClasspathEntry classpathEntry;
            classpathEntry.name = e["name"].asString();

            try {
                classpathEntry.conditions = aui::from_json<Rules>(e["conditions"]);
            } catch (...) {}

            dst.mClasspath << std::move(classpathEntry);
        }

        auto settings = json["settings"].asObject();
        dst.mWindowWidth = settings["window_width"].asInt();
        dst.mWindowHeight = settings["window_height"].asInt();
        dst.mIsFullscreen = settings["fullscreen"].asBool();

        if (json["javaVersion"].isString()) {
            dst.mJavaVersionName = json["javaVersion"].asString();
        }
    }
    else
    {
        // legacy minecraft launcher format

        // Java libraries
        for (auto& v : json["libraries"].asArray())
        {
            Rules conditions;
            if (v.asObject().contains("rules"))
            {
                parseConditions(conditions, v["rules"]);
            }

            AString name = v["name"].asString();
            try {
                if (v["downloads"].isObject()) {
                    try {
                        auto s = v["downloads"]["artifact"]["path"].asString();

                        dst.mDownloads
                                << downloadEntryFromJson("libraries/" + s,
                                                         v["downloads"]["artifact"].asObject());
                    } catch (...) {}

                    for (auto keyName : {"natives-osx", "natives-windows", "natives-linux"}) {
                        auto k = v["downloads"]["classifiers"][keyName];
                        if (k.isObject()) {
                            if (k["path"].isString()) {
                                dst.mDownloads
                                        << downloadEntryFromJson("libraries/" + k["path"].asString(), k.asObject());
                                dst.mDownloads.last().mExtract = true;
                                dst.mDownloads.last().mConditions = conditions;
                            }
                        }
                    }
                } else {
                    // maybe its a minecraft forge-style entry?
                    AString url = "https://libraries.minecraft.net/";
                    try {
                        url = v["url"].asString();
                        if (!url.endsWith("/")) {
                            url += "/";
                        }
                        dst.mDownloads << DownloadEntry{
                                "libraries/" + javaLibNameToPath(name), url + javaLibNameToPath(name), 0, false, ""
                        };
                        dst.mDownloads.last().mConditions = conditions;
                    } catch (...) {}
                }

            } catch (...) {}
            try {
                if (v["downloads"].isObject()) {
                    dst.mClasspath << ClasspathEntry{"libraries/" + v["downloads"]["artifact"]["path"].asString(), std::move(conditions)};
                } else {
                    dst.mClasspath << ClasspathEntry{"libraries/" + javaLibNameToPath(name), std::move(conditions)};
                }
            } catch (...) {}
        }

        unsigned counter = 0;


        // Game args
        {
            GameArg arg;

            auto processStringObject = [&](const AString& value)
            {
                if (counter % 2 == 0)
                {
                    arg.name = value;
                }
                else
                {
                    arg.value = value;
                    for (auto it = dst.mGameArgs.begin(); it != dst.mGameArgs.end();)
                    {
                        if (it->name == arg.name)
                        {
                            it = dst.mGameArgs.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                    dst.mGameArgs << arg;
                    arg = {};
                }

                counter += 1;
            };

            if (json.contains("minecraftArguments"))
            {
                if (dst.mGameArgs.empty()) { // should not override minecraftArguments by inheritance
                    // old-style args
                    AStringVector args = json["minecraftArguments"].asString().split(' ');
                    for (auto& s: args)
                        if (!s.empty())
                            processStringObject(s);
                }
            }
            else
            {
                auto args = json["arguments"].asObject();
                for (const auto& a : args["game"].asArray())
                {
                    if (a.isString())
                    {
                        processStringObject(a.asString());
                    }
                    else if (a.isObject())
                    {
                        // flush last arg
                        if (counter % 2 == 1)
                        {
                            dst.mGameArgs << arg;
                            arg = {};
                            counter += 1;
                        }

                        parseConditions(arg.conditions, a["rules"]);

                        // Add arguments
                        if (a["value"].isArray())
                        {
                            for (const auto& value : a["value"].asArray())
                            {
                                if (counter % 2 == 0)
                                {
                                    arg.name = value.asString();
                                }
                                else
                                {
                                    arg.value = value.asString();
                                    dst.mGameArgs << arg;
                                    arg.name.clear();
                                    arg.value.clear();
                                }
                                counter += 1;
                            }
                            if (counter % 2 == 1)
                            {
                                dst.mGameArgs << arg;
                            }

                            arg = {};
                        }
                        else if (a["value"].isString())
                        {
                            arg.name = a["value"].asString();
                            dst.mGameArgs << arg;
                            arg = {};
                        }
                    }
                }
            }
        }

        // JVM args
        try {
            auto args = json["arguments"].asObject();
            for (const auto &a : args["jvm"].asArray()) {
                if (a.isString()) {
                    dst.mJavaArgs << JavaArg{a.asString()};
                } else if (a.isObject()) {
                    Rules conditions;
                    parseConditions(conditions, a["rules"]);

                    if (a["value"].isString()) {
                        dst.mJavaArgs << JavaArg{a["value"].asString(), conditions};
                    } else if (a["value"].isArray()) {
                        for (auto &i : a["value"].asArray()) {
                            dst.mJavaArgs << JavaArg{i.asString(), conditions};
                        }
                    }
                }
            }
        } catch (...) {}

        if (dst.mJavaArgs.empty())
        {
            // add some important Java args
            dst.mJavaArgs << JavaArg{"-Djava.library.path=${natives_directory}"};
            dst.mJavaArgs << JavaArg{"-cp"};
            dst.mJavaArgs << JavaArg{"${classpath}"};
        }
        cleanupNeeded = true;

        if (json["javaVersion"].isObject()) {
            dst.mJavaVersionName = json["javaVersion"]["component"].asString();
        }
    }

    // optifine 1.15.2 classpath order fix
    if (json.contains("inheritsFrom"))
    {
        fromName(dst, uuid, json["inheritsFrom"].asString());

        // if the current profile does not have it's own main jar file, we can copy it from the inherited profile.
        // in theory, it would work recursively too.
        auto mainJarAbsolutePath = *Settings::inst().gameDir / "versions" / name / (name + ".jar");
        if (!mainJarAbsolutePath.isRegularFileExists())
        {
            APath::copy(*Settings::inst().gameDir / "versions" / dst.mName / (dst.mName + ".jar"), mainJarAbsolutePath);
        }
    }

    if (cleanupNeeded) {
        dst.makeClean();
    }

    dst.mName = name;

    if (json["mainClass"].isString())
        dst.mMainClass = json["mainClass"].asString();
    if (json["assets"].isString())
        dst.mAssetsIndex = json["assets"].asString();


    if (!isHackersMcFormat)
    {
        // client jar
        {
            auto path = "versions/" + json["id"].asString() + '/' + json["id"].asString() + ".jar";
            try {
                dst.mDownloads << downloadEntryFromJson(path, json["downloads"].asObject()["client"].asObject());
            } catch (...) {}
        }

        // Asset index
        try {
            dst.mDownloads << DownloadEntry{
                    "assets/indexes/" + dst.mAssetsIndex + ".json",
                    json["assetIndex"].asObject()["url"].asString(),
                    uint64_t(json["assetIndex"].asObject()["size"].asInt()),
                    false,
                    json["assetIndex"].asObject()["sha1"].asString()
            };
        } catch (...) {}
    }
}

void GameProfile::makeClean() {
    // remove duplicating java args
    for (auto i = 0; i < mJavaArgs.size(); ++i) {
        const auto& name = mJavaArgs[i].name;
        if (name.startsWith("--")) { // --add-exports
            continue;
        }
        mJavaArgs.erase(std::remove_if(mJavaArgs.begin() + i + 1, mJavaArgs.end(), [&name](const auto& p) {
            return p.name == name;
        }), mJavaArgs.end());
    }

    // remove duplicating game args
    for (auto i = 0; i < mGameArgs.size(); ++i) {
        const auto& name = mGameArgs[i].name;
        mGameArgs.erase(std::remove_if(mGameArgs.begin() + i + 1, mGameArgs.end(), [&name](const auto& p) {
            return p.name == name;
        }), mGameArgs.end());
    }

    // remove duplicating libraries
    for (auto i = 0; i < mClasspath.size(); ++i) {
        const auto& name = mClasspath[i].name;
        mClasspath.erase(std::remove_if(mClasspath.begin() + i + 1, mClasspath.end(), [&name](const auto& p) {
            return p.name == name;
        }), mClasspath.end());
    }
}

void GameProfile::save() {
    auto f = Settings::inst().gameDir->file("versions").file(mName).file(mName + ".hackers.json");
    f.parent().makeDirs();
    AFileOutputStream(f) << toJson();
}

AJson GameProfile::toJson() {
    AJson object;

    object["mainClass"] = mMainClass;
    object["assets"] = mAssetsIndex;
    object["javaVersion"] = mJavaVersionName;

    // export to minecraft legacy launcher format is not supported
    object["hackers-mc"] = true;

    // Downloads
    {
        AJson::Array downloads;
        for (auto& d: mDownloads) {
            AJson::Object entry;
            entry["local"] = d.mLocalPath;
            entry["url"] = d.mUrl;
            entry["size"] = int(d.mSize);
            entry["sha1"] = d.mHash;
            entry["extract"] = d.mExtract;
            entry["conditions"] = aui::to_json(d.mConditions);

            downloads << std::move(entry);
        }
        object["downloads"] = std::move(downloads);
    }

    // game args
    {
        AJson::Array gameArgs;
        for (auto& d: mGameArgs) {
            AJson::Object entry;
            entry["name"] = d.name;
            entry["value"] = d.value;

            entry["conditions"] = aui::to_json(d.conditions);

            gameArgs << std::move(entry);
        }
        object["game_args"] = std::move(gameArgs);
    }

    // java args
    AJson::Array javaArgs;
    for (auto& d : mJavaArgs)
    {
        AJson::Object entry;
        entry["name"] = d.name;

        AJson::Array conditions;

        entry["conditions"] = aui::to_json(d.conditions);

        javaArgs << entry;
    }
    object["java_args"] = javaArgs;


    // Classpath
    AJson::Array classpath;
    for (auto& lib : mClasspath)
    {
        classpath << AJson::Object{
            {"name", lib.name},
            {"conditions", aui::to_json(lib.conditions) }
        };
    }
    object["classpath"] = classpath;

    // Settings
    AJson::Object settings;

    settings["window_width"] = mWindowWidth;
    settings["window_height"] = mWindowHeight;
    settings["fullscreen"] = mIsFullscreen;

    object["settings"] = settings;

    return object;
}

void GameProfile::fromName(GameProfile& dst, const AUuid& uuid, const AString& name) {
    auto pathToConfig = Settings::inst().gameDir->file("versions").file(name).file(name + ".hackers.json");
    try {
        fromJson(dst, uuid, name, AJson::fromStream(AFileInputStream(pathToConfig)).asObject());
        return;
    } catch (const AIOException& ignored) {
    } catch (const AException& e) {
        ALogger::err("ProfileLoading") << "Failed to load (" << pathToConfig << ") profile:" << e;
    }
    fromJson(dst, uuid, name, AJson::fromStream(AFileInputStream(Settings::inst().gameDir->file("versions").file(name).file(name + ".json"))).asObject());
    dst.save();
}
