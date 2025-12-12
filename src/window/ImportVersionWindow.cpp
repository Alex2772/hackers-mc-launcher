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
#include <model/GameProfile.h>
#include "ImportVersionWindow.h"
#include "source/LegacyLauncherJsonSource.h"
#include "MainWindow.h"
#include "AUI/Model/AListModel.h"
#include "AUI/Thread/AAsyncHolder.h"
#include "AUI/Util/Archive.h"
#include "AUI/View/AForEachUI.h"
#include "AUI/View/ARadioButton.h"
#include "AUI/View/ASpacerFixed.h"
#include "model/Version.h"

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Util/ARandom.h>
#include <model/Settings.h>
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
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/copy_if.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/filter.hpp>

using namespace declarative;

static constexpr auto LOG_TAG = "Import";

namespace {
const PropertyList importButtonWrap = {
    Padding { 8_dp },
};

struct ImportZipState {
    explicit ImportZipState(APath zipFile) : zipFile(std::move(zipFile)), files(listFiles()) {

    }
    APath zipFile;

    struct File {
        AString name;
        AProperty<bool> toExtract = true;
        AProperty<bool> treeExpanded = false;
        _weak<File> parent;
        AVector<_<File>> children;

        bool isExtractRecursiveAny() const {
            return toExtract || ranges::any_of(children, [](const auto& f) { return f->isExtractRecursiveAny(); });
        }

        bool isExtractRecursiveAll() const {
            return toExtract && ranges::all_of(children, [](const auto& f) { return f->isExtractRecursiveAll(); });
        }

        void setExtractRecursive(bool extract) {
            toExtract = extract;
            for (const auto& f : children) {
                f->setExtractRecursive(extract);
            }
        }
    };

    auto localProfilesToExtract() const {
        return localProfiles | ranges::views::filter([this](const auto& p) { return *p->willBeExtracted; });
    }

    struct LocalGameProfile {
        GameProfile profile;
        APropertyPrecomputed<bool> willBeExtracted;
    };

    AVector<_<LocalGameProfile>> localProfiles;

    const AVector<_<File>> files;
    AProperty<bool> moveAwayMods = true;
    AProperty<AString> status;
    AProperty<aui::float_within_0_1> progress = 0.f;

private:
    static void sortFiles(AVector<_<File>>& files) {
        for (auto& f : files) {
            if (f->children.empty()) {
                continue;
            }
            f->name += "/"; // a directory.
            sortFiles(f->children);
        }
        ranges::sort(files, [](const _<File>& a, const _<File>& b) {
            if (a->children.empty() != b->children.empty()) {
                return b->children.empty();
            }
            return a->name < b->name;
        });
    }

