#pragma once

class RtcTime {
  int _y, _m, _d, _h, _mi, _s;

public:
  RtcTime(int y = 2024, int m = 1, int d = 1, int h = 0, int mi = 0, int s = 0)
      : _y(y), _m(m), _d(d), _h(h), _mi(mi), _s(s) {}
  int  year() const { return _y; }
  void year(int v) { _y = v; }
  int  month() const { return _m; }
  void month(int v) { _m = v; }
  int  day() const { return _d; }
  void day(int v) { _d = v; }
  int  hour() const { return _h; }
  void hour(int v) { _h = v; }
  int  minute() const { return _mi; }
  void minute(int v) { _mi = v; }
  int  second() const { return _s; }
  void second(int v) { _s = v; }
};

class RtcClass {
public:
  void    begin() {}
  void    setTime(RtcTime) {}
  RtcTime getTime() { return RtcTime(); }
};

extern RtcClass RTC;
