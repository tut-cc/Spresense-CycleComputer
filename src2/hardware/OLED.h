#pragma once
/**
 * @file OLED.h
 * @brief OLEDディスプレイの制御クラス
 *
 * Adafruit SSD1306ライブラリをラップして、
 * シンプルなインターフェースを提供します。
 * 128x64ピクセルのI2C接続OLEDに対応。
 */

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

constexpr int WIDTH   = 128;  ///< 画面幅(px)
constexpr int HEIGHT  = 64;   ///< 画面高さ(px)
constexpr int ADDRESS = 0x3C; ///< I2Cアドレス

/**
 * @class OLED
 * @brief OLEDディスプレイ制御
 */
class OLED {
public:
  /** @brief 矩形領域を表す構造体 */
  struct Rect {
    int16_t  x;
    int16_t  y;
    uint16_t w;
    uint16_t h;
  };

private:
  Adafruit_SSD1306 ssd1306; ///< SSD1306ドライバ

public:
  OLED() : ssd1306(WIDTH, HEIGHT, &Wire, -1) {}

  /**
   * @brief 初期化
   * @return 成功時true
   */
  bool begin() {
    Wire.setClock(400000); // I2C高速モード(400kHz)
    if (!ssd1306.begin(SSD1306_SWITCHCAPVCC, ADDRESS)) return false;
    ssd1306.clearDisplay();
    ssd1306.display();
    return true;
  }

  void restart() { begin(); }
  void clear() { ssd1306.clearDisplay(); }
  void display() { ssd1306.display(); }
  void setTextSize(int size) { ssd1306.setTextSize(size); }
  void setTextColor(int color) { ssd1306.setTextColor(color); }
  void setCursor(int x, int y) { ssd1306.setCursor(x, y); }
  void print(const char *text) { ssd1306.print(text); }

  void drawLine(int x0, int y0, int x1, int y1, int color) {
    ssd1306.drawLine(x0, y0, x1, y1, color);
  }

  /**
   * @brief テキストの描画領域を取得
   * @param string 対象文字列
   * @return 描画領域(Rect)
   */
  Rect getTextBounds(const char *string) {
    Rect rect;
    ssd1306.getTextBounds(string, 0, 0, &rect.x, &rect.y, &rect.w, &rect.h);
    return rect;
  }

  int getWidth() const { return WIDTH; }
  int getHeight() const { return HEIGHT; }
};
