#pragma once


#include <Model/GameProfile.h>
#include <Model/User.h>

class Launcher: public AObject {
private:
    void download(const DownloadEntry& e);

public:
    void play(const User& user, const GameProfile& profile);

signals:
    emits<AString> statusLabel;
};

