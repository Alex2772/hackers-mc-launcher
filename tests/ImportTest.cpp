//
// Created by alex2772 on 10/24/24.
//


#include <gtest/gtest.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include "Import/IImporter.h"
#include "Import/ModrinthV1.h"

static const auto DATA_DIR = APath::workingDir() / "data";
class Import : public ::testing::Test {
protected:
    void SetUp() override {
        Test::SetUp();
        do_once {
            ALogger::info("ImportTest") << "Note: test data dir" << DATA_DIR;
            ALogger::info("ImportTest") << "Note: fake game dir" << mGameDir;
        }
        mGameDir.removeFileRecursive();
        mGameDir.makeDirs();
    }

    void TearDown() override {
        Test::TearDown();
        mGameDir.removeFileRecursive();
    }

    APath mGameDir = APath::workingDir() / "fake_game_dir";
};

TEST_F(Import, modrinth) {
    auto importer = IImporter::from(DATA_DIR / "1.mrpack");
    ASSERT_NE(importer, nullptr);
    auto importerImpl = dynamic_cast<modrinth::v1::Importer*>(importer.get());
    ASSERT_NE(importerImpl, nullptr);
    const auto& manifest = importerImpl->manifest();
    EXPECT_EQ(manifest.game, "minecraft");
    EXPECT_EQ(manifest.versionId, "1.0.0");
    EXPECT_EQ(manifest.files.size(), 81);
    EXPECT_NO_THROW(importer->importTo(mGameDir));
    EXPECT_GE(mGameDir.listDir().size(), 1);
}