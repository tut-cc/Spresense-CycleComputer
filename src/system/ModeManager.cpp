#include "ModeManager.h"

void ModeManager::nextMode() {
    currentMode = static_cast<Mode>((static_cast<int>(currentMode) + 1) % static_cast<int>(Mode::TOTAL_MODES));
}

Mode ModeManager::getMode() {
    return currentMode;
}
