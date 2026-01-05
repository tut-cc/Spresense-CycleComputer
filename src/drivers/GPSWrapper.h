#pragma once

#ifdef ARDUINO_ARCH_SPRESENSE
#include <GNSS.h>
#endif

class GPSWrapper {
   public:
    GPSWrapper();
    bool begin();
    void update();
    float getSpeedKmh();
    void getTimeJST(char *buffer, size_t size);

   private:
#ifdef ARDUINO_ARCH_SPRESENSE
    SpGnss gnss;
    SpNavData navData;
#else
    // Arduino 用のモックデータ
    float mockSpeed;
    unsigned long lastUpdate;
    int mockHour;
    int mockMinute;
    int mockSecond;
#endif
    bool isFixed;
};
