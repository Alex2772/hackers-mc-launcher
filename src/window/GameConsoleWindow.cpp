#include "GameConsoleWindow.h"

#include "AUI/View/AForEachUI.h"

#include <AUI/Util/kAUI.h>
#include <AUI/Traits/iterators.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AText.h>
#include <AUI/View/ASpinnerV2.h>

using namespace declarative;

void GameConsoleWindow::handleGameExit(AWindow* parent, _<GameProcess> game) {
    if (game->process->waitForExitCode() != 0 && !game->consoleWindow) {
        // game has crashed; should display game console
        _new<GameConsoleWindow>(parent, std::move(game))->show();
    }
}


GameConsoleWindow::GameConsoleWindow(AWindow* parent, _<GameProcess> game): AWindow("Game console", 600_dp, 400_dp, parent) {
    setContents(Stacked {
        AScrollArea::Builder().withContents(
            Vertical {
                Label { "Hacker's MC Launcher" },
                AUI_DECLARATIVE_FOR(i, *game->stdoutBuffer, AVerticalLayout) {
                    return AText::fromString(i);
                },
            } << ".console_wrap").withExpanding().build(),
    });
    setCustomStyle({
        ass::Padding { 0_dp },
    });

    game->consoleWindow = this;
}