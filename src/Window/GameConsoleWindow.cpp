#include "GameConsoleWindow.h"
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/iterators.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AText.h>
#include <AUI/View/ASpinner.h>

void GameConsoleWindow::handleGameExit(AWindow* parent, _<GameProcess> game) {
    if (game->process->waitForExitCode() != 0 && !game->consoleWindow) {
        // game has crashed; should display game console
        _new<GameConsoleWindow>(parent, std::move(game))->show();
    }
}


GameConsoleWindow::GameConsoleWindow(AWindow* parent, _<GameProcess> game): AWindow("Game console", 600_dp, 400_dp, parent) {
    setContents(Stacked {
        mScroll = AScrollArea::Builder().withContents(
            mConsoleDisplayPort = Vertical {
                _new<ALabel>("Hacker's MC Launcher")
            } << ".console_wrap").withExpanding().build(),
        mSpinnerOverlay = Stacked::Expanding {
            _new<AView>() << ".spinner_overlay",
            _new<ASpinner>(),
        }
    });
    setCustomAss({
        ass::Padding { 0_dp },
    });

    game->consoleWindow = this;

    // parse existing stdout
    mTask = async {
        auto lines = AString::fromUtf8(game->stdoutBuffer).split('\n');

        for (auto& line : lines) {
            line = line.replacedAll("\t", "    ");
        }

        ui_threadX [&, lines = std::move(lines)] {
            for (auto& line : lines) {
                mConsoleDisplayPort->addView(AText::fromString(line));
            }
            mConsoleDisplayPort->updateLayout();
            mSpinnerOverlay->setVisibility(Visibility::GONE);
        };

        /*
        for (auto& line : lines) {
            line.removeAll('\r');
        }

        // find for stacktrace
        bool stacktraceTailFound = false;
        std::optional<decltype(lines)::iterator> stacktraceHead;
        for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
            bool isStacktraceEntry = it->trimLeft().startsWith("at ");
            if (!stacktraceTailFound) {
                if (isStacktraceEntry) {
                    stacktraceTailFound = true;
                }
            } else if (!isStacktraceEntry) {
                // move one more line if possible to capture error message
                if (it != lines.rend()) {
                    ++it;
                }
                stacktraceHead = aui::reverse_iterator_direction(it);
            }
        }
        if (stacktraceHead) {
            lines.erase(lines.begin(), *stacktraceHead);
        } else {
            lines.clear();
        }*/
    };
}