    AVector<_<File>> listFiles() {
        AVector<_<File>> result;
        aui::archive::zip::read(AFileInputStream(zipFile), [&](const aui::archive::FileEntry& fileEntry) {
            _<File> lastFile = nullptr;
            {
                APathView path(fileEntry.name);
                auto currentDir = &result;
                for (const auto& dir : path.split('/')) {
                    if (auto found = currentDir->findIf([&](const _<File>& f) { return f->name == dir; })) {
                        lastFile = *found;
                        currentDir = &(*found)->children;
                        continue;
                    }
                    auto entry = aui::ptr::manage_shared(new File {
                        .name = dir,
                        .parent = std::move(lastFile),
                    });
                    currentDir->push_back(entry);
                    currentDir = &entry->children;
                    lastFile = std::move(entry);
                }
            }

            try {
                // detect a profile.
                if (lastFile == nullptr) {
                    return;
                }
                if (!lastFile->name.endsWith(".hackers.json")) {
                    return;
                }
                auto profileName = lastFile->name.substr(0, lastFile->name.length() - 13);
                auto profileDir = lastFile->parent.lock();
                if (!profileDir) {
                    return;
                }

                if (profileDir->name != profileName) {
                    return;
                }
                auto versionsDir = profileDir->parent.lock();
                if (!versionsDir) {
                    return;
                }
                if (versionsDir->name != "versions") {
                    return;
                }
                if (versionsDir->parent.lock() != nullptr) {
                    return;
                }
                // at this point, we certainly deal with a profile file. we can read its contents from FileEntry
                // and present to the user.
                GameProfile dst;
                GameProfile::fromJson(dst, AUuid{}, profileName, AJson::fromStream(fileEntry.open()));
                localProfiles << aui::ptr::manage_shared(new LocalGameProfile {
                    .profile = std::move(dst),
                    .willBeExtracted = AUI_REACT(profileDir->isExtractRecursiveAll())
                });

            } catch (const AException& e) {
                ALogger::err(LOG_TAG) << "Failed to parse zip entry " << fileEntry.name << ": " << e;
            }
        });
        sortFiles(result);
        return result;
    }
};


void doImport(_<ImportZipState> state) {
    ALogger::info(LOG_TAG) << "Importing " << state->zipFile;
    auto extractFolder = *Settings::inst().gameDir;
    try {
        size_t i = 0;
        aui::archive::zip::read(AFileInputStream(state->zipFile), [&](const aui::archive::FileEntry& fileEntry) {
            AThread::interruptionPoint();
            ALogger::info(LOG_TAG) << "Unpacking " << fileEntry.name;

            AUI_UI_THREAD_X [state, name = AString(fileEntry.name)] {
                state->status = "Extracting {}..."_format(name);
            };

            if (!fileEntry.name.empty() && fileEntry.name != "/") {
                if (fileEntry.name.endsWith("/")) {
                    // folder
                    try {
                        extractFolder.file(fileEntry.name).makeDirs();
                    } catch (const AException& e) {
                        ALogger::warn(LOG_TAG) << (e.getMessage());
                    }
                } else {
                    // file
                    APath dstFile = extractFolder / fileEntry.name;
                    dstFile.parent().makeDirs();

                    try {
                        AFileOutputStream fos(dstFile);
                        fos << *fileEntry.open();
                    } catch (...) {
                        throw AException("launcher.error.unable_to_write"_i18n.format(fileEntry.name));
                    }
                }
            }

            AUI_UI_THREAD_X [state, v = float(++i) / float(fileEntry.archiveInfo.numberOfFiles)] {
                state->progress = v;
            };
        });

    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Unable to import zip archive: " << e;
    }
}

class ImportVersionWindow: public AWindow {
public:
    ImportVersionWindow(State& state);

private:
    State& mState;
    _<AListView> mMinecraftRepoList;
    _<IListModel<Version>> mVersionModel;
    AProperty<VersionType> mVersionTypeValue = VersionType::RELEASE;
    AProperty<AString> mSearchTextField;
    AAsyncHolder mAsync;

    void doImportFromMinecraftRepo();
    void showChooseFileDialog();
    void presentImportZip(_<ImportZipState> state);
    void presentImportZipProgress(_<ImportZipState> state);
};
}


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
                   AThread::main()->enqueue([this] {
                       setContents(Centered { _new<ASpinnerV2>() });
                   });
                   mAsync << AUI_THREADPOOL {
                       try {
                           ALogger::info(LOG_TAG) << "Parsing: " << p;
                           auto state = _new<ImportZipState>(p);
                           ALogger::info(LOG_TAG) << "Parsing complete";
                           AUI_UI_THREAD_X [this, state = std::move(state)]() mutable {
                               presentImportZip(std::move(state));
                           };
                       } catch (const AException& e) {
                           auto msg = e.getMessage();
                           AUI_UI_THREAD {
                               AMessageBox::show(this, "Importing failed", msg);
                               close();
                           };
                           ALogger::err(LOG_TAG) << "Parsing " << p << " failed: " << e;
                       }

                  };
               });
}

static _<AView> contentTree(_<const AVector<_<ImportZipState::File>>> state) {
    return AUI_DECLARATIVE_FOR(i, *state, AVerticalLayout) {
        return Horizontal {
            Label { AUI_REACT(i->treeExpanded ? "v" : ">") } AUI_OVERRIDE_STYLE {
                FixedSize { 14_dp },
                ATextAlign::CENTER,
            } AUI_LET {
                AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [i] { i->treeExpanded = !i->treeExpanded; });
                if (i->children.empty()) {
                    it->setVisibility(Visibility::INVISIBLE);
                }
            },
            Vertical {
                CheckBox {
                  .checked = AUI_REACT(i->isExtractRecursiveAny()),
                  .onCheckedChange = [i](bool g) { i->setExtractRecursive(g); },
                  .content = Label { i->name },
                },
                Horizontal {
                    contentTree(AUI_PTR_ALIAS(i, children)),
                } AUI_LET {
                    AObject::connect(i->treeExpanded, AObject::GENERIC_OBSERVER, [it, i] {
                        it->setVisibility(i->treeExpanded ? Visibility::VISIBLE : Visibility::GONE);
                    });
                },
            }
        };
    };
}

