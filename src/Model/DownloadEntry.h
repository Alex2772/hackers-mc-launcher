#pragma once


#include <AUI/Common/AString.h>

struct DownloadEntry {

    AString mLocalPath;
    AString mUrl;
    uint64_t mSize = 0;
    bool mExtract = false;
    AString mHash;
};

