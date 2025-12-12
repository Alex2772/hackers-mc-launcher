//
// Created by alex2772 on 11/27/25.
//

#include "MyUpdater.h"

#include "AUI/Updater/AppropriatePortablePackagePredicate.h"
#include "AUI/Updater/GitHub.h"
#include "AUI/Updater/Semver.h"
#include "AUI/Util/kAUI.h"
#include "model/Settings.h"

#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/view/transform.hpp>

static constexpr auto LOG_TAG = "MyUpdater";

AFuture<void> MyUpdater::checkForUpdatesImpl() {
    return AUI_THREADPOOL {
         ALogger::info(LOG_TAG) << "Checking for updates...";
         try {
                auto githubLatestRelease = aui::updater::github::latestRelease("alex2772", "hackers-mc-launcher");
                ALogger::info(LOG_TAG) << "Found latest release: " << githubLatestRelease.tag_name;
                auto ourVersion = aui::updater::Semver::fromString(AUI_PP_STRINGIZE(AUI_CMAKE_PROJECT_VERSION));
                auto theirVersion = aui::updater::Semver::fromString(githubLatestRelease.tag_name);

                if (theirVersion <= ourVersion) {
                    ALogger::info(LOG_TAG) << "No updates found";
                    return;
                }
                aui::updater::AppropriatePortablePackagePredicate predicate {};
                auto it = ranges::find_if(
                    githubLatestRelease.assets, predicate, &aui::updater::github::LatestReleaseResponse::Asset::name);
                if (it == ranges::end(githubLatestRelease.assets)) {
                    ALogger::warn(LOG_TAG)
                        << "Newer version was found but a package appropriate for your platform is not available. "
                           "Expected: "
                        << predicate.getQualifierDebug() << ", got: "
                        << (githubLatestRelease.assets |
                            ranges::view::transform(&aui::updater::github::LatestReleaseResponse::Asset::name));
                    return;
                }

                ALogger::info(LOG_TAG) << "New version found: " << githubLatestRelease.tag_name;
                ALogger::info(LOG_TAG) << "To download: " << (mDownloadUrl = it->browser_download_url);

                downloadUpdate();
            } catch (const AException& e) {
                ALogger::err(LOG_TAG) << "Can't check for updates: " << e;
            }
    };
}

MyUpdater::MyUpdater() {
}

AFuture<void> MyUpdater::downloadUpdateImpl(const APath& unpackedUpdateDir) {
    return AUI_THREADPOOL {
        try {
            AUI_ASSERTX(!mDownloadUrl.empty(), "make a successful call to checkForUpdates first");
            ALogger::info(LOG_TAG) << "Downloading update from " << mDownloadUrl << " to " << unpackedUpdateDir;
            downloadAndUnpack(mDownloadUrl, unpackedUpdateDir);
            ALogger::info(LOG_TAG) << "Update downloaded successfuly";
            reportReadyToApplyAndRestart(makeDefaultInstallationCmdline());
        } catch (const AException& e) {
            ALogger::err(LOG_TAG) << "Can't download update" << e;
        }
    };
}

MyUpdater& MyUpdater::inst() {
    static auto updater = aui::ptr::manage_shared(new MyUpdater);
    AUI_DO_ONCE {
        connect(Settings::inst().autoUpdate, updater, [&updater = *updater](bool upd) {
            if (upd) {
                AUI_DO_ONCE {
                    updater.checkForUpdates();
                }
            }
        });
    }
    return *updater;
}
