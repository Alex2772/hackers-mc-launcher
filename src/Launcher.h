#pragma once


#include <Model/GameProfile.h>
#include <Model/Account.h>
#include <AUI/Platform/AProcess.h>

class Launcher: public AObject {
private:
    struct ToDownload {
        AString localPath;
        AString url;
        std::size_t bytes;
    };

    [[nodiscard]]
    bool isJavaWorking() const noexcept;
    void downloadAndInstallJava();
    APath javaExecutable() const noexcept;
    void performDownload(const APath& destinationDir, const AVector<ToDownload>& toDownload);

public:
    _<AChildProcess> play(const Account& user, const GameProfile& profile, bool doUpdate = false);

signals:
    emits<AString> updateStatus;
    emits<size_t> updateDownloadedSize;
    emits<size_t> updateTotalDownloadSize;
    emits<AString> updateTargetFile;

    AString retrieveJavaManifestUrl() const;

};

