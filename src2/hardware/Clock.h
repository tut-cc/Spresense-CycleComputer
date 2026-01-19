#pragma once

#include <GNSS.h>
#include <RTC.h>

class Clock {
public:
  void begin() {
    RTC.begin();
  }

  // GNSS時刻（UTC）でRTCを同期
  void sync(const SpGnssTime &gpsTime) {
    // 異常値チェック
    if (gpsTime.year < 2024) return;

    RtcTime rtcTime;
    rtcTime.year(gpsTime.year);
    rtcTime.month(gpsTime.month);
    rtcTime.day(gpsTime.day);
    rtcTime.hour(gpsTime.hour);
    rtcTime.minute(gpsTime.minute);
    rtcTime.second(gpsTime.sec);

    RTC.setTime(rtcTime);
  }

  // 現在時刻（UTC）を取得
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
