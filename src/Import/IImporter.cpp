//
// Created by alex2772 on 10/24/24.
//

#include <AUI/Logging/ALogger.h>
#include <AUI/Util/ARandom.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/Json/AJson.h>
#include "IImporter.h"
#include "ModrinthV1.h"
#include "AUI/Util/Archive.h"

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

    APath extractDir = APath::getDefaultPath(APath::TEMP) / "hackers-mc-" + ARandom().nextUuid();

    aui::archive::zip::read(AFileInputStream(archivePath), [&](const aui::archive::FileEntry& entry) {
        AThread::interruptionPoint();
        APath fileName = entry.name;
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
                APath dstFile = extractDir / fileName;
                dstFile.parent().makeDirs();

                try {
                    AFileOutputStream(dstFile) << *entry.open();
                } catch (...) {
                    throw AException(
                            "launcher.error.unable_to_write"_i18n.format(
                                    fileName));
                }
            }
        }
    });

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
