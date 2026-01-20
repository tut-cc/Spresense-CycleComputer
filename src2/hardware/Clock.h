#pragma once

/**
 * @file Clock.h
 * @brief RTCを使用したシステム時刻管理
 *
 * GPS時刻からRTCへの同期と、現在時刻の取得を行います。
 */

#include "../common/Config.h"
#include <GNSS.h>
#include <RTC.h>

class Clock {
public:
  void begin() { RTC.begin(); }

  void sync(const SpGnssTime &gpsTime) {

    if (gpsTime.year < Config::Time::MIN_VALID_YEAR) return;

    RtcTime rtcTime;
    rtcTime.year(gpsTime.year);
    rtcTime.month(gpsTime.month);
    rtcTime.day(gpsTime.day);
    rtcTime.hour(gpsTime.hour);
    rtcTime.minute(gpsTime.minute);
    rtcTime.second(gpsTime.sec);

    RTC.setTime(rtcTime);
  }

  SpGnssTime now() {
    RtcTime    rtcTime = RTC.getTime();
    SpGnssTime t;
    t.year   = rtcTime.year();
    t.month  = rtcTime.month();
    t.day    = rtcTime.day();
    t.hour   = rtcTime.hour();
    t.minute = rtcTime.minute();
    t.sec    = rtcTime.second();
    t.usec   = 0;
    return t;
  }
};
