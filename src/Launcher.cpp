//
// Created by alex2772 on 4/20/21.
//

#include <range/v3/algorithm/find_if.hpp>
#include <AUI/Logging/ALogger.h>
#include <Model/Settings.h>
#include <AUI/Crypt/AHash.h>
#include <AUI/Traits/iterators.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/Platform/AProgramModule.h>
#include <Util/VariableHelper.h>
#include "Launcher.h"
#include "AUI/Common/AException.h"
#include "Util.h"
#include "AUI/Util/Archive.h"

#include <AUI/i18n/AI18n.h>
#include <AUI/Traits/strings.h>
#include <AUI/Traits/platform.h>
#include <AUI/IO/AFileOutputStream.h>
#include <numeric>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/parallel.h>
#include <AUI/Util/AStdOutputRecorder.h>

static constexpr auto LOG_TAG = "Launcher";
static constexpr auto JAVA_VERSIONS_URL = "https://launchermeta.mojang.com/v1/products/java-runtime/2ec0cc96c44e5a76b9c8b7c39df7210883d12871/all.json";

void Launcher::download(const Account& user, const GameProfile& profile, bool doUpdate) {
    emit updateStatus("Locating Java");
    ALogger::info(LOG_TAG) << "Checking for Java " << profile.getJavaVersionName() << "...";

    if (!isJavaWorking(profile.getJavaVersionName())) {
        ALogger::info(LOG_TAG) << "Could not find a working java, trying to install";
        downloadAndInstallJava(profile.getJavaVersionName());
    }

    ALogger::info(LOG_TAG) << "Java is working";

    emit updateStatus("Scanning files to download");
    const APath gameFolder = Settings::inst().gameDir;
    const APath extractFolder = gameFolder / "bin" / profile.getUuid().toRawString();
    ALogger::info(LOG_TAG) << ("Install path: " + gameFolder);

    const APath assetsJson = gameFolder / "assets" / "indexes" / (profile.getAssetsIndex() + ".json");
    ALogger::info(LOG_TAG) << ("Assets: " + assetsJson);

    auto checkAndDownload = [&] {
        // we should have asset indexes on disk in order to determine how much data will we download
        if (!assetsJson.isRegularFileExists()) {
            // if it does not exist we should find it in downloads array
            AString filepath = "assets/indexes/" + profile.getAssetsIndex() + ".json";

            // the asset is located near the end so using reverse iterator wrap
            for (const auto& download : aui::reverse_iterator_wrap(profile.getDownloads())) {
                if (download.localPath == filepath) {
                    // here we go. download it
                    auto localFile = gameFolder.file(filepath);
                    localFile.parent().makeDirs();
                    ACurl(ACurl::Builder(download.url).withOutputStream(_new<AFileOutputStream>(localFile))).run();
                    break;
                }
            }
        }
        if (!assetsJson.isRegularFileExists()) {
            throw AException("Could not download asset indexes file. Try to reset game profile");
        }


        AVector<ToDownload> toDownload;

        // regular downloads
        for (auto& download : profile.getDownloads()) {
            auto localFilePath = gameFolder.file(download.localPath);
            if (!localFilePath.isRegularFileExists()) {
                ALogger::info(LOG_TAG) << ("[not exists   ] To download: " + download.localPath);
                toDownload << ToDownload{download.localPath, download.url, download.size, download.sha1};
                continue;
            }
            if (AHash::sha1(AFileInputStream(localFilePath)).toHexString() != download.sha1) {
                if (download.sha1.empty()) {
                    if (!doUpdate) {
                        // hash is empty. just skip if doUpdate is not requested.
                        continue;
                    }
                    ALogger::info(LOG_TAG) << ("[forced update] To download: " + download.localPath);
                    toDownload << ToDownload{download.localPath, download.url, download.size, download.sha1};
                    continue;
                }
                ALogger::info(LOG_TAG) << ("[hash mismatch] To download: " + download.localPath);
                toDownload << ToDownload{download.localPath, download.url, download.size, download.sha1};
            }
        }

        // asset downloads
        auto assets = AJson::fromStream(AFileInputStream(assetsJson));

        auto objects = assets["objects"].asObject();

        auto objectsDir = gameFolder / "assets" / "objects";

        for (auto& object : objects) {
            auto hash = object.second["hash"].asString();
            auto path = AString() + hash[0] + hash[1] + '/' + hash;
            auto local = objectsDir / path;

            if (local.isRegularFileExists()) {
                if (AHash::sha1(AFileInputStream(local)).toHexString() == hash // check file's sha1
                        ) {
                    continue;
                }
            }

            ALogger::debug(LOG_TAG) << "[asset     ] To download: " << object.first;

            toDownload << ToDownload{
                    .localPath = "assets/objects/" + path,
                    .url = "https://resources.download.minecraft.net/" + path,
                    .bytes = std::size_t(object.second["size"].asInt()),
                    .sha1 = hash,
            };
        }

        emit updateStatus("Downloading...");

        performDownload(gameFolder, toDownload);
    };

    checkAndDownload();

    emit updateTargetFile("");
    emit updateStatus("Checking...");

    checkAndDownload();

    //Util::getSettingsDir()["jre-"]

    emit updateTargetFile("");

}

