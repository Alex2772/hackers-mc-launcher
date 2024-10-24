//
// Created by alex2772 on 3/2/21.
//

#include <AUI/View/AButton.h>
#include <AUI/View/AListView.h>
#include <AUI/View/ADropdownList.h>
#include <AUI/View/AText.h>
#include <AUI/Curl/ACurl.h>
#include <AUI/Json/AJson.h>
#include <AUI/Model/AModels.h>
#include <AUI/View/ACheckBox.h>
#include <Model/GameProfile.h>
#include "ImportVersionWindow.h"
#include "Source/LegacyLauncherJsonSource.h"
#include "Util/Zip.h"
#include "MainWindow.h"

#include <AUI/Util/UIBuildingHelpers.h>
#include <Repository/GameProfilesRepository.h>
#include <AUI/Util/ARandom.h>
#include <Model/Settings.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/View/ATextField.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/View/AGroupBox.h>
#include <AUI/View/AVDividerView.h>
#include <AUI/View/ASpinner.h>
#include <AUI/View/AProgressBar.h>

using namespace declarative;

static constexpr auto LOG_TAG = "Import";

const PropertyList importButtonWrap = {
    Padding { 8_dp },
};

ImportVersionWindow::ImportVersionWindow():
    AWindow("Import version", 500_dp, 400_dp, &MainWindow::inst(), WindowStyle::MODAL)
{
    connect(mReleaseTypeGroup.selectionChanged, [&](int d) {
        mVersionTypeValue = (VersionType)d;
        emit invalidateSearch;
    });
    _<AView> minecraftRepoListWrap = Horizontal::Expanding {
        mMinecraftRepoList = _new<AListView>() let {
            it->setCustomStyle({ Expanding{}, MinSize { 70_dp }, });
            connect(it->itemDoubleClicked, me::doImportFromMinecraftRepo);
        },
        Vertical {
            _new<ALabel>("Search:"),
            mSearchTextField = _new<ATextField>().connect(&ATextField::textChanging, [&]{ emit invalidateSearch; }) let { it->focus(); },
            _new<ALabel>("Filter:"),
            mReleaseTypeGroup.addRadioButton(_new<ARadioButton>("Releases"), int(VersionType::RELEASE)),
            mReleaseTypeGroup.addRadioButton(_new<ARadioButton>("Snapshots"), int(VersionType::SNAPSHOT)),
            mReleaseTypeGroup.addRadioButton(_new<ARadioButton>("Betas"), int(VersionType::OLD_BETA)),
            mReleaseTypeGroup.addRadioButton(_new<ARadioButton>("Alphas"), int(VersionType::OLD_ALPHA)),
        },
    };

    mReleaseTypeGroup.setSelectedId(int(VersionType::RELEASE));

    auto importFromFile = Centered {
        Vertical {
            AText::fromString("You can import a modpack packed into the zip file sent you by your friend or "
                              "downloaded from the internet."),
                              Centered {
                Button {
                    Icon { ":svg/archive.svg" },
                    Label {  "Choose file" },
                    }.clicked(me::showChooseFileDialog)
            } with_style { importButtonWrap },
        } with_style { Expanding { true, false } }
    };

    setContents(Vertical {
        _new<ALabel>("Import version") << ".title",
        AText::fromString("Please choose where do you want import version from:"),

        Horizontal::Expanding {
            GroupBox {
                Label { "Official Minecraft repository" },
                Vertical::Expanding {
                    minecraftRepoListWrap,
                    Centered {
                        Button{
                            Icon { ":svg/download.svg" },
                            Label { "Download" },
                        }.clicked(me::doImportFromMinecraftRepo),
                    } with_style { importButtonWrap }
                },
            } with_style { Expanding{} },
            Centered {
               _new<AVDividerView>() with_style { Expanding { 0 }, MinSize { 0, 20_dp } },
            },
            GroupBox {
                Label{ "Zip archive" },
                importFromFile,
            } with_style { Expanding{} },
        },
        Horizontal {
            SpacerExpanding{},
            _new<AButton>("Cancel").connect(&AView::clicked, me::close)
        }
    });

    minecraftRepoListWrap->setDisabled();

    mAsync << async {
        _<AListModel<Version>> versionModel = _new<AListModel<Version>>(Version::fetchAll());

        ui_thread {
            minecraftRepoListWrap->setEnabled();
            auto filterModel = AModels::filter(versionModel, [&](const Version& v) {
                if (v.type != mVersionTypeValue) return false;

                auto filterString = mSearchTextField->text();
                if (!filterString.empty()) {
                    if (!v.id.contains(filterString)) {
                        return false;
                    }
                }

                return true;
            });
            connect(invalidateSearch, slot(filterModel)::invalidate);
            mMinecraftRepoList->setModel(AModels::adapt<AString>(mVersionModel = filterModel, [](const Version& v) { return v.id; }));
        };
    };
}

