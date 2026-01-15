#pragma once

class Stopwatch {
private:
  unsigned long movingTimeMs = 0;
  unsigned long totalTimeMs  = 0;
  bool          isPaused     = false;

public:
  void update(bool isMoving, unsigned long dt) {
    if (isMoving) movingTimeMs += dt;
    if (!isPaused) totalTimeMs += dt;
  }

  void resetTotalTime() {
    totalTimeMs = 0;
  }

  void resetMovingTime() {
    movingTimeMs = 0;
  }

  void setMovingTime(unsigned long ms) {
    movingTimeMs = ms;
  }

  void reset() {
    resetTotalTime();
    resetMovingTime();
  }

  void togglePause() {
    isPaused = !isPaused;
  }

  unsigned long getMovingTimeMs() const {
    return movingTimeMs;
  }

  unsigned long getElapsedTimeMs() const {
    return totalTimeMs;
  }
};
