//
// Created by alex2772 on 4/20/21.
//

#include <AUI/Logging/ALogger.h>
#include <Model/Settings.h>
#include <AUI/Crypt/AHash.h>
#include <AUI/Traits/iterators.h>
#include <AUI/IO/FileInputStream.h>
#include <AUI/Curl/ACurl.h>
#include "Launcher.h"


void Launcher::play(const User& user, const GameProfile& profile) {
    emit statusLabel("Scanning files to download");

    ALogger::info("== PLAY BUTTON PRESSED ==");
    ALogger::info("User: " + user.username);
    ALogger::info("Profile: " + profile.getName());

    const APath gameFolder = Settings::inst().game_folder;
    ALogger::info("Install path: " + gameFolder);

    const APath assetsJson = gameFolder["assets"]["indexes"][profile.getAssetsIndex() + ".json"];
    ALogger::info("Assets: " + assetsJson);

    // we should have asset indexes on disk in order to determine how much data will we download
    if (!assetsJson.isRegularFileExists()) {
        // if it does not exist we should find it in downloads array
        AString filepath = "assets/indexes/" + profile.getAssetsIndex() + ".json";

        // the asset is located near the end so using reverse iterator wrap
        for (const auto& download : aui::reverse_iterator_wrap(profile.getDownloads())) {
            if (download.mLocalPath == filepath) {
                // here we go. download it
                auto localFile = gameFolder.file(filepath);
                localFile.parent().makeDirs();
                _new<ACurl>(download.mUrl) >> _new<FileOutputStream>(localFile);
                break;
            }
        }
    }
    if (!assetsJson.isRegularFileExists()) {
        throw AException("Could not download asset indexes file. Try to reset game profile");
    }


    AVector<const DownloadEntry*> toDownload;
    for (auto& download : profile.getDownloads()) {
        auto localFilePath = gameFolder.file(download.mLocalPath);
        if (!localFilePath.isRegularFileExists()) {
            ALogger::info("[x] To download: " + download.mLocalPath);
        } else if (download.mHash.empty() || AHash::sha1(_new<FileInputStream>(localFilePath)).toHexString() != download.mHash) {
            ALogger::info("[#] To download: " + download.mLocalPath);
        } else {
            continue;
        }
        toDownload << &download;
    }
}

void Launcher::download(const DownloadEntry& e) {
}
