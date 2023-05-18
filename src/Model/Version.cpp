//
// Created by Alex2772 on 4/14/2022.
//

#include <AUI/Curl/ACurl.h>
#include <AUI/Json/AJson.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/ARandom.h>
#include <AUI/IO/AFileInputStream.h>
#include "Version.h"
#include "Settings.h"

static constexpr auto LOG_TAG = "Version";

AVector<Version> Version::fetchAll() {
    auto versionManifest = AJson::fromBuffer(ACurl::Builder("https://launchermeta.mojang.com/mc/game/version_manifest.json").toByteBuffer());
    const auto& jsonVersionArray = versionManifest["versions"].asArray();
    AVector<Version> versionModel;
    versionModel.reserve(jsonVersionArray.size());
    for (auto& version : jsonVersionArray) {
        versionModel.push_back({
            version["id"].asString(),
            version["url"].asString(),
            AEnumerate<VersionType>::byName(version["type"].asString()),
        });
    }
    return versionModel;
}

GameProfile Version::import() const {
    GameProfile p;
    auto versionDir = Settings::inst().gameDir / "versions" / id;
    auto file = versionDir / (id + ".json");

    if (versionDir.isDirectoryExists()) {
        versionDir.removeFileRecursive();
    }
    versionDir.makeDirs();

    ALogger::info(LOG_TAG) << "Importing " << url;
    ACurl(ACurl::Builder(url).withOutputStream(_new<AFileOutputStream>(file))).run();
    static ARandom r;
    GameProfile::fromJson(p, r.nextUuid(), id, AJson::fromStream(AFileInputStream(file)).asObject());
    p.save();
    ALogger::info(LOG_TAG) << "Imported " << p.getName();
    return p;
}
