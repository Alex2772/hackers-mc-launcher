//
// Created by alex2772 on 4/20/21.
//

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
#include <unzip.h>
#include "Launcher.h"
#include "Util.h"
#include "Util/Zip.h"

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

_<AChildProcess> Launcher::play(const Account& user, const GameProfile& profile, bool doUpdate) {
    emit updateStatus("Locating Java");

    ALogger::info(LOG_TAG) << ("== PLAY BUTTON PRESSED ==");
    ALogger::info(LOG_TAG) << ("User: " + user.username);
    ALogger::info(LOG_TAG) << ("Profile: " + profile.getName());

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
                if (download.mLocalPath == filepath) {
                    // here we go. download it
                    auto localFile = gameFolder.file(filepath);
                    localFile.parent().makeDirs();
                    ACurl(ACurl::Builder(download.mUrl).withOutputStream(_new<AFileOutputStream>(localFile))).run();
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
            auto localFilePath = gameFolder.file(download.mLocalPath);
            if (!localFilePath.isRegularFileExists()) {
                ALogger::info(LOG_TAG) << ("[x] To download: " + download.mLocalPath);
            } else if (doUpdate && (download.mHash.empty() ||
                                    AHash::sha1(_new<AFileInputStream>(localFilePath)).toHexString() !=
                                    download.mHash)) {
                ALogger::info(LOG_TAG) << ("[#] To download: " + download.mLocalPath);
            } else {
                continue;
            }

            toDownload << ToDownload{download.mLocalPath, download.mUrl, download.mSize, download.mHash};
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
                if (!doUpdate
                    || AHash::sha1(_new<AFileInputStream>(local)).toHexString() == hash // check file's sha1
                        ) {
                    continue;
                }
            }

            ALogger::info(LOG_TAG) << ("[A] To download: " + object.first);

            toDownload << ToDownload{
                    "assets/objects/" + path,
                    "http://resources.download.minecraft.net/" + path,
                    std::size_t(object.second["size"].asInt())
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
    emit updateStatus("Preparing to launch...");

    VariableHelper::Context c = {
            &user,
            &profile
    };

    // extract necessary files
    ALogger::info(LOG_TAG) << "Extract folder: " << extractFolder;
    for (auto& d : profile.getDownloads()) {
        if (d.mExtract) {
            if (!VariableHelper::checkRules(c, d.mConditions)) {
                continue;
            }
            ALogger::info(LOG_TAG) << ("Extracting " + d.mLocalPath);


            unzip::File unzip = _new<AFileInputStream>(gameFolder / d.mLocalPath);
            unz_global_info info;
            if (unzGetGlobalInfo(unzip, &info) != UNZ_OK) {
                ALogger::warn(LOG_TAG) << "unzGetGlobalInfo failed for " << d.mLocalPath;
                continue;
            }
            for (size_t entryIndex = 0; entryIndex < info.number_entry; ++entryIndex) {
                AThread::interruptionPoint();
                char fileNameBuf[0x400];
                unz_file_info fileInfo;
                if (unzGetCurrentFileInfo(unzip, &fileInfo, fileNameBuf, sizeof(fileNameBuf), nullptr, 0, nullptr,
                                          0) != UNZ_OK) {
                    ALogger::warn(LOG_TAG) << "unzGetGlobalInfo failed for " << d.mLocalPath << "/" << fileNameBuf;
                    break;
                }
                APath fileName = AString::fromLatin1(fileNameBuf);

                auto checks = [&] {
                    // filter
                    if (fileName.endsWith(".sha1")) {
                        return false;
                    }
                    if (fileName.endsWith(".git")) {
                        return false;
                    }
                    if (fileName.startsWith("META-INF/")) {
                        return false;
                    }

                    bool isDylib = fileName.endsWith(".dylib");
                    bool isSo = fileName.endsWith(".so");
                    bool isDll = fileName.endsWith(".dll");

                    if (isDylib || isSo || isDll) {
                        if (fileName.endsWith("." + AProgramModule::getDllExtension())) {
                            /*
                            if (!fileName.endsWith("64." + Dll::getDllExtension())) {
                                return false;
                            }*/
                        } else {
                            return false;
                        }
                    }
                    return true;
                };
                if (checks()) {
                    if (!fileName.empty() && fileName != "/") {
                        if (fileName.endsWith('/')) {
                            // folder
                            try {
                                extractFolder.file(fileName).makeDirs();
                            } catch (const AException& e) {
                                ALogger::warn(LOG_TAG) << (e.getMessage());
                            }
                        } else {
                            // file
                            if (unzOpenCurrentFile(unzip) != UNZ_OK) {
                                throw AException(
                                        "launcher.error.unpack"_i18n.format(fileName));
                            }


                            APath dstFile = extractFolder / fileName;
                            dstFile.parent().makeDirs();

                            _<AFileOutputStream> fos;
                            try {
                                fos = _new<AFileOutputStream>(dstFile);
                            } catch (...) {
                                unzCloseCurrentFile(unzip);
                                throw AException(
                                        "launcher.error.unable_to_write"_i18n.format(
                                                fileName));
                            }

                            uint64_t total = 0;
                            char buf[0x800];
                            for (int read; (read = unzReadCurrentFile(unzip, buf, sizeof(buf))) > 0;) {
                                fos->write(buf, read);
                                total += read;
                            }

                            unzCloseCurrentFile(unzip);
                        }
                    }
                }
                if ((entryIndex + 1) < info.number_entry) {
                    if (unzGoToNextFile(unzip) != UNZ_OK)
                        break;
                }
            }
        }
    }

    // java args
    AStringVector args;
    for (auto& arg : profile.getJavaArgs()) {
        if (VariableHelper::checkRules(c, arg.conditions)) {
            args << VariableHelper::parseVariables(c, arg.name);
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
        if (VariableHelper::checkRules(c, arg.conditions)) {
            /*
            args << VariableHelper::parseVariables(c, arg.mName);
            if (!arg.mValue.empty()) {
                args << VariableHelper::parseVariables(c, arg.mValue);
            }*/

            if (arg.value.empty()) {
                args << VariableHelper::parseVariables(c, arg.name);
            } else {
                args << VariableHelper::parseVariables(c, arg.name) + " " + VariableHelper::parseVariables(c, arg.value);
            }
        }
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

            AByteBuffer rawFileBlob;
            rawFileBlob.reserve(d.bytes);

            ACurl(ACurl::Builder(d.url).withWriteCallback([&](AByteBufferView b) {
                rawFileBlob << b;
                downloadedBytes += b.size();
                if (AWindow::isRedrawWillBeEfficient()) {
                    emit updateDownloadedSize(downloadedBytes);
                }
                return b.size();
            })).run();
            if (d.hash.empty()) {
                //ALogger::warn(LOG_TAG) << "Hash is missing for file " << d.url;
            } else {
                if (AHash::sha1(rawFileBlob).toHexString() != d.hash) {
                    throw AException("file corrupted {}"_format(d.url));
                }
            }
            AFileOutputStream(local) << rawFileBlob;
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

        auto fullOutput = AString::fromLatin1(output->stdoutBuffer());
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
    auto manifest = AJson::fromBuffer(ACurl::Builder(manifestUrl).toByteBuffer());
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
    auto allJavaVersions = AJson::fromBuffer(ACurl::Builder(JAVA_VERSIONS_URL).toByteBuffer());

    const char* platformName = nullptr;
    if constexpr (aui::platform::current::is_windows()) {
        if constexpr (aui::platform::is_64_bit) {
            platformName = "windows-x64";
        } else {
            platformName = "windows-x64";
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

    return allJavaVersions[platformName][version][0]["manifest"]["url"].asString();
}

APath Launcher::javaExecutable(const AString& version) const noexcept {
    auto path = Util::launcherDir() / "jvm" / version / "bin" / "java";
    if constexpr (aui::platform::current::is_windows()) {
        path += "w.exe"; // javaw.exe
    }
    return path;
}
