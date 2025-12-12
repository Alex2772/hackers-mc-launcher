#include <gmock/gmock.h>
#include <AUI/Common/AString.h>
#include "Launcher.h"
#include "model/Version.h"
#include "model/GameProcess.h"
#include "TestUtil.h"
#include "Util.h"
#include "model/Settings.h"
#include "model/State.h"
#include "source/LegacyLauncherJsonSource.h"

#include <chrono>
#include <AUI/Thread/AThreadPool.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Common/ATimer.h>
#include <AUI/Util/AStdOutputRecorder.h>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/find_if.hpp>

class MinecraftLaunch : public ::testing::Test {
protected:
    void SetUp() override {
        Test::SetUp();
        if (std::getenv("CI")) {
            GTEST_SKIP();
        }
        TestUtil::prepareApp();
        AThreadPool::global().setWorkersCount(4);
    }

    void launch(AStringView version) {
        auto launcher = _new<Launcher>();
        Account account = { AUuid(), "Player", false };

        auto targetVersion = importVanillaVersion(version);
        handleGameProcess(launcher->play(account, targetVersion.import(), false));
    }

    void launchFromTestData(AStringView version, AStringView baseVersion) {
        auto launcher = _new<Launcher>();
        Account account = { AUuid(), "Player", false };

        {
            auto vanilla = importVanillaVersion(baseVersion);
            auto process = launcher->play(account, vanilla.import(), false);
        }
        {
            auto testData = TestUtil::testDataDir() / "{}.json"_format(version);
            if (!testData.isRegularFileExists()) {
                throw AException("launchFromTestData: missing test data: {}"_format(testData));
            }
            auto versionDir = Settings::inst().gameDir / "versions" / version;
            versionDir.makeDirs();
            APath::copy(testData, versionDir / "{}.json"_format(version));
        }
        auto state = _new<State>();
        LegacyLauncherJsonSource::load(*state);
        auto it = ranges::find_if(*state->profile.list, [&](const _<GameProfile>& profile) {
            return profile->name == version;
        });
        ASSERT_NE(it, state->profile.list->end());
        handleGameProcess(launcher->play(account, **it, false));
    }

private:
    void handleGameProcess(_<AChildProcess> process) {
        auto game = _new<GameProcess>();
        game->process = process;

        auto recorder = _new<AStdOutputRecorder>(process);

        process->run(ASubProcessExecutionFlags::MERGE_STDOUT_STDERR);

        auto testFinished = _new<bool>(false);

        AObject::connect(process->finished, process, [=] {
            ALogger::info("GameOutput") << AString::fromUtf8(recorder->stdoutBuffer());
            if (recorder->stdoutBuffer().size() < 10) {
                FAIL() << "game did not output anything";
            }
            if (!*testFinished) {
                FAIL() << "game has crashed";
            }
        });

        using namespace std::chrono;
        using namespace std::chrono_literals;

        // give a game 10 seconds to load
        auto timer = _new<ATimer>(10s);
        AObject::connect(timer->fired, timer, [&] {
            *testFinished = true;
            process->kill();
        });
        timer->start();

        while (!*testFinished) {
            AThread::processMessages();
        }

        *testFinished = true;
        if (process->waitForExitCode() != 0) {
            FAIL() << "game has crashed";
        }

        AThread::processMessages();
    }
    Version importVanillaVersion(AStringView version) {
        static auto allVersions = Version::fetchAll();
        auto targetVersion = std::find_if(allVersions.begin(), allVersions.end(), [&](const Version& v) {
            return v.id == version;
        });
        if (targetVersion == allVersions.end()) {
            throw AException("unable to find vanilla version {}"_format(version));
        }
        return *targetVersion;
    }

};

TEST_F(MinecraftLaunch, v1_21) { launch("1.21"); }

TEST_F(MinecraftLaunch, v1_20_1) { launch("1.20.1"); }

TEST_F(MinecraftLaunch, v1_20_1_fabric) { launchFromTestData("fabric-loader-0.16.7-1.20.1", "1.20.1"); }

TEST_F(MinecraftLaunch, v1_20) { launch("1.20"); }

TEST_F(MinecraftLaunch, v1_19_2) { launch("1.19.2"); }

TEST_F(MinecraftLaunch, v1_19) { launch("1.19"); }

TEST_F(MinecraftLaunch, v1_18_2) { launch("1.18.2"); }

TEST_F(MinecraftLaunch, v1_12_2) { launch("1.12.2"); }

TEST_F(MinecraftLaunch, v1_8_9) { launch("1.8.9"); }

TEST_F(MinecraftLaunch, v1_7_10) { launch("1.7.10"); }

TEST_F(MinecraftLaunch, v1_6_4) { launch("1.7.10"); }

TEST_F(MinecraftLaunch, v1_5_2) { launch("1.5.2"); }

TEST_F(MinecraftLaunch, v1_4_7) { launch("1.4.7"); }

TEST_F(MinecraftLaunch, v1_3_2) { launch("1.3.2"); }

TEST_F(MinecraftLaunch, v1_2_5) { launch("1.2.5"); }

TEST_F(MinecraftLaunch, v1_0) { launch("1.0"); }

TEST_F(MinecraftLaunch, vb1_8_1) { launch("b1.8.1"); }

TEST_F(MinecraftLaunch, vb1_0) { launch("b1.0"); }

TEST_F(MinecraftLaunch, va1_2_6) { launch("a1.2.6"); }

TEST_F(MinecraftLaunch, va1_1_0) { launch("a1.1.0"); }

TEST_F(MinecraftLaunch, vrd_161348) { launch("rd-161348"); }