_<AChildProcess> Launcher::play(const Account& user, const GameProfile& profile, bool doUpdate) {
    ALogger::info(LOG_TAG) << ("== PLAY BUTTON PRESSED ==");
    ALogger::info(LOG_TAG) << ("User: " + user.username);
    ALogger::info(LOG_TAG) << ("Profile: " + profile.name);

    download(user, profile, doUpdate);

   emit updateStatus("Preparing to launch...");

    VariableHelper::Context variableHelperContext = {
            &user,
            &profile
    };

    const APath gameFolder = Settings::inst().gameDir;
    const APath extractFolder = gameFolder / "bin" / profile.getUuid().toRawString();

    // extract necessary files
    ALogger::info(LOG_TAG) << "Extract folder: " << extractFolder;
    for (auto& d : profile.getDownloads()) {
        if (d.toExtract) {
            if (!VariableHelper::checkRules(variableHelperContext, d.conditions)) {
                continue;
            }
            ALogger::info(LOG_TAG) << ("Extracting " + d.localPath);


            aui::archive::zip::read(AFileInputStream(gameFolder / d.localPath), [&](const aui::archive::FileEntry& entry) {
                AThread::interruptionPoint();
                  auto checks = [&] {
                    // filter
                    if (entry.name.endsWith(".sha1")) {
                        return false;
                    }
                    if (entry.name.endsWith(".git")) {
                        return false;
                    }
                    if (entry.name.startsWith("META-INF/")) {
                        return false;
                    }

                    bool isDylib = entry.name.endsWith(".dylib");
                    bool isSo = entry.name.endsWith(".so");
                    bool isDll = entry.name.endsWith(".dll");

                    if (isDylib || isSo || isDll) {
                        if (entry.name.endsWith("." + AProgramModule::getDllExtension())) {
                            /*
                            if (!entry.name.endsWith("64." + Dll::getDllExtension())) {
                                return false;
                            }*/
                        } else {
                            return false;
                        }
                    }
                    return true;
                };
                if (checks()) {
                    if (!entry.name.empty() && entry.name != "/") {
                        if (entry.name.endsWith("/")) {
                            // folder
                            try {
                                extractFolder.file(entry.name).makeDirs();
                            } catch (const AException& e) {
                                ALogger::warn(LOG_TAG) << (e.getMessage());
                            }
                        } else {
                            APath dstFile = extractFolder / entry.name;
                            dstFile.parent().makeDirs();

                            try {
                                AFileOutputStream fos(dstFile);
                                fos << *entry.open();
                            } catch (...) {
                                throw AException(
                                        "launcher.error.unable_to_write"_i18n.format(
                                                entry.name));
                            }
                        }
                    }
                }
            });
        }
    }

    // java args
    AStringVector args;
    for (auto& arg : profile.getJavaArgs()) {
        if (VariableHelper::checkRules(variableHelperContext, arg.conditions)) {
            args << VariableHelper::parseVariables(variableHelperContext, arg.name);
        }
    }
    /*
-Xmx2G
-Xss1M
-XX:+UnlockExperimentalVMOptions
-XX:+UseG1GC
-XX:G1NewSizePercent=20
-XX:G1ReservePercent=20
-XX:MaxGCPauseMillis=50
-XX:G1HeapRegionSize=32M
     */

    args << profile.getMainClass();

    // game args
    for (auto& arg : profile.getGameArgs()) {
        if (VariableHelper::checkRules(variableHelperContext, arg.conditions)) {
            /*
            args << VariableHelper::parseVariables(c, arg.mName);
            if (!arg.mValue.empty()) {
                args << VariableHelper::parseVariables(c, arg.mValue);
            }*/

            if (arg.value.empty()) {
                args << VariableHelper::parseVariables(variableHelperContext, arg.name);
            } else {
                args << VariableHelper::parseVariables(variableHelperContext, arg.name) << VariableHelper::parseVariables(variableHelperContext, arg.value);
            }
        }
    }

    // fix: fabric json for some weird reason wraps its argument with spaces:
    // "-DFabricMcEmu= net.minecraft.client.main.Main "
    // while this is not an issue on linux, on windows this might confuse java's command line and cause an error
    //
    // could not find or load main class net.minecraft.client.main.Main
    //
    // workaround is to remove all whitespaces per argument.
    for (auto& arg : args) {
        arg.removeAll(' ');
    }


    auto java = javaExecutable(profile.getJavaVersionName());
    ALogger::info(LOG_TAG) << "Command line: " << (java + " " + args.join(' '));
    return AProcess::make(java, args.join(' '), Settings::inst().gameDir);
}

