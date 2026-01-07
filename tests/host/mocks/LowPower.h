#pragma once

#define CLOCK_MODE_8MHz 0

class LowPowerMock {
public:
    void begin() {}
    void clockMode(int) {}
    int getVoltage() { return 5000; } 
};

extern LowPowerMock LowPower;
