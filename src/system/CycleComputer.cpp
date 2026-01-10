#include "CycleComputer.h"

#include "../Config.h"
#include "../drivers/OLEDDriver.h"
#include "Logger.h"

inline void formatFloat(float val, int width, int prec, char* buf, size_t size) {
    char fmt[6];
    snprintf(fmt, sizeof(fmt), "%%%d.%df", width, prec);
    snprintf(buf, size, fmt, val);
}

CycleComputer::CycleComputer(OLEDDriver* display) : display(display) {}

void CycleComputer::begin() {
    display->begin();
    inputManager.begin();
    gps.begin();
    tripComputer.begin();
    powerManager.begin();
}

void CycleComputer::update() {
    handleInput();
    gps.update();
    tripComputer.update(gps.getSpeedKmh(), millis());
    powerManager.update();
    updateDisplay();
}

void CycleComputer::handleInput() {
    InputEvent event = inputManager.update();

    switch (event) {
        case InputEvent::BTN_A:
            modeManager.nextMode();
            forceUpdate = true;
            break;
        case InputEvent::BTN_BOTH:
            tripComputer.reset();
            forceUpdate = true;
            break;
        case InputEvent::BTN_B:
            break;
        default:
            break;
    }
}

void CycleComputer::updateDisplay() {
    unsigned long currentMillis = millis();

    if (!forceUpdate && (currentMillis - lastDisplayUpdate < Config::DISPLAY_UPDATE_INTERVAL_MS))
        return;

    lastDisplayUpdate = currentMillis;
    forceUpdate = false;

    char buf[20];
    DisplayDataType type;
    getDisplayData(modeManager.getMode(), type, buf, sizeof(buf));

    logDebugInfo(modeManager.getMode(), buf);

    display->show(type, buf);
}

void CycleComputer::logDebugInfo(Mode currentMode, const char* value) {
#ifdef DEBUG_MODE
    static int lastDebugMode = -1;

    int modeInt = (int)currentMode;
    if (lastDebugMode != modeInt) {
        LOG_DEBUG("[CycleComputer] Mode changed: %d Value: %s", modeInt, value);
        lastDebugMode = modeInt;
    }
#endif
}

void CycleComputer::getDisplayData(Mode mode, DisplayDataType& type, char* buf, size_t size) {
    switch (mode) {
        case Mode::SPEED:
            type = DisplayDataType::SPEED;
            formatFloat(gps.getSpeedKmh(), 4, 1, buf, size);
            break;
        case Mode::TIME:
            type = DisplayDataType::TIME;
            gps.getTimeJST(buf, size);
            break;
        case Mode::MAX_SPEED:
            type = DisplayDataType::MAX_SPEED;
            formatFloat(tripComputer.getMaxSpeedKmh(), 4, 1, buf, size);
            break;
        case Mode::DISTANCE:
            type = DisplayDataType::DISTANCE;
            formatFloat(tripComputer.getDistanceKm(), 5, 2, buf, size);
            break;
        case Mode::MOVING_TIME:
            type = DisplayDataType::MOVING_TIME;
            tripComputer.getMovingTimeStr(buf, size);
            break;
        case Mode::ELAPSED_TIME:
            type = DisplayDataType::ELAPSED_TIME;
            tripComputer.getElapsedTimeStr(buf, size);
            break;
        case Mode::AVG_SPEED:
            type = DisplayDataType::AVG_SPEED;
            formatFloat(tripComputer.getAvgSpeedKmh(), 4, 1, buf, size);
            break;
        default:
            type = DisplayDataType::INVALID;
            buf[0] = '\0';
            break;
    }
}
