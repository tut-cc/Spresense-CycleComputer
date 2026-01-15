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
  void update(float curKmh, unsigned long movingTimeMs, float totalKm) {
    speed.curKmh = curKmh;
    if (speed.maxKmh < speed.curKmh) speed.maxKmh = speed.curKmh;
    if (0 < movingTimeMs) speed.avgKmh = totalKm / (movingTimeMs / (60.0f * 60.0f * 1000.0f));
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
