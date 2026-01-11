#pragma once

namespace application {

enum class Mode {
  SPEED,
  MAX_SPEED,
  AVG_SPEED,
  TIME,
  MOVING_TIME,
  ELAPSED_TIME,
  DISTANCE,
};

class ModeManager {
private:
  Mode currentMode;

public:
  ModeManager() : currentMode(Mode::SPEED) {}

  void nextMode() {
    currentMode = static_cast<Mode>((static_cast<int>(currentMode) + 1) % 7);
  }

  void prevMode() {
    currentMode = static_cast<Mode>((static_cast<int>(currentMode) + 6) % 7);
  }

  Mode getMode() const {
    return currentMode;
  }
};

} // namespace application
