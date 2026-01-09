#pragma once

#include <stdint.h>

// Mock constants
#define GPS 0
#define QZ_L1CA 1
#define COLD_START 0
#define HOT_START 1
// Fix modes
#define FixInvalid 0
#define Fix2D 1
#define Fix3D 2

struct SpNavTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int usec;
};

struct SpNavData {
    SpNavTime time;
    float velocity; // m/s
    int posFixMode;
    double latitude;
    double longitude;
    float altitude; // not used but good to have
    int numSatellites;
};

class SpGnss {
public:
    int begin();
    int start(int mode);
    int stop();
    void select(int satelliteSystem);
    bool waitUpdate(int timeout);
    void getNavData(SpNavData* navData);

    // Mock control
    static SpNavTime mockTimeData;
    static float mockVelocityData;
};
