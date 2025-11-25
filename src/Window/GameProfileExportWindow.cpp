//
// Created by alex2772 on 11/24/25.
//

#include "GameProfileExportWindow.h"

#include "AUI/AppInfo.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/Platform/ADesktop.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Thread/AAsyncHolder.h"
#include "AUI/Traits/platform.h"
#include "AUI/Util/Archive.h"
#include "AUI/Util/Declarative/Containers.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/AForEachUI.h"
#include "AUI/View/AHDividerView.h"
#include "AUI/View/AVDividerView.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/View/ASpacerExpanding.h"
#include "AUI/View/ASpacerFixed.h"
#include "AUI/View/ASpinnerV2.h"
#include "AUI/View/AText.h"
#include "Model/Settings.h"

#include <range/v3/algorithm/contains.hpp>
#include <range/v3/range/conversion.hpp>

using namespace declarative;

static constexpr auto LOG_TAG = "GameProfileExportWindow";

static constexpr auto DEFAULTS_TO_INCLUDE = {
    "mods",
    "shaderpacks",
    "resourcepacks",
    "config",
};

namespace {
struct ExportState {
    _<GameProfile> profile;
    struct File {
        AString name;
        AProperty<bool> toExport = false;
    };
    AProperty<AVector<_<File>>> files;
    AProperty<AString> exportStatus;
    AAsyncHolder async;
};

void doExportOneFile(const _<ExportState>& state, aui::archive::zip::Writer& writer, const APath& source) {
    ALogger::info(LOG_TAG) << "Exporting " << source;
    if (source.isDirectoryExists()) {
        for (const auto& p : source.listDir()) {
            doExportOneFile(state, writer, p);
        }
        return;
    }
    AThread::main()->enqueue([state, name = source.filename()] {
        state->exportStatus = name;
    });
    writer.openFileInZip(source.relativelyTo(*Settings::inst().gameDir), [&](IOutputStream& os) {
        os << AFileInputStream(source);
    });
}

void doExport(_<ExportState> state, std::function<void()> onFinish) {
    state->async << ADesktop::browseForDir(AWindow::current(), {}).onSuccess([=](const AString& path) {
        if (path.empty()) {
            AThread::main()->enqueue(std::move(onFinish));
            return;
        }
        state->async << AUI_THREADPOOL mutable {
            auto output = path + "/" + state->profile->name + ".zip";
            {
                aui::archive::zip::Writer writer(std::make_unique<AFileOutputStream>(output));
                for (const auto& file : *state->files) {
                    if (!file->toExport) {
                        continue;
                    }
                    auto source = Settings::inst().gameDir / file->name ;
                    doExportOneFile(state, writer, source);
                }
                doExportOneFile(state, writer, Settings::inst().gameDir / "versions" / state->profile->name);
            }
            AThread::main()->enqueue(std::move(onFinish));
        };
    });
}

_<AView> column0(_<AView> content) {
    return Horizontal { content } AUI_OVERRIDE_STYLE { Expanding{}, FixedSize { {}, 22_dp } };
}

_<AView> column1(_<AView> content) {
    return Centered { content } AUI_OVERRIDE_STYLE { FixedSize { 50_dp, 22_dp } };
}

_<AView> exportTable(_<ExportState> state) {
    return Vertical {
        Horizontal {
            column0(Label { "File" } AUI_OVERRIDE_STYLE {
                Padding { {}, 2_dp },
            }),
            _new<AVDividerView>(),
            column1(Label { "Export?" } AUI_OVERRIDE_STYLE {
                Padding { {}, 2_dp },
            }),
        },
        _new<AHDividerView>(),
        AUI_DECLARATIVE_FOR(file, *state->files, AVerticalLayout) {
            auto cb = file->name == "versions/"
            ? _<AView>(Label { "auto" })
            : _<AView>(CheckBox {
                    .checked = AUI_REACT(file->toExport),
                    .onCheckedChange = [file](bool g) { file->toExport = g; },
                });
            return Horizontal {
                column0(Label { file->name } AUI_OVERRIDE_STYLE {
                    Opacity(!file->name.startsWith(".") ? 1.f : 0.5f),
                    Padding { {}, 2_dp },
                }),
                _new<AVDividerView>(),
                column1(cb),
            };
        }
    };
}

_<AViewContainer> configurePage(_<ExportState> state, declarative::contract::Slot<> nextPage, contract::Slot<> cancel) {
    return Vertical {
        _new<ALabel>("Export") << ".title",
        AText::fromString("Pack game profile to *.zip file to make a backup, or share your modpack."),
        AScrollArea::Builder()
            .withContents(exportTable(state))
            .build()
            AUI_OVERRIDE_STYLE {
                Expanding(),
                BackgroundSolid { AColor:: WHITE },
                Border { 1_px, AColor::BLACK },
                Padding { 2_px },
            },
        Horizontal {
            SpacerExpanding{},
            Button {
                .content = Label { "Export" },
                .onClick = std::move(nextPage),
                .isDefault = true,
            },
            Button {
                .content = Label { "Cancel" },
                .onClick = std::move(cancel),
            },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp } },
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 8_dp} };
}

_<AViewContainer> exportPage(_<ExportState> state) {
    return Vertical {
        _new<ALabel>("Exporting...") << ".title",
        Centered::Expanding {
            Horizontal {
                _new<ASpinnerV2>(),
                Label { AUI_REACT(state->exportStatus), },
            },
        },
    };
}

}

GameProfileExportWindow::GameProfileExportWindow(AWindow* parent, _<GameProfile> profile):
    AWindow("Export game profile", 300_dp, 400_dp, parent)
{
    auto state = _new<ExportState>();
    state->profile = std::move(profile);
    setContents(configurePage(state, [this, state] {
        setContents(exportPage(state));
        doExport(state, [this] { close(); });
    }, [this] { close(); }));

    auto gameDir = *Settings::inst().gameDir;
    state->async << AUI_THREADPOOL {
        auto list = gameDir.listDir();
        list.sort([](const APath& lhs, const APath& rhs) {
            if (lhs.isDirectoryExists() != rhs.isDirectoryExists()) {
                return lhs.isDirectoryExists() > rhs.isDirectoryExists();
            }
            return lhs.filename().lowercase() < rhs.filename().lowercase();
        });
        auto items = list | ranges::view::transform([](const APath& p) {
            auto f =  _new<ExportState::File>();
            f->name = p.filename();
            f->toExport = ranges::contains(DEFAULTS_TO_INCLUDE, f->name);
            if (p.isDirectoryExists()) {
                f->name += "/";
            }
            return f;
        }) | ranges::to_vector;
        AThread::main()->enqueue([state, items = std::move(items)] {
            state->files.writeScope() << std::move(items);
        });
    };
}