#pragma once

#include <stdint.h>

// Mock constants
#define GPS 0
#define GLONASS 2
#define GALILEO 3
#define QZ_L1CA 1
#define QZ_L1S 4
#define COLD_START 0
#define HOT_START 1
typedef int SpStartMode;

enum SpGnssFixType { FixInvalid = 0, Fix2D = 1, Fix3D = 2 };
typedef SpGnssFixType SpFixMode;

struct SpNavTime {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int sec;
  int usec;
};

struct SpNavData {
  SpNavTime     time;
  float         velocity; // m/s
  SpGnssFixType posFixMode;
  double        latitude;
  double        longitude;
  float         altitude; // not used but good to have
  int           numSatellites;

  /**
   * @brief Helper to move coordinates by meters (approximate)
   */
  void moveByMeters(float meters) {
    // Approx 111,111 meters per degree of latitude
    latitude += (double)meters / 111111.0;
  }
};

class SpGnss {
public:
  int  begin();
  int  start(int mode);
  int  stop();
  void select(int satelliteSystem);
  bool waitUpdate(int timeout);
  void getNavData(SpNavData *navData);

  // Mock control
  static SpNavTime mockTimeData;
  static float     mockVelocityData;
  static int       mockBeginResult;
  static int       mockStartResult;
};
