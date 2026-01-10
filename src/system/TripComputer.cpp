#include "TripComputer.h"

namespace application {

TripComputer::TripComputer() : distanceKm(0.0f), maxSpeedKmh(0.0f), lastUpdateTime(0), movingTimeMs(0), startTimeMs(0) {}

void TripComputer::begin() {
  reset();
}

void TripComputer::update(float currentSpeedKmh, unsigned long currentMillis) {
  if (lastUpdateTime == 0) {
    lastUpdateTime = currentMillis;
    if (startTimeMs == 0)
      startTimeMs = currentMillis;
    return;
  }

  unsigned long dt = currentMillis - lastUpdateTime;
  lastUpdateTime = currentMillis;

  if (currentSpeedKmh > 3.0f) { // Moving threshold
    float distanceInc = (currentSpeedKmh * dt) / 3600000.0f;
    distanceKm += distanceInc;
    movingTimeMs += dt;
  }

  if (currentSpeedKmh > maxSpeedKmh) {
    maxSpeedKmh = currentSpeedKmh;
  }
}

void TripComputer::reset() {
  distanceKm = 0.0f;
  maxSpeedKmh = 0.0f;
  movingTimeMs = 0;
  startTimeMs = millis();
  lastUpdateTime = 0;
}

float TripComputer::getDistanceKm() const {
  return distanceKm;
}
float TripComputer::getMaxSpeedKmh() const {
  return maxSpeedKmh;
}

float TripComputer::getAvgSpeedKmh() const {
  if (movingTimeMs == 0)
    return 0.0f;
  return (distanceKm / (movingTimeMs / 3600000.0f));
}

void TripComputer::msToTimeStr(unsigned long ms, char *buffer, size_t size) const {
  unsigned long totalSeconds = ms / 1000;
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;
  snprintf(buffer, size, "%02d:%02d:%02d", (int)hours, (int)minutes, (int)seconds);
}

void TripComputer::getMovingTimeStr(char *buffer, size_t size) const {
  msToTimeStr(movingTimeMs, buffer, size);
}

void TripComputer::getElapsedTimeStr(char *buffer, size_t size) const {
  if (startTimeMs == 0) {
    msToTimeStr(0, buffer, size);
  } else {
    msToTimeStr(millis() - startTimeMs, buffer, size);
  }
}

} // namespace application
