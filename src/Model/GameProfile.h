#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Json/AJson.h>
#include <AUI/Common/AUuid.h>
#include <AUI/Common/AProperty.h>
#include "DownloadEntry.h"

class GameProfile {
    friend class GameProfileWindow;
public:
    class GameArg
    {
    public:
        AString name;
        AString value;
        Rules conditions;
    };

    /**
     * \brief not all arguments have it's own value
     */
    class JavaArg
    {
    public:
        AString name;
        Rules conditions;
    };

    class ClasspathEntry
    {
    public:
        AString name;
        Rules conditions;
    };

private:

    AUuid mUuid;
    AString mMainClass;
    AString mAssetsIndex;
    AString mJavaVersionName = "java-runtime-alpha";

    AVector<DownloadEntry> mDownloads;
    AVector<GameArg> mGameArgs;
    AVector<JavaArg> mJavaArgs;
    AVector<ClasspathEntry> mClasspath;


    bool mIsFullscreen = false;
    unsigned short mWindowWidth = 854;
    unsigned short mWindowHeight = 500;

    void makeClean();

public:
    static void fromJson(GameProfile& dst, const AUuid& uuid, const AString& name, const AJson& json);

    AProperty<AString> name;

    [[nodiscard]] const AUuid& getUuid() const noexcept {
        return mUuid;
    }

    [[nodiscard]] const AString& getMainClass() const noexcept {
        return mMainClass;
    }

    [[nodiscard]] const AString& getAssetsIndex() const noexcept {
        return mAssetsIndex;
    }

    [[nodiscard]] const AVector<DownloadEntry>& getDownloads() const noexcept {
        return mDownloads;
    }

    [[nodiscard]] const AVector<GameArg>& getGameArgs() const noexcept {
        return mGameArgs;
    }

    [[nodiscard]] const AVector<JavaArg>& getJavaArgs() const noexcept {
        return mJavaArgs;
    }

    [[nodiscard]] const AVector<ClasspathEntry>& getClasspath() const noexcept {
        return mClasspath;
    }

    [[nodiscard]] bool isFullscreen() const noexcept {
        return mIsFullscreen;
    }

    [[nodiscard]] unsigned short getWindowWidth() const noexcept {
        return mWindowWidth;
    }

    [[nodiscard]] unsigned short getWindowHeight() const noexcept {
        return mWindowHeight;
    }

    [[nodiscard]]
    const AString& getJavaVersionName() const noexcept {
        return mJavaVersionName;
    }

    void save();

    AJson toJson();

    static void fromName(GameProfile& dst, const AUuid& uuid, const AString& name);
};

