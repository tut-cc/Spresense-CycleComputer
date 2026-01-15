#pragma once

class Stopwatch {
private:
  struct Duration {
    unsigned long movingTimeMs = 0;
    unsigned long totalTimeMs  = 0;
  };

  Duration duration;
  bool     isPaused = false;

public:
  void update(bool isMoving, unsigned long dt) {
    if (isMoving) duration.movingTimeMs += dt;
    if (!isPaused) duration.totalTimeMs += dt;
  }

  void reset() {
    duration.totalTimeMs = 0;
  }

  void pause() {
    if (isPaused) isPaused = false;
    else isPaused = true;
  }

  unsigned long getMovingTimeMs() const {
    return duration.movingTimeMs;
  }

  unsigned long getElapsedTimeMs() const {
    return duration.totalTimeMs;
  }
};
