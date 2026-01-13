#pragma once

class Speedometer {
private:
  struct Speed {
    float currentKmh = 0.0f;
    float maxKmh     = 0.0f;
  };

  Speed speed;

public:
  void update(float currentKmh) {
    speed.currentKmh = currentKmh;
    if (speed.currentKmh > speed.maxKmh) speed.maxKmh = speed.currentKmh;
  }

  void reset() {
    speed = {};
  }

  float get() const {
    return speed.currentKmh;
  }

  float getMax() const {
    return speed.maxKmh;
  }
};
