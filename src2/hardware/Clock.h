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

  /// GPS時刻をRTCに同期
  void sync(const SpGnssTime &gpsTime) {
    // GPS初期化直後は無効な日時が返されるため、妥当性をチェック
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