_<AView> importedGameProfilesList(_<ImportZipState> state) {
    // replicate visuals of game profile list in main window.
    return AUI_DECLARATIVE_FOR(i, state->localProfilesToExtract(), AWordWrappingLayout) {
        return GameProfilesView::item(AUI_PTR_ALIAS(i, profile));
    };
}

void ImportVersionWindow::presentImportZip(_<ImportZipState> state) {
    setContents(Vertical {
        Vertical::Expanding {
            AText::fromString("Please select which files you want to import:"),
            AScrollArea::Builder()
               .withContents(contentTree(AUI_PTR_ALIAS(state, files)))
               .build()
               AUI_OVERRIDE_STYLE {
                   Expanding(),
                   BackgroundSolid { AColor:: WHITE },
                   Border { 1_px, AColor::BLACK },
                   Padding { 2_px },
                   MinSize { 200_dp },
               },
        },

        Vertical {
            AText::fromString("The following game profiles will be added:"),

            AScrollArea::Builder()
               .withContents(importedGameProfilesList(state))
               .build()
               AUI_OVERRIDE_STYLE {
                   BackgroundSolid { AColor:: WHITE },
                   Border { 1_px, AColor::BLACK },
                   Padding { 2_px },
                   FixedSize { {}, 140_dp },
               },
        } AUI_LET {
            AObject::connect(AUI_REACT(state->localProfilesToExtract().empty()), it, [&it = *it](bool v) {
                it.setVisibility(v ? Visibility::GONE : Visibility::VISIBLE);
            });
        },

        CheckBox {
            .checked = AUI_REACT(state->moveAwayMods),
            .onCheckedChange = [state](bool g) { state->moveAwayMods = g; },
            .content = Label { "Move away my mods/ dir" },
        } AUI_LET {
            const bool show = (Settings::inst().gameDir / "mods").isDirectoryExists();
            it->setVisibility(show ? Visibility::VISIBLE : Visibility::GONE);
        },

        Horizontal {
            SpacerExpanding{},
            Button {
                .content = Label { "Import" },
                .onClick = [this, state] {
                    presentImportZipProgress(state);
                },
                .isDefault = true,
            } AUI_LET {
                auto enable = AUI_REACT(ranges::any_of(state->files, [](const _<ImportZipState::File>& f){ return f->isExtractRecursiveAny(); }));
                AObject::connect(enable, AUI_SLOT(it)::setEnabled);
            },
            Button {
              .content = Label { "Cancel" },
              .onClick = [this] { close(); },
            },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },

    } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } });
}

void ImportVersionWindow::presentImportZipProgress(_<ImportZipState> state) {
    setContents(
      Vertical {
          Centered::Expanding {
            Vertical {
              Horizontal {
                _new<ASpinnerV2>(),
                Label { AUI_REACT(state->status) },
              },
              _new<AProgressBar>() && state->progress,
            } AUI_OVERRIDE_STYLE {
                Expanding { true, false },
                LayoutSpacing { 8_dp},
            },
          },
          Horizontal {
            SpacerExpanding{},
            Button { .content =  Label { "Cancel" }, .onClick = [this] { close(); } },
          },
        } AUI_OVERRIDE_STYLE {
          LayoutSpacing { 8_dp },
      });

    mAsync << AUI_THREADPOOL {
        if (*state->moveAwayMods) {
            auto myModsDir = Settings::inst().gameDir / "mods";
            if (myModsDir.isDirectoryExists()) {
                auto targetModsDir = Settings::inst().gameDir / "mods.old";
                while (targetModsDir.isDirectoryExists()) {
                    targetModsDir += ".old";
                }
                APath::move(myModsDir, targetModsDir);
            }
        }
        doImport(state);

        AUI_UI_THREAD {
            LegacyLauncherJsonSource::reload(mState);
            close();
            mState.profile.notify();
        };
    };
}

void ui::presentImportVersion(State& state) {
    _new<ImportVersionWindow>(state)->show();

}