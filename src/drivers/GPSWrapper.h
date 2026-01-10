#pragma once

#include <Arduino.h>
#include <GNSS.h>

class GPSWrapper {
   public:
    bool begin();
    void update();
    float getSpeedKmh() const;
    void getTimeJST(char* buffer, size_t size) const;

   private:
    SpGnss gnss;
    SpNavData navData;

#ifdef UNIT_TEST
   public:
    static void setMockTime(int h, int m, int s);
    static void setMockSpeed(float speedKmh);
#endif
};
