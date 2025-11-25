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
#include "AUI/View/ASpacerFixed.h"

#include <AUI/Util/UIBuildingHelpers.h>
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
#include <AUI/View/ASpinnerV2.h>
#include <AUI/View/AProgressBar.h>

using namespace declarative;

static constexpr auto LOG_TAG = "Import";

const PropertyList importButtonWrap = {
    Padding { 8_dp },
};

ImportVersionWindow::ImportVersionWindow(State& state)
  : AWindow("Import version", 500_dp, 400_dp, &MainWindow::inst(), WindowStyle::MODAL), mState(state) {
    _<AView> minecraftRepoListWrap = Horizontal::Expanding {
        mMinecraftRepoList =
            _new<AListView>() AUI_LET {
                it->setCustomStyle({
                  Expanding {},
                  MinSize { 70_dp },
                });
                connect(it->itemDoubleClicked, me::doImportFromMinecraftRepo);
            },
        Vertical {
            Vertical {
              _new<ALabel>("Search:"),
              _new<ATextField>() AUI_LET { it && mSearchTextField; it->focus(); },
            },
            Vertical {
          _new<ALabel>("Filter:"),
          RadioButton {
              .checked = AUI_REACT(mVersionTypeValue == VersionType::RELEASE),
              .onClick = [this] { mVersionTypeValue = VersionType::RELEASE; },
              .content = Label { "Releases" },
          },

         RadioButton {
             .checked = AUI_REACT(mVersionTypeValue == VersionType::SNAPSHOT),
             .onClick = [this] { mVersionTypeValue = VersionType::SNAPSHOT; },
             .content = Label { "Snapshots" },
         },

         RadioButton {
             .checked = AUI_REACT(mVersionTypeValue == VersionType::OLD_BETA),
             .onClick = [this] { mVersionTypeValue = VersionType::OLD_BETA; },
             .content = Label { "Betas" },
         },

         RadioButton {
             .checked = AUI_REACT(mVersionTypeValue == VersionType::OLD_ALPHA),
             .onClick = [this] { mVersionTypeValue = VersionType::OLD_ALPHA; },
             .content = Label { "Alphas" },
         },
        },
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } };

    auto importFromFile = Centered {
        Vertical {
          AText::fromString(
              "You can import a modpack packed into the zip file sent you by your friend or "
              "downloaded from the internet."),
          Centered { Button {
            .content =
                Horizontal {
                  Icon { ":svg/archive.svg" },
                  SpacerFixed { 2_dp },
                  Label { "Choose file" },
                },
            .onClick = [this] { showChooseFileDialog(); },
          } } AUI_OVERRIDE_STYLE { importButtonWrap },
        } AUI_OVERRIDE_STYLE { Expanding { true, false } }
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
              Button {
                .content =
                    Horizontal {
                      Icon { ":svg/download.svg" },
                      SpacerFixed { 2_dp },
                      Label { "Download" },
                    },
                .onClick = [this] { doImportFromMinecraftRepo(); },
              },
            } AUI_OVERRIDE_STYLE { importButtonWrap } },
        } AUI_OVERRIDE_STYLE { Expanding {} },
        Centered {
          _new<AVDividerView>() AUI_OVERRIDE_STYLE { Expanding { 0 }, MinSize { 0, 20_dp } },
        },
        GroupBox {
          Label { "Zip archive" },
          importFromFile,
        } AUI_OVERRIDE_STYLE { Expanding {} },
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
      Horizontal { SpacerExpanding {}, _new<AButton>("Cancel").connect(&AView::clicked, me::close) },
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } });

    minecraftRepoListWrap->setDisabled();

    mAsync << AUI_THREADPOOL {
        _<AListModel<Version>> versionModel = _new<AListModel<Version>>(Version::fetchAll());

        AUI_UI_THREAD {
            minecraftRepoListWrap->setEnabled();
            auto filterModel = AModels::filter(versionModel, [&](const Version& v) {
                if (mVersionTypeValue != v.type)
                    return false;

                auto filterString = *mSearchTextField;
                if (!filterString.empty()) {
                    if (!v.id.contains(filterString)) {
                        return false;
                    }
                }

                return true;
            });
            connect(mSearchTextField.changed, AUI_SLOT(filterModel)::invalidate);
            connect(mVersionTypeValue.changed, AUI_SLOT(filterModel)::invalidate);
            mMinecraftRepoList->setModel(AModels::adapt<AString>(mVersionModel = filterModel, [](const Version& v) {
                return v.id;
            }));
        };
    };
}

