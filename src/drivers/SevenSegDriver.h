#pragma once

#include <Arduino.h>

#include "../interfaces/IDisplay.h"

class SevenSegDriver : public IDisplay {
   public:
    void begin() override;
    void clear() override;
    void show(DisplayDataType type, const char* value) override;
    void update() override;

   private:
    void hc595_shift(byte data);

    // Internal state
    byte digitBuffer[4] = {0, 0, 0, 0}; // Buffer for 4 digits (segment data)
    int currentDigit = 0; // Currently active digit (0-3) for multiplexing
    unsigned long lastMultiplexTime = 0;
    const unsigned long MULTIPLEX_INTERVAL_US = 2000; // 2ms per digit

    // Constant data
    static const int placePin[4];
    static const int colonPin[5];
    static const unsigned char* number;
    static const unsigned char* numdot;
};
