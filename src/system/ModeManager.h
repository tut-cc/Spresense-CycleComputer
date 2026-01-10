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
  ModeManager();
  void nextMode();
  void prevMode();
  Mode getMode() const;
};

} // namespace application
