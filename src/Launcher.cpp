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

#include <AUI/i18n/AI18n.h>
#include <AUI/Traits/strings.h>
#include <AUI/Traits/platform.h>
#include <AUI/IO/AFileOutputStream.h>
#include <numeric>
#include <AUI/Util/kAUI.h>

static constexpr auto LOG_TAG = "Launcher";

_<AChildProcess> Launcher::play(const Account& user, const GameProfile& profile, bool doUpdate) {
    emit updateStatus("Locating Java");

    ALogger::info(LOG_TAG) << ("== PLAY BUTTON PRESSED ==");
    ALogger::info(LOG_TAG) << ("User: " + user.username);
    ALogger::info(LOG_TAG) << ("Profile: " + profile.getName());

    ALogger::info(LOG_TAG) << "Checking for Java...";

    if (!isJavaWorking()) {
        ALogger::info(LOG_TAG) << "Could not find a working java, trying to install";
        downloadAndInstallJava();
    }

    ALogger::info(LOG_TAG) << "Java is working";

    emit updateStatus("Scanning files to download");
    const APath gameFolder = Settings::inst().gameDir;
    const APath extractFolder = gameFolder["bin"][profile.getUuid().toRawString()];
    ALogger::info(LOG_TAG) << ("Install path: " + gameFolder);

    const APath assetsJson = gameFolder["assets"]["indexes"][profile.getAssetsIndex() + ".json"];
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

            toDownload << ToDownload{download.mLocalPath, download.mUrl, download.mSize};
        }

        // asset downloads
        auto assets = AJson::fromStream(AFileInputStream(assetsJson));

        auto objects = assets["objects"].asObject();

        auto objectsDir = gameFolder["assets"]["objects"];

        for (auto& object : objects) {
            auto hash = object.second["hash"].asString();
            auto path = AString() + hash[0] + hash[1] + '/' + hash;
            auto local = objectsDir[path];

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
    for (auto& d : profile.getDownloads()) {
        if (d.mExtract) {
            if (!VariableHelper::checkRules(c, d.mConditions)) {
                continue;
            }
            ALogger::info(LOG_TAG) << ("Extracting " + d.mLocalPath);

            struct z {
                unzFile unz;
                z(unzFile unz): unz(unz) {

                }
                ~z() {
                    unzClose(unz);
                }
                operator unzFile() const {
                    return unz;
                }
            };

            z unzip = unzOpen(gameFolder[d.mLocalPath].toStdString().c_str());
            unz_global_info info;
            if (unzGetGlobalInfo(unzip, &info) != UNZ_OK) {
                throw AException("launcher.error.invalid_zip"_as.format(d.mLocalPath));
            }
            for (size_t entryIndex = 0; entryIndex < info.number_entry; ++entryIndex) {
                char fileNameBuf[0x400];
                unz_file_info fileInfo;
                if (unzGetCurrentFileInfo(unzip, &fileInfo, fileNameBuf, sizeof(fileNameBuf), nullptr, 0, nullptr,
                                          0) != UNZ_OK) {
                    throw AException("launcher.error.invalid_zip"_as.format(d.mLocalPath));
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


                            APath dstFile = extractFolder.file(fileName);
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
        if (VariableHelper::checkRules(c, arg.mConditions)) {
            args << VariableHelper::parseVariables(c, arg.mName);
        }
    }

    args << profile.getMainClass();

    // game args
    for (auto& arg : profile.getGameArgs()) {
        if (VariableHelper::checkRules(c, arg.mConditions)) {
            /*
            args << VariableHelper::parseVariables(c, arg.mName);
            if (!arg.mValue.empty()) {
                args << VariableHelper::parseVariables(c, arg.mValue);
            }*/

            if (arg.mValue.empty()) {
                args << VariableHelper::parseVariables(c, arg.mName);
            } else {
                args << VariableHelper::parseVariables(c, arg.mName) + "=" + VariableHelper::parseVariables(c, arg.mValue);
            }
        }
    }

    auto java = javaExecutable();
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
    std::size_t downloadedBytes;
    for (const auto& d : toDownload) {
        auto local = destinationDir[d.localPath];
        ALogger::info(LOG_TAG) << ("Downloading: " + d.url + " > " + local);
        emit updateTargetFile(d.localPath);
        local.parent().makeDirs();
        AFileOutputStream fos(local);
        ACurl curl(ACurl::Builder(d.url).withWriteCallback([&](AByteBufferView b) {
            fos << b;
            downloadedBytes += b.size();
            if (AWindow::isRedrawWillBeEfficient()) {
                emit updateDownloadedSize(downloadedBytes);
            }
            return b.size();
        }));
        curl.run();
    }
}


bool Launcher::isJavaWorking() const noexcept {
    try {
        auto process = AProcess::make(javaExecutable(), "-version");
        process->run(ASubProcessExecutionFlags::MERGE_STDOUT_STDERR);

        if (process->waitForExitCode() != 0) {
            ALogger::info(LOG_TAG) << "Java does not work: nonzero exit code";
        }

        auto fullOutput = AString::fromLatin1(AByteBuffer::fromStream(process->getStdOutStream()));
        ALogger::info(LOG_TAG) << "Java version output: " << fullOutput.mid(0, fullOutput.find('\n'));
    } catch (...) {
        return false;
    }
}

void Launcher::downloadAndInstallJava() {
    AString manifestUrl = retrieveJavaManifestUrl();

    emit updateStatus("Downloading Java...");
    auto manifest = AJson::fromBuffer(ACurl::Builder(manifestUrl).toByteBuffer());
    auto jvmDir = Util::launcherDir()["jvm"];
    jvmDir.removeFileRecursive().makeDirs();

    AVector<ToDownload> toDownload;
    for (const auto&[localPath, file] : manifest["files"].asObject()) {
        if (file["type"].asString() == "file") {
            auto raw = file["downloads"]["raw"].asObject();
            toDownload << ToDownload{
                localPath,
                raw["url"].asString(),
                std::size_t(raw["size"].asInt())
            };
        }
    }
    performDownload(jvmDir, toDownload);
    if (!isJavaWorking()) {
        throw AException("fresh installed java does not work - giving up");
    }
}

AString Launcher::retrieveJavaManifestUrl() const {
    // TODO linux
    auto json = AJson::fromBuffer(ACurl::Builder("http://launchermeta.mojang.com/v1/products/launcher/d03cf0cf95cce259fa9ea3ab54b65bd28bb0ae82/windows-x86.json").toByteBuffer());
    auto manifest = json[aui::platform::is_64_bit ? "jre-x64" : "jre-x86"][0]["manifest"].asObject();
    auto manifestUrl = manifest["url"].asString();
    if (manifestUrl.empty()) {
        throw AException("could not locate package manifest location");
    }
    return manifestUrl;
}

APath Launcher::javaExecutable() const noexcept {
    auto path = Util::launcherDir()["jvm"]["bin"]["java"];
    if constexpr (aui::platform::current::is_windows()) {
        path += ".exe";
    }
    return path;
}
