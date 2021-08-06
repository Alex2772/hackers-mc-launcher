//
// Created by alex2772 on 4/15/21.
//

#include <Util/VariableHelper.h>
#include <AUI/IO/FileOutputStream.h>
#include <AUI/IO/FileInputStream.h>
#include <AUI/Logging/ALogger.h>
#include "GameProfile.h"
#include "DownloadEntry.h"
#include "Settings.h"

DownloadEntry downloadEntryFromJson(const AString& path, const AJsonObject& v)
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

void GameProfile::fromJson(GameProfile& dst, const AUuid& uuid, const AString& name, const AJsonObject& json) {
    dst.mUuid = uuid;

    bool cleanupNeeded = false;

    bool isHackersMcFormat = json.contains("hackers-mc");

    auto parseConditions = [](Rules& dst, const AJsonElement& in)
    {
        // find positive conditions

        for (auto& r : in.asArray())
        {
            LauncherRule myRule;
            auto rule = r.asObject();
            myRule.action = rule["action"].asString() == "allow" ? LauncherRule::Action::ALLOW : LauncherRule::Action::DISALLOW;
            for (auto& r : rule)
            {
                if (r.first == "features")
                {
                    auto features = r.second.asObject();
                    for (auto& feature : features)
                    {
                        myRule.conditions << std::pair<AString, AVariant>{feature.first, feature.second.asVariant()};
                    }
                }
                else if (r.first != "action")
                {
                    if (r.second.isObject())
                    {
                        auto sub = r.second.asObject();
                        for (auto& subKey : sub)
                        {
                            myRule.conditions << std::pair<AString, AVariant>{r.first + '.' + subKey.first, subKey.second.asVariant()};
                        }
                    }
                    else
                    {
                        myRule.conditions << std::pair<AString, AVariant>{r.first, r.second.asVariant()};
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
            entry.mName = d["name"].asString();
            entry.mValue = d["value"].asString();

            try {
                entry.mConditions = aui::from_json<Rules>(d["conditions"]);
            } catch (...) {}

            dst.mGameArgs << entry;
        }
        // java args
        for (auto& d : json["java_args"].asArray())
        {
            JavaArg entry;
            entry.mName = d["name"].asString();

            try {
                entry.mConditions = aui::from_json<Rules>(d["conditions"]);
            } catch (...) {}

            dst.mJavaArgs << entry;
        }

        // classpath
        for (auto& e : json["classpath"].asArray())
        {
            dst.mClasspath << e.asString();
        }

        auto settings = json["settings"].asObject();
        dst.mWindowWidth = settings["window_width"].asInt();
        dst.mWindowHeight = settings["window_height"].asInt();
        dst.mIsFullscreen = settings["fullscreen"].asBool();
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
                        try {
                            auto k = v["downloads"]["classifiers"][keyName];

                            dst.mDownloads << downloadEntryFromJson("libraries/" + k["path"].asString(), k.asObject());
                            dst.mDownloads.last().mExtract = true;
                            dst.mDownloads.last().mConditions = conditions;
                            //dst.mClasspath << "libraries/" + k["path"].asString();
                        } catch (...) {

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
                    dst.mClasspath << "libraries/" + v["downloads"]["artifact"]["path"].asString();
                } else {
                    dst.mClasspath << "libraries/" + javaLibNameToPath(name);
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
                    arg.mName = value;
                }
                else
                {
                    arg.mValue = value;
                    for (auto it = dst.mGameArgs.begin(); it != dst.mGameArgs.end();)
                    {
                        if (it->mName == arg.mName)
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
                // old-style args
                AStringVector args = json["minecraftArguments"].asString().split(' ');
                for (auto& s : args)
                    if (!s.empty())
                        processStringObject(s);
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

                        parseConditions(arg.mConditions, a["rules"]);

                        // Add arguments
                        if (a["value"].isArray())
                        {
                            for (const auto& value : a["value"].asArray())
                            {
                                if (counter % 2 == 0)
                                {
                                    arg.mName = value.asString();
                                }
                                else
                                {
                                    arg.mValue = value.asString();
                                    dst.mGameArgs << arg;
                                    arg.mName.clear();
                                    arg.mValue.clear();
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
                            arg.mName = a["value"].asString();
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
    }

    // optifine 1.15.2 classpath order fix
    if (json.contains("inheritsFrom"))
    {
        fromName(dst, uuid, json["inheritsFrom"].asString());

        // if the current profile does not have it's own main jar file, we can copy it from the inherited profile.
        // in theory, it would work recursively too.
        auto mainJarAbsolutePath = Settings::inst().game_folder["versions"][name][name + ".jar"];
        if (!mainJarAbsolutePath.isRegularFileExists())
        {
            APath::copy(Settings::inst().game_folder["versions"][dst.mName][dst.mName + ".jar"], mainJarAbsolutePath);
        }
    }

    if (cleanupNeeded)
        dst.makeClean();

    dst.mName = name;

    if (json["mainClass"].isString())
        dst.mMainClass = json["mainClass"].asString();
    if (json["assets"].isString())
        dst.mAssetsIndex = json["assets"].asString();


    if (!isHackersMcFormat)
    {
        // again, due to Optifine 1.15.2 load order main game jar should be in the end of classpath load order.

        // client jar
        {
            auto path = "versions/" + json["id"].asString() + '/' + json["id"].asString() + ".jar";
            try {
                dst.mDownloads << downloadEntryFromJson(path, json["downloads"].asObject()["client"].asObject());
            } catch (...) {}
            dst.mClasspath << path;
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
}

void GameProfile::save() {
    auto f = Settings::inst().game_folder.file("versions").file(mName).file(mName + ".hackers.json");
    f.parent().makeDirs();
    AJson::write(_new<FileOutputStream>(f), toJson());
}

AJsonElement GameProfile::toJson() {
    AJsonObject object;

    object["mainClass"] = mMainClass;
    object["assets"] = mAssetsIndex;

    // export to minecraft legacy launcher format is not supported
    object["hackers-mc"] = true;

    // Downloads
    AJsonArray downloads;
    for (auto& d : mDownloads)
    {
        AJsonObject entry;
        entry["local"] = d.mLocalPath;
        entry["url"] = d.mUrl;
        entry["size"] = int(d.mSize);
        entry["sha1"] = d.mHash;
        entry["extract"] = d.mExtract;
        entry["conditions"] = aui::to_json(d.mConditions);

        downloads << entry;
    }
    object["downloads"] = downloads;

    // game args
    AJsonArray gameArgs;
    for (auto& d : mGameArgs)
    {
        AJsonObject entry;
        entry["name"] = d.mName;
        entry["value"] = d.mValue;

        entry["conditions"] = aui::to_json(d.mConditions);

        gameArgs << entry;
    }
    object["game_args"] = gameArgs;

    // java args
    AJsonArray javaArgs;
    for (auto& d : mJavaArgs)
    {
        AJsonObject entry;
        entry["name"] = d.mName;

        AJsonArray conditions;

        entry["conditions"] = aui::to_json(d.mConditions);

        javaArgs << entry;
    }
    object["java_args"] = javaArgs;


    // Classpath
    AJsonArray classpath;
    for (auto& lib : mClasspath)
    {
        classpath << AJsonValue(lib);
    }
    object["classpath"] = classpath;

    // Settings
    AJsonObject settings;

    settings["window_width"] = mWindowWidth;
    settings["window_height"] = mWindowHeight;
    settings["fullscreen"] = mIsFullscreen;

    object["settings"] = settings;

    return object;
}

void GameProfile::fromName(GameProfile& dst, const AUuid& uuid, const AString& name) {
    _<FileInputStream> fis;
    try {
        fis = _new<FileInputStream>(Settings::inst().game_folder.file("versions").file(name).file(name + ".hackers.json"));
    } catch (...) {
        fis = _new<FileInputStream>(Settings::inst().game_folder.file("versions").file(name).file(name + ".json"));
    }
    fromJson(dst, uuid, name, AJson::read(fis).asObject());
}
