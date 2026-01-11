#pragma once

namespace application {

class Speedometer {
private:
  float maxSpeedKmh;

public:
  Speedometer() : maxSpeedKmh(0.0f) {}

  void update(float currentSpeedKmh) {
    if (currentSpeedKmh > maxSpeedKmh) { maxSpeedKmh = currentSpeedKmh; }
  }

  void reset() {
    maxSpeedKmh = 0.0f;
  }

  float getMax() const {
    return maxSpeedKmh;
  }
};

} // namespace application
