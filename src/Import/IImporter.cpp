//
// Created by alex2772 on 10/24/24.
//

#include <AUI/Logging/ALogger.h>
#include <AUI/Util/ARandom.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/Json/AJson.h>
#include "IImporter.h"
#include "Util/Zip.h"
#include "ModrinthV1.h"

static constexpr auto LOG_TAG = "ImportHelper";

namespace {

    using Factory = std::function<_unique<IImporter>()>;
    template<aui::derived_from<IImporter> T>
    Factory makeFactory() {
        return [] { return std::make_unique<T>(); };
    }
    Factory factories[] = {
        makeFactory<modrinth::v1::Importer>(),
    };
}

_unique<IImporter> IImporter::from(const APath& archivePath) {
    ALogger::info(LOG_TAG) << "Importing " << archivePath;
    unzip::File unzip = _new<AFileInputStream>(archivePath);
    unz_global_info info;
    if (unzGetGlobalInfo(unzip, &info) != UNZ_OK) {
        throw AException("unzGetGlobalInfo failed");
    }

    APath extractDir = APath::getDefaultPath(APath::TEMP) / "hackers-mc-" + ARandom().nextUuid();
    for (size_t entryIndex = 0; entryIndex < info.number_entry; ++entryIndex) {
        AThread::interruptionPoint();
        char fileNameBuf[0x400];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(unzip,
                                  &fileInfo,
                                  fileNameBuf,
                                  sizeof(fileNameBuf),
                                  nullptr,
                                  0,
                                  nullptr,
                                  0) != UNZ_OK) {
            throw AException("failed getting info for {}"_format(fileNameBuf));
        }
        APath fileName = fileNameBuf;
        ALogger::info(LOG_TAG) << "Unpacking " << fileName;
        if (!fileName.empty() && fileName != "/") {
            if (fileName.endsWith('/')) {
                // folder
                try {
                    (extractDir / fileName).makeDirs();
                } catch (const AException& e) {
                    ALogger::warn(LOG_TAG) << (e.getMessage());
                }
            } else {
                // file
                if (unzOpenCurrentFile(unzip) != UNZ_OK) {
                    throw AException(
                            "launcher.error.unpack"_i18n.format(fileName));
                }


                APath dstFile = extractDir / fileName;
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

        if ((entryIndex + 1) < info.number_entry) {
            if (unzGoToNextFile(unzip) != UNZ_OK)
                break;
        }
//        ui_threadX [v = float(entryIndex) / float(info.number_entry), progressBar] {
//            AUI_NULLSAFE(progressBar)->setValue(v);
//        };
    }

    for (const auto& factory : factories) {
        auto impl = factory();
        ALogger::info(LOG_TAG) << "Trying: " << AReflect::name(impl.get());
        if (impl->tryLoad(extractDir)) {
            impl->mExtractDir = extractDir;
            ALogger::info(LOG_TAG) << "Using: " << AReflect::name(impl.get());
            return impl;
        }
    }
    throw AException("could not find appropriate importer");
}

IImporter::~IImporter() {
    mExtractDir.removeFileRecursive();
}