void Launcher::performDownload(const APath& destinationDir, const AVector<ToDownload>& toDownload) {
    std::size_t totalDownloadBytes = 0;
    for (const auto& d : toDownload) {
        totalDownloadBytes += d.bytes;
    }
    emit updateTotalDownloadSize(totalDownloadBytes);

    if (!toDownload.empty()) {
        ALogger::info(LOG_TAG) << ("== BEGIN DOWNLOAD ==");
    }
    std::atomic_size_t downloadedBytes = 0;
    aui::parallel(toDownload.begin(), toDownload.end(), [&](const AVector<ToDownload>::const_iterator& begin, const AVector<ToDownload>::const_iterator& end) {
        for (const auto& d : aui::range(begin, end)) {
            auto local = destinationDir / d.localPath;
            ALogger::info(LOG_TAG) << ("Downloading: " + d.url + " > " + local);
            emit updateTargetFile(d.localPath);
            local.parent().makeDirs();


            lolWatTryAgain:
            auto response = ACurl::Builder(d.url).runBlocking();
            if (int(response.code) == 0) {
                goto lolWatTryAgain;
            }
            if (response.code != ACurl::ResponseCode::HTTP_200_OK) {
                throw AException("unable to download file {}: http code {}"_format(d.url, response.code));
            }

            downloadedBytes += response.body.size();
            using namespace std::chrono;
            using namespace std::chrono_literals;
            const auto now = high_resolution_clock::now();
            if (static std::atomic lastUpdate = now; now - lastUpdate.load() >= 500ms) {
                emit updateDownloadedSize(downloadedBytes);
                lastUpdate = now;
            }

            if (d.sha1.empty()) {
                //ALogger::warn(LOG_TAG) << "Hash is missing for file " << d.url;
            } else {
                if (auto actualHash = AHash::sha1(response.body).toHexString(); actualHash != d.sha1) {
                    throw AException("file corrupted {} (expected {}, actual {})"_format(d.url, d.sha1, actualHash));
                }
            }
            AFileOutputStream(local) << response.body;
#ifdef AUI_PLATFORM_UNIX
            local.chmod(0777); // avoid unwanted "permission denied" on *nix
#endif
        }
    }).waitForAll();
}


