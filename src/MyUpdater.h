#pragma once
#include "AUI/Updater/AUpdater.h"
#include "AUI/Updater/GitHub.h"

class MyUpdater: public AUpdater {
public:
    static MyUpdater& inst();

protected:
    AFuture<void> downloadUpdateImpl(const APath& unpackedUpdateDir) override;
    AFuture<void> checkForUpdatesImpl() override;

private:
    AString mDownloadUrl;

    MyUpdater();
};
