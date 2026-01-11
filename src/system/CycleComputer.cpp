#include "CycleComputer.h"

#include "../Config.h"

namespace application {

inline void formatFloat(float val, int width, int prec, char *buf, size_t size) {
  char fmt[6];
  snprintf(fmt, sizeof(fmt), "%%%d.%df", width, prec);
  snprintf(buf, size, fmt, val);
}

CycleComputer::CycleComputer(hal::IDisplay *display, hal::IGnssProvider &gnss, hal::IInputProvider &input) : display(display), gnssProvider(gnss), inputProvider(input) {}

void CycleComputer::begin() {
  display->begin();
  inputProvider.begin();
  gnssProvider.begin();
  tripManager.begin();
  powerManager.begin();
}

void CycleComputer::update() {
  handleInput();
  gnssProvider.update();
  tripManager.update(gnssProvider.getSpeedKmh(), millis());
  powerManager.update();
  updateDisplay();
}

void CycleComputer::handleInput() {
  InputEvent event = inputProvider.update();

  switch (event) {
  case InputEvent::BTN_A:
    modeManager.nextMode();
    forceUpdate = true;
    break;
  case InputEvent::BTN_B:
    modeManager.prevMode();
    forceUpdate = true;
    break;
  case InputEvent::BTN_BOTH:
    tripManager.reset();
    forceUpdate = true;
    break;
  default:
    break;
  }
}

void CycleComputer::updateDisplay() {
  unsigned long currentMillis = millis();

  if (!forceUpdate && (currentMillis - lastDisplayUpdate < Config::DISPLAY_UPDATE_INTERVAL_MS)) return;

  lastDisplayUpdate = currentMillis;
  forceUpdate       = false;

  char            buf[20];
  DisplayDataType type;
  getDisplayData(modeManager.getMode(), type, buf, sizeof(buf));

  display->show(type, buf);
}

void CycleComputer::getDisplayData(Mode mode, DisplayDataType &type, char *buf, size_t size) {
  switch (mode) {
  case Mode::SPEED:
    type = DisplayDataType::SPEED;
    formatFloat(gnssProvider.getSpeedKmh(), 4, 1, buf, size);
    break;
  case Mode::TIME:
    type = DisplayDataType::TIME;
    gnssProvider.getTimeJST(buf, size);
    break;
  case Mode::MAX_SPEED:
    type = DisplayDataType::MAX_SPEED;
    formatFloat(tripManager.getMaxSpeedKmh(), 4, 1, buf, size);
    break;
  case Mode::DISTANCE:
    type = DisplayDataType::DISTANCE;
    formatFloat(tripManager.getDistanceKm(), 5, 2, buf, size);
    break;
  case Mode::MOVING_TIME:
    type = DisplayDataType::MOVING_TIME;
    tripManager.getMovingTimeStr(buf, size);
    break;
  case Mode::ELAPSED_TIME:
    type = DisplayDataType::ELAPSED_TIME;
    tripManager.getElapsedTimeStr(buf, size);
    break;
  case Mode::AVG_SPEED:
    type = DisplayDataType::AVG_SPEED;
    formatFloat(tripManager.getAvgSpeedKmh(), 4, 1, buf, size);
    break;
  default:
    type   = DisplayDataType::INVALID;
    buf[0] = '\0';
    break;
  }
}

} // namespace application
