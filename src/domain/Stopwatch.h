#pragma once

class Stopwatch {
public:
  void update(bool isMoving, unsigned long dt) {
    if (isMoving) { duration.movingTimeMs += dt; }
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

private:
  struct Duration {
    unsigned long movingTimeMs = 0;
    unsigned long totalTimeMs  = 0;
  };

  Duration duration;
};
