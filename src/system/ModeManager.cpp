#include "ModeManager.h"

void ModeManager::nextMode() {
    currentMode = static_cast<Mode>((currentMode + 1) % TOTAL_MODES);
}

Mode ModeManager::getMode() {
    return currentMode;
}
