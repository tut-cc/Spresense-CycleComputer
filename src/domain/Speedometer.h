#pragma once

class Speedometer {
private:
  float curKmh = 0.0f;
  float maxKmh = 0.0f;
  float avgKmh = 0.0f;

  static constexpr float MS_PER_HOUR = 60.0f * 60.0f * 1000.0f;

public:
  void update(float curKmh, unsigned long movingTimeMs, float totalKm) {
    this->curKmh = curKmh;
    if (maxKmh < curKmh) maxKmh = curKmh;
    if (0 < movingTimeMs) avgKmh = totalKm / (movingTimeMs / MS_PER_HOUR);
  }

  void reset() {
    curKmh = 0.0f;
    maxKmh = 0.0f;
    avgKmh = 0.0f;
  }

  float getCur() const {
    return curKmh;
  }

  float getMax() const {
    return maxKmh;
  }

  float getAvg() const {
    return avgKmh;
  }
};