bool Launcher::isJavaWorking(const AString& version) const noexcept {
    auto pathToJavaExecutable = javaExecutable(version);
    try {
        auto process = AProcess::make(pathToJavaExecutable, "-version");
        auto output = _new<AStdOutputRecorder>(process);
        process->run(ASubProcessExecutionFlags::MERGE_STDOUT_STDERR);

        if (process->waitForExitCode() != 0) {
            throw AException("nonzero exit code");
        }

        auto fullOutput = AString::fromLatin1({output->stdoutBuffer().data(), output->stdoutBuffer().size()});
        ALogger::info(LOG_TAG) << "Java version output: " << fullOutput.substr(0, fullOutput.find('\n'));
        return true;
    } catch (const AException& e) {
        ALogger::warn(LOG_TAG) << "Java does not work: " << e;
        ALogger::warn(LOG_TAG) << pathToJavaExecutable << " -- broken";
        return false;
    }
}

void Launcher::downloadAndInstallJava(const AString& version) {
    AString manifestUrl = retrieveJavaManifestUrl(version);

    emit updateStatus("Downloading Java...");
    auto manifest = AJson::fromBuffer(ACurl::Builder(manifestUrl).runBlocking().body);
    auto jvmDir = Util::launcherDir() / "jvm" / version;
    jvmDir.removeFileRecursive().makeDirs();

    AVector<ToDownload> toDownload;
    for (const auto&[localPath, file] : manifest["files"].asObject()) {
        if (file["type"].asString() == "file") {
            auto raw = file["downloads"]["raw"].asObject();
            toDownload << ToDownload{
                localPath,
                raw["url"].asString(),
                std::size_t(raw["size"].asInt()),
                raw["sha1"].asString()
            };
        }
    }
    performDownload(jvmDir, toDownload);
    if (!isJavaWorking(version)) {
        throw AException("fresh installed java does not work - giving up");
    }
}

AString Launcher::retrieveJavaManifestUrl(const AString& version) const {
    auto allJavaVersions = AJson::fromBuffer(ACurl::Builder(JAVA_VERSIONS_URL).runBlocking().body);

    const char* platformName = nullptr;
    if constexpr (aui::platform::current::is_windows()) {
        if constexpr (aui::platform::is_x86_64) {
            platformName = "windows-x64";
        } else if constexpr (aui::platform::is_x86) {
            platformName = "windows-x86";
        } else if constexpr (aui::platform::is_arm64) {
            platformName = "windows-arm64";
        }
    } else if constexpr (aui::platform::current::is_apple()) {
        platformName = "mac-os";
    } else if constexpr (aui::platform::current::is_unix()) {
        if constexpr (aui::platform::is_64_bit) {
            platformName = "linux";
        } else {
            platformName = "linux-i386";
        }
    }
    const auto& platformVersions = allJavaVersions[platformName].asObject();
    const auto* targetVersion = &platformVersions[version];
    if (targetVersion->asArray().empty()) {
        // they don't support this particular version? (that's actually the case on windows-arm64)
        // then try to use at least some non-empty version
        auto it = ranges::find_if(platformVersions, [](const auto& version) -> bool {
            return !version.second.asArray().empty();
        });
        if (it == platformVersions.end()) {
            throw AException("unable to find java for platform {}"_format(platformName));
        }
        targetVersion = &it->second;
        ALogger::warn(LOG_TAG) << "Unable to find java for platform " << platformName << " '" << version << "', using '" << it->first << "' instead";
    }

    return (*targetVersion)[0]["manifest"]["url"].asString();
}

APath Launcher::javaExecutable(const AString& version) const noexcept {
    auto path = Util::launcherDir() / "jvm" / version / "bin" / "java";
    if constexpr (aui::platform::current::is_windows()) {
        path += "w.exe"; // javaw.exe
    }
    return path;
}
