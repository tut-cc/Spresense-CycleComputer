#pragma once

#ifdef ARDUINO_ARCH_SPRESENSE
#include <GNSS.h>
#endif

class GPSWrapper {
   public:
    bool begin();
    void update();
    float getSpeedKmh();
    void getTimeJST(char *buffer, size_t size);

   private:
#ifdef ARDUINO_ARCH_SPRESENSE
    SpGnss gnss;
    SpNavData navData;
#else
    float mockSpeed = 0;
    unsigned long lastUpdate = 0;
    int mockHour = 0;
    int mockMinute = 0;
    int mockSecond = 0;
#endif
    bool isFixed;
};
