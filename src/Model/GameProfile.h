#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Json/AJsonElement.h>
#include "DownloadEntry.h"

class GameProfile {
public:
    class GameArg
    {
    public:
        AString mName;
        AString mValue;
        AVector<std::pair<AString, AVariant>> mConditions;
    };

    /**
     * \brief not all arguments have it's own value
     */
    class JavaArg
    {
    public:
        AString mName;
        AVector<std::pair<AString, AVariant>> mConditions;
    };
    
private:

    AString mName;
    AString mMainClass;
    AString mAssetsIndex;

    AVector<DownloadEntry> mDownloads;
    AVector<GameArg> mGameArgs;
    AVector<JavaArg> mJavaArgs;
    AVector<AString> mClasspath;


    bool mIsFullscreen = false;
    unsigned short mWindowWidth = 854;
    unsigned short mWindowHeight = 500;

    void makeClean();

public:
    static void fromJson(GameProfile& dst, const AString& name, const AJsonObject& json);
};