void ImportVersionWindow::doImportFromMinecraftRepo() {
    setContents(Centered {
            Horizontal {
                    _new<ASpinner>(),
                    Label { "Importing..." },

                    Centered {
                        Button { Label { "Cancel" } }.clicked(me::close),
                    }
            }
    });

    for (const auto& row : mMinecraftRepoList->getSelectionModel()) {
        Version version = mVersionModel->listItemAt(row.getIndex().getRow());

        mAsync << async {
            try {
                GameProfile p = version.import();
                ui_thread {
                    GameProfilesRepository::inst().addGameProfile(p);
                };
                LegacyLauncherJsonSource::save();
            } catch (const AJsonException& e) {
                ALogger::err(LOG_TAG) << e;
                AMessageBox::show(this, "Could not import version", "We unable to parse version manifest", AMessageBox::Icon::CRITICAL);
            } catch (const AException& e) {
                ALogger::err(LOG_TAG) << e;
                AMessageBox::show(this, "Could not import version", e.getMessage(), AMessageBox::Icon::CRITICAL);
            }
            ui_thread {
                close();
            };
        };
    }
}

void ImportVersionWindow::showChooseFileDialog() {
    mAsync << ADesktop::browseForFile(this, {}, { ADesktop::FileExtension{ "Zip archive", "zip" } }).onSuccess([this](const APath& p) {
        if (p.empty()) {
            return;
        }
        mAsync << async {
            _<AProgressBar> progressBar;

            ui_threadX [&] {
                setContents(Centered{
                        Vertical{
                                Horizontal{
                                        _new<ASpinner>(),
                                        Label{"Importing " + p.filename()},
                                },
                                progressBar = _new<AProgressBar>(),
                                Centered{
                                        Button{Label{"Cancel"}}.clicked(me::close),
                                }
                        },
                });
            };
            ALogger::info(LOG_TAG) << "Importing " << p;
            auto extractFolder = Settings::inst().gameDir;
            try {
                unzip::File unzip = _new<AFileInputStream>(p);
                unz_global_info info;
                if (unzGetGlobalInfo(unzip, &info) != UNZ_OK) {
                    throw AException("unzGetGlobalInfo failed");
                }

                for (size_t entryIndex = 0; entryIndex < info.number_entry; ++entryIndex) {
                    AThread::interruptionPoint();
                    char fileNameBuf[0x400];
                    unz_file_info fileInfo;
                    if (unzGetCurrentFileInfo(unzip,
                                              &fileInfo,
                                              fileNameBuf,
                                              sizeof(fileNameBuf),
                                              nullptr,
                                              0,
                                              nullptr,
                                              0) != UNZ_OK) {
                        throw AException("failed getting info for {}"_format(fileNameBuf));
                    }
                    APath fileName = fileNameBuf;
                    ALogger::info(LOG_TAG) << "Unpacking " << fileName;
                    if (!fileName.empty() && fileName != "/") {
                        if (fileName.endsWith('/')) {
                            // folder
                            try {
                                extractFolder.file(fileName).makeDirs();
                            } catch (const AException& e) {
                                ALogger::warn(LOG_TAG) << (e.getMessage());
                            }
                        } else {
                            // file
                            if (unzOpenCurrentFile(unzip) != UNZ_OK) {
                                throw AException(
                                        "launcher.error.unpack"_i18n.format(fileName));
                            }


                            APath dstFile = extractFolder / fileName;
                            dstFile.parent().makeDirs();

                            _<AFileOutputStream> fos;
                            try {
                                fos = _new<AFileOutputStream>(dstFile);
                            } catch (...) {
                                unzCloseCurrentFile(unzip);
                                throw AException(
                                        "launcher.error.unable_to_write"_i18n.format(
                                                fileName));
                            }

                            uint64_t total = 0;
                            char buf[0x800];
                            for (int read; (read = unzReadCurrentFile(unzip, buf, sizeof(buf))) > 0;) {
                                fos->write(buf, read);
                                total += read;
                            }

                            unzCloseCurrentFile(unzip);
                        }
                    }

                    if ((entryIndex + 1) < info.number_entry) {
                        if (unzGoToNextFile(unzip) != UNZ_OK)
                            break;
                    }
                    ui_threadX [v = float(entryIndex) / float(info.number_entry), progressBar] {
                        AUI_NULLSAFE(progressBar)->setValue(v);
                    };
                }

                ui_thread {
                    close();
                    LegacyLauncherJsonSource::reload();
                };
            } catch (const AException& e) {
                ALogger::err(LOG_TAG) << "Unable to import zip archive: " << e;
            }

            ui_thread {
                close();
                LegacyLauncherJsonSource::reload();
            };
        };
    });
}
