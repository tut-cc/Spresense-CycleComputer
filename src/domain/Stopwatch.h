#pragma once

class Stopwatch {
private:
  struct Duration {
    unsigned long movingTimeMs = 0;
    unsigned long totalTimeMs  = 0;
  };

  Duration duration;

public:
  void update(const bool isMoving, const unsigned long dt) {
    if (isMoving) duration.movingTimeMs += dt;
    duration.totalTimeMs += dt;
  }

  void reset() {
    duration = {};
  }

  unsigned long getMovingTimeMs() const {
    return duration.movingTimeMs;
  }

  unsigned long getElapsedTimeMs() const {
    return duration.totalTimeMs;
  }
};
