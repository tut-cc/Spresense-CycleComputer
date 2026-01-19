#pragma once
/**
 * @file Clock.h
 * @brief リアルタイムクロック(RTC)の管理
 *
 * GPSから時刻を同期し、RTCで時刻を保持します。
 */

#include <GNSS.h>
#include <RTC.h>

/**
 * @class Clock
 * @brief RTCを管理するクラス
 */
class Clock {
public:
  /** @brief RTC初期化 */
  void begin() { RTC.begin(); }

  /**
   * @brief GPS時刻でRTCを同期
   * @param gpsTime GPS時刻
   * @note 2026年未満の時刻は無効とみなして無視
   */
  void sync(const SpGnssTime &gpsTime) {
    if (gpsTime.year < 2026) return; // GPS同期前は無視

    RtcTime rtcTime;
    rtcTime.year(gpsTime.year);
    rtcTime.month(gpsTime.month);
    rtcTime.day(gpsTime.day);
    rtcTime.hour(gpsTime.hour);
    rtcTime.minute(gpsTime.minute);
    rtcTime.second(gpsTime.sec);

    RTC.setTime(rtcTime);
  }

  /**
   * @brief 現在時刻を取得
   * @return SpGnssTime形式の時刻
   */
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
