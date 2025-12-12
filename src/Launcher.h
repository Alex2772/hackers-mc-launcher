#pragma once


#include <model/GameProfile.h>
#include <model/Account.h>
#include <AUI/Platform/AProcess.h>

class Launcher: public AObject {
private:
    struct ToDownload {
        AString       localPath;
        AString       url;
        std::uint64_t bytes;
        AString       sha1;
    };

    [[nodiscard]]
    bool isJavaWorking(const AString& version) const noexcept;
    APath javaExecutable(const AString& version) const noexcept;
    void performDownload(const APath& destinationDir, const AVector<ToDownload>& toDownload);

public:
    void download(const Account& user, const GameProfile& profile, bool doUpdate);
    _<AChildProcess> play(const Account& user, const GameProfile& profile, bool doUpdate = false);

signals:
    emits<AString> updateStatus;
    emits<size_t> updateDownloadedSize;
    emits<size_t> updateTotalDownloadSize;
    emits<AString> updateTargetFile;

    void downloadAndInstallJava(const AString& version);

    AString retrieveJavaManifestUrl(const AString& version) const;
};

