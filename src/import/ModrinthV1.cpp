//
// Created by alex2772 on 10/24/24.
//

#include <AUI/Json/AJson.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Thread/AAsyncHolder.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Crypt/AHash.h>
#include "ModrinthV1.h"

static constexpr auto LOG_TAG = "modrinth::v1";

AJSON_FIELDS(modrinth::v1::Index::File::Hashes,
             AJSON_FIELDS_ENTRY(sha512)
             AJSON_FIELDS_ENTRY(sha1)
)

AJSON_FIELDS(modrinth::v1::Index::File,
             AJSON_FIELDS_ENTRY(path)
             AJSON_FIELDS_ENTRY(downloads)
             AJSON_FIELDS_ENTRY(hashes)
             AJSON_FIELDS_ENTRY(fileSize)
)

AJSON_FIELDS(modrinth::v1::Index,
             AJSON_FIELDS_ENTRY(game)
             AJSON_FIELDS_ENTRY(formatVersion)
             AJSON_FIELDS_ENTRY(versionId)
             AJSON_FIELDS_ENTRY(name)
             (summary, "summary", AJsonFieldFlags::OPTIONAL)
             AJSON_FIELDS_ENTRY(files)
)

bool modrinth::v1::Importer::tryLoad(const APath& extractedDir) {
    auto manifestFile = extractedDir / MANIFEST_FILE;
    if (!manifestFile.isRegularFileExists()) {
        return false;
    }
    auto json = AJson::fromStream(AFileInputStream(manifestFile));
    auto formatVersion = json["formatVersion"].asInt();
    if (formatVersion != 1) {
        ALogger::err(LOG_TAG) << "Unsupported formatVersion: " << formatVersion;
        return false;
    }
    aui::from_json(json, mManifest);
    return true;
}

void modrinth::v1::Importer::importTo(const APath& gameDir) {
    for (const auto& file : mManifest.files) {
        for (const auto& url : file.downloads) {
            ALogger::info(LOG_TAG) << "Downloading " << url;
            auto response = *ACurl::Builder(url).runAsync();
            if (response.code != ACurl::ResponseCode::HTTP_200_OK) {
                ALogger::err(LOG_TAG) << "Downloading from " << url << " failed: invalid http code";
                continue;
            }
            if (response.body.size() != file.fileSize) {
                ALogger::err(LOG_TAG) << "File size mismatch for " << url;
                continue;
            }
            if (auto actualHash = AHash::sha1(response.body).toHexString(); actualHash != file.hashes.sha1) {
                ALogger::err(LOG_TAG) << "File hash mismatch for " << url << " expected " << file.hashes.sha1 << ", actual " << actualHash;
                continue;
            }
            auto filePath = gameDir / file.path;
            filePath.parent().makeDirs();
            AFileOutputStream(filePath) << response.body;
            goto success;
        }
        throw AException("unable to resolve {}"_format(file.path));
        success:
        ALogger::info(LOG_TAG) << "Resolved: " << file.path;
    }
}
