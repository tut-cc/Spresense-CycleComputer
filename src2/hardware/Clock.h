#pragma once

#include "../Config.h"
#include <GNSS.h>
#include <RTC.h>

class Clock {
public:
  void begin() { RTC.begin(); }
  void sync(const SpGnssTime &gt) {
    if (gt.year < Config::Time::MIN_VALID_YEAR) return;
    RtcTime rt(gt.year, gt.month, gt.day, gt.hour, gt.minute, gt.sec);
    RTC.setTime(rt);
  }
  SpGnssTime now() {
    RtcTime rt = RTC.getTime();
    return {(unsigned short)rt.year(),
            (unsigned char)rt.month(),
            (unsigned char)rt.day(),
            (unsigned char)rt.hour(),
            (unsigned char)rt.minute(),
            (unsigned char)rt.second(),
            0};
  }
};
