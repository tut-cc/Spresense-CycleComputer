#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct DrawCall {
  enum class Type {
    Pixel,
    Line,
    Rect,
    FillRect,
    Circle,
    Text,
    Cursor,
    TextSize,
    TextColor,
    Display,
    Clear
  };
  Type        type;
  int16_t     x0, y0, x1, y1, color, size, r;
  std::string text;
};

class DisplayLogger {
public:
  static std::vector<DrawCall> calls;
  static void                  log(DrawCall call) {
    calls.push_back(call);
  }
  static void clear() {
    calls.clear();
  }
};
