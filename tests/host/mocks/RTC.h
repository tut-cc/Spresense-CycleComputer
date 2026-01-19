#pragma once

class RtcTime {
public:
  int year() const {
    return 2024;
  }
  void year(int) {}
  int  month() const {
    return 1;
  }
  void month(int) {}
  int  day() const {
    return 1;
  }
  void day(int) {}
  int  hour() const {
    return 0;
  }
  void hour(int) {}
  int  minute() const {
    return 0;
  }
  void minute(int) {}
  int  second() const {
    return 0;
  }
  void second(int) {}
};

class RtcClass {
public:
  void    begin() {}
  void    setTime(RtcTime) {}
  RtcTime getTime() {
    return RtcTime();
  }
};

extern RtcClass RTC;
