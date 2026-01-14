#pragma once

class Speedometer {
private:
  struct Speed {
    float curKmh = 0.0f;
    float maxKmh = 0.0f;
    float avgKmh = 0.0f;
  };

  Speed speed;

public:
  void update(float curKmh, unsigned long movingTimeMs, float distanceKm) {
    speed.curKmh = curKmh;
    if (speed.maxKmh < speed.curKmh) speed.maxKmh = speed.curKmh;
    if (0 < movingTimeMs) speed.avgKmh = distanceKm * 3600000.0f / movingTimeMs;
  }

  void reset() {
    speed = {};
  }

  float getCur() const {
    return speed.curKmh;
  }

  float getMax() const {
    return speed.maxKmh;
  }

  float getAvg() const {
    return speed.avgKmh;
  }
};
