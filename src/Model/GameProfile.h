#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Json/AJson.h>
#include <AUI/Common/AUuid.h>
#include "DownloadEntry.h"

class GameProfile {
    friend class GameProfileWindow;
public:
    class GameArg
    {
    public:
        AString mName;
        AString mValue;
        Rules mConditions;
    };

    /**
     * \brief not all arguments have it's own value
     */
    class JavaArg
    {
    public:
        AString mName;
        Rules mConditions;
    };
    
private:

    AUuid mUuid;
    AString mName;
    AString mMainClass;
    AString mAssetsIndex;

    AVector<DownloadEntry> mDownloads;
    AVector<GameArg> mGameArgs;
    AVector<JavaArg> mJavaArgs;
    AStringVector mClasspath;


    bool mIsFullscreen = false;
    unsigned short mWindowWidth = 854;
    unsigned short mWindowHeight = 500;

    void makeClean();

public:
    static void fromJson(GameProfile& dst, const AUuid& uuid, const AString& name, const AJson& json);

    [[nodiscard]] const AUuid& getUuid() const {
        return mUuid;
    }

    [[nodiscard]] const AString& getName() const {
        return mName;
    }

    [[nodiscard]] const AString& getMainClass() const {
        return mMainClass;
    }

    [[nodiscard]] const AString& getAssetsIndex() const {
        return mAssetsIndex;
    }

    [[nodiscard]] const AVector<DownloadEntry>& getDownloads() const {
        return mDownloads;
    }

    [[nodiscard]] const AVector<GameArg>& getGameArgs() const {
        return mGameArgs;
    }

    [[nodiscard]] const AVector<JavaArg>& getJavaArgs() const {
        return mJavaArgs;
    }

    [[nodiscard]] const AStringVector& getClasspath() const {
        return mClasspath;
    }

    [[nodiscard]] bool isFullscreen() const {
        return mIsFullscreen;
    }

    [[nodiscard]] unsigned short getWindowWidth() const {
        return mWindowWidth;
    }

    [[nodiscard]] unsigned short getWindowHeight() const {
        return mWindowHeight;
    }

    void save();

    AJson toJson();

    static void fromName(GameProfile& dst, const AUuid& uuid, const AString& name);
};

