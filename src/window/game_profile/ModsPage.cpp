//
// Created by alex2772 on 4/13/25.
//

#include "ModsPage.h"

#include "AUI/IO/AFileInputStream.h"
#include "AUI/Thread/AAsyncHolder.h"
#include "AUI/Util/AImageDrawable.h"
#include "AUI/Util/Archive.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/AForEachUI.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/View/ASpacerFixed.h"
#include "AUI/View/ASplitter.h"
#include "model/Modloader.h"
#include "model/Settings.h"
#include "view/Common.h"

#include <future>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/Platform/AMessageBox.h>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/view/cartesian_product.hpp>

using namespace declarative;

static constexpr auto LOG_TAG = "ModsPage";
static constexpr auto GRID_LINE_COLOR = 0x808080_rgb;


namespace {

struct EllipsisLabel {
    contract::In<AString> text;

    _<AView> operator()() {
        return Label { .text = std::move(text) } AUI_OVERRIDE_STYLE { ATextOverflow::ELLIPSIS, Expanding{} };
    }
};

enum class Problem {
    NONE,
    NO_MODLOADER,
    UNSUPPORTED_MODLOADER,
};

enum class ColumnSort {
    ENABLED,
    PROBLEM,
    NAME,
    MODLOADER,
    VERSION,
};

struct Mod {
    AProperty<bool> isSelected = false;
    AProperty<_<IDrawable>> icon = nullptr;
    APath path;
    AProperty<bool> enabled = true;
    AProperty<ABitField<Modloader>> modloaders = Modloader::NONE;
    AProperty<AString> version;
    AProperty<Problem> problem = Problem::NONE;
};

struct ModsState: public AObject {
    APath modsDir;
    AAsyncHolder async;
    AProperty<AVector<_<Mod>>> mods;
    AProperty<ColumnSort> sortColumn = ColumnSort::NAME;
    _<GameProfile> gameProfile;

    ModsState(APath modsDir) : modsDir(std::move(modsDir)) {
        connect(sortColumn, [this] { handleSort(); });
        connect(mods, [this] { handleSort(); });
    }

private:
    void handleSort() {
        switch (*sortColumn) {
            case ColumnSort::ENABLED:
                mods.writeScope()->sort([](const _<Mod>& a, const _<Mod>& b) { return *a->enabled < *b->enabled; });
                break;

            case ColumnSort::PROBLEM:
                mods.writeScope()->sort([](const _<Mod>& a, const _<Mod>& b) { return *a->problem > *b->problem; });
                break;

            case ColumnSort::NAME:
                mods.writeScope()->sort([](const _<Mod>& a, const _<Mod>& b) {
                    return a->path.filename() < b->path.filename();
                });
                break;

            case ColumnSort::MODLOADER:
                mods.writeScope()->sort([](const _<Mod>& a, const _<Mod>& b) { return static_cast<int>(a->modloaders->value()) < static_cast<int>(b->modloaders->value()); });
                break;

            case ColumnSort::VERSION:
                mods.writeScope()->sort([](const _<Mod>& a, const _<Mod>& b) { return *a->version < *b->version; });
                break;
        }
    }
};

void scan(_<ModsState> state) {
    state->async << AUI_THREADPOOL {
        if (!state->modsDir.isDirectoryExists()) {
            return;
        }
        AVector<_<Mod>> mods;
        for (auto i : state->modsDir.listDir()) {
            if (!i.endsWith(".jar")) {
                continue;
            }
            mods << aui::ptr::manage_shared(new Mod { .path = std::move(i) });
        }
        mods.sort([](const _<Mod>& a, const _<Mod>& b) {
            return a->path.filename() < b->path.filename();
        });
        AThread::main()->enqueue([state, mods = std::move(mods)] {
            state->async << AUI_THREADPOOL {
                const auto gameProfileModloader = state->gameProfile->detectModloader();
                for (const auto& mod : mods) {
                    try {
                        aui::archive::zip::read(AFileInputStream(mod->path), [&](const aui::archive::FileEntry& e) {
                            if (e.name.endsWith(".png") && !e.name.contains("/")) {
                                auto image = AImage::fromBuffer(AByteBuffer::fromStream(e.open()));
                                AThread::main()->enqueue([image = std::move(image), mod]() mutable {
                                    mod->icon = _new<AImageDrawable>(std::move(image));
                                });
                            }

                            if (e.name == "fabric.mod.json") {
                                auto json = AJson::fromStream(e.open());

                                AThread::main()->enqueue([
                                    version = json["version"].asStringOpt().valueOr(""),
                                    mod]() mutable {
                                    mod->modloaders << Modloader::FABRIC;
                                    mod->version = std::move(version);
                                });
                            }

                            if (e.name == "pack.mcmeta") {
                                auto json = AJson::fromStream(e.open());
                                if (auto pack = json.containsOpt("pack")) {
                                    if (pack->contains("forge:resource_pack_format") || pack->contains("forge:data_pack_format")) {
                                        AThread::main()->enqueue([mod]() mutable {
                                            mod->modloaders << Modloader::FORGE;
                                        });
                                    }
                                }
                            }
                        });

                        AThread::main()->enqueue([mod, gameProfileModloader] {
                            if (mod->modloaders == static_cast<Modloader>(0)) {
                                mod->problem = Problem::NO_MODLOADER;
                                return;
                            }
                            if (!mod->modloaders->test(gameProfileModloader)) {
                                mod->problem = Problem::UNSUPPORTED_MODLOADER;
                                return;
                            }
                        });
                    } catch (const AException& e) {
                        ALogger::err(LOG_TAG) << "Failed to analyze mod " << mod->path << ": " << e;
                    }
                }
            };
            state->mods = std::move(mods);
        });
    };
}

_<AView> tableHeader(_<ModsState> state, _<AView> wrap, ColumnSort column) {
    wrap->setExpanding();
    return Centered {
        std::move(wrap),
        Vertical {
            SpacerExpanding {},
            Icon { ":uni/svg/down.svg" } AUI_OVERRIDE_STYLE {
                FixedSize { 8_dp, 8_dp },
                BackgroundImage { {}, AColor::BLACK },
            } AUI_LET {
                AObject::connect(AUI_REACT(state->sortColumn == column ? Visibility::VISIBLE : Visibility::INVISIBLE), AUI_SLOT(it)::setVisibility);
            },
        } AUI_OVERRIDE_STYLE { Expanding { 0, 1 } },
    } AUI_OVERRIDE_STYLE {
        BackgroundSolid { AColor::WHITE.transparentize(0.5f) },
        // Backdrop { Backdrop::GaussianBlur { 32_dp } },
        FixedSize { {}, 24_dp },
        Padding { 2_dp, 4_dp },
        BorderBottom { 1_px, GRID_LINE_COLOR },
    } AUI_LET {
        AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [=] { state->sortColumn = column; });
    };
}

