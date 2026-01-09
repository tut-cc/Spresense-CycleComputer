#pragma once

#include <Arduino.h>
#include <GNSS.h>

class GPSWrapper {
   public:
    bool begin();
    void update();
    float getSpeedKmh();
    void getTimeJST(char *buffer, size_t size);

   private:
    SpGnss gnss;
    SpNavData navData;

    bool isFixed;
};
