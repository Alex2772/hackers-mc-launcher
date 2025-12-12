#include <AUI/UITest.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/View/AButton.h>
#include "TestUtil.h"
#include "model/Settings.h"
#include "window/MainWindow.h"

/**
 * Tests auto refresh feature.
 *
 * When other launcher or Minecraft Forge installer of Optifine installer changes launcher_profiles.json, HMCL should
 * respond to these changes.
 *
 * The launcher responds on mouse move so we use mouseMove action in order to trigger reload.
 */


using namespace std::chrono_literals;

class AutoRefreshTest: public testing::UITest {
protected:
    void SetUp() override {
        UITest::SetUp();
        TestUtil::prepareApp();
    }
};

TEST_F(AutoRefreshTest, DoesNotAutoreloadOnInvalidProfiles) {

    // write a config with invalid profiles
    {
        AFileOutputStream fos(*Settings::inst().gameDir / "launcher_profiles.json");
        const char blob[] = R"(
{
  "authenticationDatabase" : {
    "cec5c3c3a1ce8995d2a9b2026b7bb334" : {
      "properties" : [],
      "username" : "rtrtdtr"
    }
  },
  "clientToken" : "",
  "launcherVersion" : {
    "format" : 21,
    "name" : "",
    "profilesFormat" : 2
  },
  "profiles" : {
    "2950b62619ec093625f8bc0399d2ed8f" : {
      "icon" : "Furnace",
      "javaArgs" : "-Xmx16G -XX:+UnlockExperimentalVMOptions -XX:+UseG1GC -XX:G1NewSizePercent=20 -XX:G1ReservePercent=20 -XX:MaxGCPauseMillis=50 -XX:G1HeapRegionSize=32M",
      "lastUsed" : "2021-12-12T04:19:09.347Z",
      "lastVersionId" : "1.16.5-forge-36.2.19",
      "name" : "1.16.5-forge-36.2.19",
      "type" : ""
    },
    "4bfe61fd9e7ec461ff91963632ff470b" : {
      "lastUsed" : "2021-12-06T00:54:41.090Z",
      "lastVersionId" : "1.17.1",
      "name" : "1.17.1",
      "type" : ""
    },
    "509c9f762743853eef991d1284101c0d" : {
      "lastUsed" : "2021-12-06T00:54:41.090Z",
      "lastVersionId" : "1.16.5",
      "name" : "1.16.5",
      "type" : ""
    },
    "5d2f3e3f068f2a9a369155392e3f7e64" : {
      "lastUsed" : "2021-12-06T00:54:41.090Z",
      "lastVersionId" : "1.15.2",
      "name" : "1.15.2",
      "type" : ""
    },
    "89e82bd9be7651704abda6707ba51068" : {
      "created" : "1970-01-01T00:00:00.000Z",
      "icon" : "Crafting_Table",
      "lastUsed" : "1970-01-01T00:00:00.000Z",
      "lastVersionId" : "latest-snapshot",
      "name" : "",
      "type" : "latest-snapshot"
    },
    "8dd8f63aa2eaef620e5a9f6641811a75" : {
      "created" : "1970-01-02T00:00:00.000Z",
      "icon" : "Grass",
      "lastUsed" : "1970-01-02T00:00:00.000Z",
      "lastVersionId" : "latest-release",
      "name" : "",
      "type" : "latest-release"
    },
    "dd48223123cc0aa36ed7f2190e593ca1" : {
      "lastUsed" : "2021-12-06T00:55:08.134Z",
      "lastVersionId" : "1.16.2",
      "name" : "1.16.2",
      "type" : ""
    }
  },
  "selectedUser" : {
    "account" : "cec5c3c3a1ce8995d2a9b2026b7bb334",
    "profile" : "4bfe61fd9e7ec461ff91963632ff470b"
  },
  "settings" : {
    "crashAssistance" : true,
    "enableAdvanced" : false,
    "enableAnalytics" : true,
    "enableHistorical" : false,
    "enableReleases" : true,
    "enableSnapshots" : false,
    "keepLauncherOpen" : false,
    "profileSorting" : "ByLastPlayed",
    "showGameLog" : false,
    "showMenu" : false,
    "soundOn" : false
  }
}
)";
        fos.write(blob, sizeof(blob));
    }

    TestUtil::prepareMainWindow();
    AObject::connect(MainWindow::inst().reloadProfiles, &MainWindow::inst(), [] {
        FAIL() << "launcher reloaded profiles";
    });

    AThread::sleep(6s); // wait 6 sec
    By::type<AButton>().perform(mouseMove()); // hover mouse to trigger reload
}

