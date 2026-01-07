#pragma once


#include <Arduino.h>

#ifdef ARDUINO_ARCH_SPRESENSE
#include <GNSS.h>
#endif

class GPSWrapper {
   public:
    bool begin();
    void update();
    float getSpeedKmh();
    void getTimeJST(char *buffer, size_t size);

#ifndef ARDUINO_ARCH_SPRESENSE
    static void setMockSpeed(float speed);
    static void setMockTime(int hour, int minute, int second);
#endif

   private:
#ifdef ARDUINO_ARCH_SPRESENSE
    SpGnss gnss;
    SpNavData navData;
#else
    static float mockSpeed;
    static unsigned long lastUpdate;
    static int mockHour;
    static int mockMinute;
    static int mockSecond;
#endif
    bool isFixed;
};
