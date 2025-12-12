#pragma once


#include "IImporter.h"

namespace modrinth {
    static constexpr auto MANIFEST_FILE = "modrinth.index.json";
    namespace v1 {
        struct Index {
            AString game;
            int formatVersion;
            AString versionId;
            AString name;
            AString summary;

            struct File {
                AString path;
                struct Hashes {
                    AString sha512;
                    AString sha1;
                } hashes;
                AVector<AString> downloads;
                int fileSize;
            };

            AVector<File> files;
        };

        class Importer : public IImporter {
        public:
            Importer() = default;
            ~Importer() override = default;

            void importTo(const APath& gameDir) override;

            const Index& manifest() const {
                return mManifest;
            }

        protected:
            bool tryLoad(const APath& extractedDir) override;

        private:
            Index mManifest;

        };
    }
};
