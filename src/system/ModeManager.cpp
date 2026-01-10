#include "ModeManager.h"

namespace application {

ModeManager::ModeManager() : currentMode(Mode::SPEED) {}

void ModeManager::nextMode() {
  currentMode = static_cast<Mode>((static_cast<int>(currentMode) + 1) % 7);
}

void ModeManager::prevMode() {
  currentMode = static_cast<Mode>((static_cast<int>(currentMode) + 6) % 7);
}

Mode ModeManager::getMode() const {
  return currentMode;
}

} // namespace application
