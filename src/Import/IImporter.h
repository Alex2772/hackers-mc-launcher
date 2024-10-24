#pragma once


#include <AUI/IO/APath.h>

class IImporter: public AObject {
public:
    static _unique<IImporter> from(const APath& archivePath);
    virtual ~IImporter();

    virtual void importTo(const APath& gameDir) = 0;

protected:
    APath mExtractDir;

    virtual bool tryLoad(const APath& extractedDir) = 0;
};
