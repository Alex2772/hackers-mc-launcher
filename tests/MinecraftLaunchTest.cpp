#include <gmock/gmock.h>
#include <AUI/Common/AString.h>
#include "Launcher.h"
#include "Model/Version.h"
#include "Model/GameProcess.h"
#include "TestUtil.h"
#include <chrono>
#include <AUI/Thread/AThreadPool.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Common/ATimer.h>
#include <AUI/Util/AStdOutputRecorder.h>

class MinecraftLaunch : public ::testing::Test {
protected:
    void SetUp() override {
        Test::SetUp();
        TestUtil::prepareApp();
        AThreadPool::global().setWorkersCount(4);
    }

    void launch(const AString& version) {
        auto launcher = _new<Launcher>();

        Account account = {
            AUuid(),
            "Player",
            false
        };

        auto allVersions = Version::fetchAll();
        auto targetVersion = std::find_if(allVersions.begin(), allVersions.end(), [&](const Version& v) {
            return v.id == version;
        });
        if (targetVersion == allVersions.end()) {
            FAIL() << "Unknown version: " << version;
        }

        auto process = launcher->play(
                account,
                targetVersion->import(),
                true
        );

        auto game = _new<GameProcess>();
        game->process = process;

        auto recorder = _new<AStdOutputRecorder>(process);

        process->run(ASubProcessExecutionFlags::MERGE_STDOUT_STDERR);

        bool testFinished = false;

        AObject::connect(process->finished, process, [&] {
            ALogger::info("GameOutput") << AString::fromUtf8(recorder->stdoutBuffer());
            if (!testFinished) {
                FAIL() << "game has crashed";
            }
        });

        using namespace std::chrono;
        using namespace std::chrono_literals;

        // give a game 10 seconds to load
        auto timer = _new<ATimer>(10s);
        AObject::connect(timer->fired, timer, [&] {
            testFinished = true;
            process->kill();
        });
        timer->start();

        while (!testFinished) {
            AThread::processMessages();
        }

        testFinished = true;
        process->waitForExitCode();

        AThread::processMessages();
    }
};

TEST_F(MinecraftLaunch, v1_21) {
  launch("1.21");
}
TEST_F(MinecraftLaunch, v1_20) {
  launch("1.20");
}
TEST_F(MinecraftLaunch, v1_19_2) {
  launch("1.19.2");
}
TEST_F(MinecraftLaunch, v1_19) {
    launch("1.19");
}
TEST_F(MinecraftLaunch, v1_18_2) {
    launch("1.18.2");
}
TEST_F(MinecraftLaunch, v1_12_2) {
    launch("1.12.2");
}
TEST_F(MinecraftLaunch, v1_8_9) {
    launch("1.8.9");
}
TEST_F(MinecraftLaunch, v1_7_10) {
    launch("1.7.10");
}
TEST_F(MinecraftLaunch, v1_6_4) {
    launch("1.7.10");
}
TEST_F(MinecraftLaunch, v1_5_2) {
    launch("1.5.2");
}
TEST_F(MinecraftLaunch, v1_4_7) {
    launch("1.4.7");
}
TEST_F(MinecraftLaunch, v1_3_2) {
    launch("1.3.2");
}
TEST_F(MinecraftLaunch, v1_2_5) {
    launch("1.2.5");
}
TEST_F(MinecraftLaunch, v1_0) {
    launch("1.0");
}
TEST_F(MinecraftLaunch, vb1_8_1) {
    launch("b1.8.1");
}
TEST_F(MinecraftLaunch, vb1_0) {
    launch("b1.0");
}
TEST_F(MinecraftLaunch, va1_2_6) {
    launch("a1.2.6");
}
TEST_F(MinecraftLaunch, va1_1_0) {
    launch("a1.1.0");
}
TEST_F(MinecraftLaunch, vrd_161348) {
    launch("rd-161348");
}