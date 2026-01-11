#pragma once

#include <cstdint>

namespace ui {

class GraphicsContext {
public:
  virtual ~GraphicsContext() = default;

  virtual void clear()                                                                                                     = 0;
  virtual void display()                                                                                                   = 0;
  virtual void setTextSize(int size)                                                                                       = 0;
  virtual void setTextColor(int color)                                                                                     = 0;
  virtual void setCursor(int x, int y)                                                                                     = 0;
  virtual void print(const char *text)                                                                                     = 0;
  virtual void drawLine(int x0, int y0, int x1, int y1, int color)                                                         = 0;
  virtual void drawRect(int x, int y, int w, int h, int color)                                                             = 0;
  virtual void fillRect(int x, int y, int w, int h, int color)                                                             = 0;
  virtual void drawCircle(int x0, int y0, int r, int color)                                                                = 0;
  virtual void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) = 0;

  virtual int getWidth() const  = 0;
  virtual int getHeight() const = 0;
};

} // namespace ui
