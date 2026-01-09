#pragma once

enum class Mode {
    SPEED,
    TIME,
    MAX_SPEED,
    DISTANCE,
    MOVING_TIME,
    ELAPSED_TIME,
    AVG_SPEED,
    TOTAL_MODES
};

class ModeManager {
   private:
    Mode currentMode = Mode::SPEED;

   public:
    void nextMode();
    Mode getMode();
};