void ImportVersionWindow::doImportFromMinecraftRepo() {
    setContents(Centered {
      Horizontal {
        _new<ASpinnerV2>(),
        Label { "Importing..." },

        Centered {
          Button { .content = Label { "Cancel" }, .onClick = [this] { close(); } },
        },
      },
    });

    for (const auto& row : mMinecraftRepoList->getSelectionModel()) {
        Version version = mVersionModel->listItemAt(row.getIndex().getRow());

        mAsync << AUI_THREADPOOL {
            try {
                auto p = _new<GameProfile>(version.import());
                AUI_UI_THREAD {
                    mState.profile.list.writeScope() << p;
                    mState.profile.selected = p;
                    LegacyLauncherJsonSource::save(mState);
                };
            } catch (const AJsonException& e) {
                ALogger::err(LOG_TAG) << e;
                AMessageBox::show(
                    this, "Could not import version", "We unable to parse version manifest",
                    AMessageBox::Icon::CRITICAL);
            } catch (const AException& e) {
                ALogger::err(LOG_TAG) << e;
                AMessageBox::show(this, "Could not import version", e.getMessage(), AMessageBox::Icon::CRITICAL);
            }
            AUI_UI_THREAD { close(); };
        };
    }
}

void ImportVersionWindow::showChooseFileDialog() {
    mAsync
        << ADesktop::browseForFile(this, {}, { ADesktop::FileExtension { "Zip archive", "zip" } })
               .onSuccess([this](const APath& p) {
                   if (p.empty()) {
                       return;
                   }
                   mAsync << AUI_THREADPOOL {
                       _<AProgressBar> progressBar;

                       AUI_UI_THREAD_X [&] {
                           setContents(Centered {
                             Vertical {
                               Horizontal {
                                 _new<ASpinnerV2>(),
                                 Label { "Importing " + p.filename() },
                               },
                               progressBar = _new<AProgressBar>(),
                               Centered {
                                 Button { .content =  Label { "Cancel" }, .onClick = [this] { close(); } },
                               },
                             },
                           });
                       };
                       ALogger::info(LOG_TAG) << "Importing " << p;
                       auto extractFolder = *Settings::inst().gameDir;
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
                               if (unzGetCurrentFileInfo(
                                       unzip, &fileInfo, fileNameBuf, sizeof(fileNameBuf), nullptr, 0, nullptr, 0) !=
                                   UNZ_OK) {
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
                                           throw AException("launcher.error.unpack"_i18n.format(fileName));
                                       }

                                       APath dstFile = extractFolder / fileName;
                                       dstFile.parent().makeDirs();

                                       _<AFileOutputStream> fos;
                                       try {
                                           fos = _new<AFileOutputStream>(dstFile);
                                       } catch (...) {
                                           unzCloseCurrentFile(unzip);
                                           throw AException("launcher.error.unable_to_write"_i18n.format(fileName));
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
                               AUI_UI_THREAD_X [v = float(entryIndex) / float(info.number_entry), progressBar] {
                                   AUI_NULLSAFE(progressBar)->setValue(v);
                               };
                           }

                           LegacyLauncherJsonSource::reload(mState);
                           AUI_UI_THREAD {
                               close();
                               mState.profile.notify();
                           };
                       } catch (const AException& e) {
                           ALogger::err(LOG_TAG) << "Unable to import zip archive: " << e;
                       }

                       AUI_UI_THREAD {
                           close();
                           LegacyLauncherJsonSource::reload(mState);
                       };
                   };
               });
}