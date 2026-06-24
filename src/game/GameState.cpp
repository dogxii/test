#include "game/GameState.h"

#include <algorithm>

void GameState::setup(int totalTargets) {
    total = std::max(0, totalTargets);
    defeated = 0;
}

bool GameState::update(int defeatedTargets) {
    const int nextDefeated = std::clamp(defeatedTargets, 0, total);
    const bool changed = nextDefeated != defeated;
    defeated = nextDefeated;
    return changed;
}

int GameState::defeatedTargets() const {
    return defeated;
}

int GameState::totalTargets() const {
    return total;
}

bool GameState::cleared() const {
    return total > 0 && defeated >= total;
}

std::string GameState::title() const {
    const std::string progressText = std::to_string(defeated) + "/" + std::to_string(total);

    if (cleared()) {
        return "Robot 3D Roaming World - area clear - targets " + progressText;
    }

    return "Robot 3D Roaming World - targets " + progressText;
}
