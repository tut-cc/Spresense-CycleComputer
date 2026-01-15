#pragma once

#include <cstring>

struct Frame {
  struct Item {
    char value[16] = "";
    char unit[16]  = "";

    bool operator==(const Item &other) const {
      return strcmp(value, other.value) == 0 && strcmp(unit, other.unit) == 0;
    }
  };

  struct Header {
    char fixStatus[8] = "";
    char modeSpeed[8] = "";
    char modeTime[8]  = "";

    bool operator==(const Header &other) const {
      const bool fixStatusEq = strcmp(fixStatus, other.fixStatus) == 0;
      const bool modeSpeedEq = strcmp(modeSpeed, other.modeSpeed) == 0;
      const bool modeTimeEq  = strcmp(modeTime, other.modeTime) == 0;
      return fixStatusEq && modeSpeedEq && modeTimeEq;
    }
  };

  Header header;
  Item   main;
  Item   sub;

  Frame() = default;

  bool operator==(const Frame &other) const {
    return header == other.header && main == other.main && sub == other.sub;
  }
};