_<AView> tableCell(_<AView> wrap) {
    wrap->setExpanding();
    return Centered {
        std::move(wrap),
    } AUI_OVERRIDE_STYLE {
        Padding { 2_dp, 4_dp },
        FixedSize { {}, 20_dp },
    };
}


_<AView> tableColumn(AVector<_<AView>> views) {
    return Vertical { std::move(views) } AUI_OVERRIDE_STYLE {
        BorderLeft { 1_px, GRID_LINE_COLOR },
    };
}

_<AView> columnSelect(_<ModsState> state) {
    return Vertical {
        tableHeader(state, Centered { CheckBox {
            .checked = AUI_REACT(ranges::all_of(*state->mods, [](const _<Mod>& m) { return *m->isSelected; })),
            .onCheckedChange = [state](bool v) {
                for (const auto& mod : *state->mods) {
                    mod->isSelected = v;
                }
            },
        } }, ColumnSort::ENABLED),
        AUI_DECLARATIVE_FOR(i, *state->mods, AVerticalLayout) {
            return tableCell(Centered { CheckBox {
                .checked = AUI_REACT(i->isSelected),
                .onCheckedChange = [i](bool v) { i->isSelected = v; },
            } });
        },
    };
}

AString formatModloaders(const ABitField<Modloader>& modloaders) {
    AStringVector result;
    for (const auto& i : aui::enumerate::ALL_VALUES<Modloader>) {
        if (i == Modloader::NONE) {
            continue;
        }
        if (modloaders.test(i)) {
            result << AEnumerate<Modloader>::toName(i);
        }
    }
    return result.join("/");
}

void explainProblem(const ModsState& state, const Mod& mod) {
    auto msg = [&]() -> AString {
        switch (*mod.problem) {
            case Problem::NONE:
                break;
            case Problem::NO_MODLOADER:
                return "This jar file does not contain any mod loader information.";
            case Problem::UNSUPPORTED_MODLOADER:
                return "This mod does not support the modloader of your game profile.\n"
                       "\n"
                       "Game profile mod loader: {}\n"
                       "Mod loader expected by the mod: {}"_format(
                    state.gameProfile->detectModloader(), formatModloaders(mod.modloaders));
        }
        return "";
    }();
    AMessageBox::show(dynamic_cast<AWindow*>(AWindow::current()), "Mod problem",
        "{}\n\n{}"_format(mod.path.filename(), msg),
        AMessageBox::Icon::WARNING);
}

_<AView> columnProblem(_<ModsState> state) {
    return tableColumn({
        tableHeader(state, Icon { ":svg/error.svg" } AUI_LET {
            AObject::connect(AUI_REACT(ass::PropertyListRecursive{
                BackgroundImage { {}, ranges::all_of(*state->mods, [](const _<Mod>& m) { return *m->problem == Problem::NONE; }) ? GRID_LINE_COLOR.transparentize(0.5f) : 0xff0000_rgb },
            }), AUI_SLOT(it)::setCustomStyle);
        }, ColumnSort::PROBLEM),
        AUI_DECLARATIVE_FOR(i, *state->mods, AVerticalLayout) {
            return tableCell(Centered { Icon { ":svg/error.svg" } AUI_OVERRIDE_STYLE { BackgroundImage { {}, AColor::RED } } AUI_LET {
                AObject::connect(AUI_REACT(i->problem == Problem::NONE ? Visibility::GONE : Visibility::VISIBLE), AUI_SLOT(it)::setVisibility);
              },
            }) AUI_LET {
                AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [=] { explainProblem(*state, *i); });
            };
        },
    });
}

_<AView> columnName(_<ModsState> state) {
    return tableColumn({
        tableHeader(state, Horizontal {
            SpacerFixed { 16_dp },
            SpacerFixed { 2_dp },
            Label { "Name" }
        }, ColumnSort::NAME),
        AUI_DECLARATIVE_FOR(i, *state->mods, AVerticalLayout) {
            return tableCell(Horizontal {
                Icon {} & i->icon AUI_OVERRIDE_STYLE { FixedSize { 16_dp } },
                SpacerFixed { 2_dp },
                EllipsisLabel { i->path.filename() },
            });
        },
    });
}

_<AView> columnEnabled(_<ModsState> state) {
    return tableColumn({
        tableHeader(state, Label { "Enable" }, ColumnSort::ENABLED),
        AUI_DECLARATIVE_FOR(i, *state->mods, AVerticalLayout) {
            return tableCell(Centered { CheckBox {
                .checked = AUI_REACT(i->enabled),
                .onCheckedChange = [i](bool value) { /* i->enabled = value; */ },
            } });
        },
    });
}

_<AView> columnModloaders(_<ModsState> state) {
    return tableColumn({
        tableHeader(state, Label { "Mod loader" }, ColumnSort::MODLOADER),
        AUI_DECLARATIVE_FOR(i, *state->mods, AVerticalLayout) {
            return tableCell(EllipsisLabel { AUI_REACT(formatModloaders(i->modloaders)) });
        },
    });
}

_<AView> columnVersion(_<ModsState> state) {
    return tableColumn({
        tableHeader(state, Label { "Version" }, ColumnSort::VERSION),
        AUI_DECLARATIVE_FOR(i, *state->mods, AVerticalLayout) {
            return tableCell(EllipsisLabel { AUI_REACT(i->version) });
        },
    });
}

_<AView> modTable(_<ModsState> state) {
    // enum class Column {
    //     SELECT,
    //     NAME,
    //     ENABLED,
    // };
    // static constexpr auto COLUMNS = std::array { Column::SELECT, Column::NAME, Column::ENABLED };
    //
    //
    // return aui::detail::makeForEach([=]() -> decltype(auto) { return (ranges::view::cartesian_product(COLUMNS, *state->mods)); },
    //     std::make_unique<AAdvancedGridLayout>(COLUMNS.size(), 1)) - [=](const auto& i) -> _<AView> {
    //         const auto& [column, mod] = i;
    //         switch (column) {
    //             case Column::SELECT: {
    //                 return CheckBox {
    //                     .checked = true
    //                 };
    //             }
    //         }
    //     return EllipsisLabel { i->name };
    // };

    return ASplitter::Horizontal().withItems({
        columnSelect(state),
        columnProblem(state),
        columnName(state) AUI_LET { it->setExpanding(); },
        columnEnabled(state),
        columnModloaders(state),
        columnVersion(state),
    });
}
}

_<AView> game_profile::modsPage(_<GameProfile> profile) {
    auto state = _new<ModsState>(Settings::inst().gameDir / "mods");
    state->gameProfile = std::move(profile);
    scan(state);

    return Centered {
        AScrollArea::Builder().withExpanding().withContents(modTable(state))
    } AUI_OVERRIDE_STYLE {
        Expanding(),
        Padding { 2_px },
        Border { 1_px, AColor::BLACK },
        BackgroundSolid { AColor:: WHITE },
    };
}