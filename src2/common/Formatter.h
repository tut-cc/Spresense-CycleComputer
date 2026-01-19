#pragma once
/**
 * @file Formatter.h
 * @brief 数値のフォーマット（文字列変換）ユーティリティ
 *
 * 速度・距離・時間・時刻をOLED表示用の文字列に変換する関数群です。
 * snprintfなど標準ライブラリを避け、メモリ効率を重視した実装になっています。
 */

#include <math.h>
#include <stddef.h>

namespace Formatter {

/**
 * @brief 内部実装用の名前空間
 *
 * 外部から直接呼び出すことは想定していません。
 */
namespace Internal {

/**
 * @brief 文字列を反転させる
 * @param begin 開始ポインタ
 * @param end 終了ポインタ
 *
 * 数値を桁ごとに取り出すと逆順になるため、最後に反転が必要です。
 */
inline void reverse(char *begin, char *end) {
  char t;
  while (begin < end) {
    t        = *begin;
    *begin++ = *end;
    *end--   = t;
  }
}

/**
 * @brief 整数を文字列に変換（内部実装）
 * @param value 変換する整数値
 * @param str 出力先バッファ
 * @return 書き込み終了位置のポインタ
 * @note 結果は反転した状態で格納されます
 */
inline char *itoa_impl(int value, char *str) {
  char *p = str;
  int   v = value;
  if (v < 0) v = -v; // 負数は絶対値に変換

  // 各桁を1桁ずつ取り出して文字化
  do {
    *p++ = (char)('0' + (v % 10));
    v /= 10;
  } while (v > 0);

  return p; // 終端位置を返す
}

/**
 * @brief ゼロ埋め付きで整数を文字列に変換
 * @param value 変換する整数値
 * @param str 出力先バッファ
 * @param digits 出力桁数
 *
 * 例: value=5, digits=2 → "05"
 */
inline void itoa_pad(int value, char *str, int digits) {
  char *p = itoa_impl(value, str);
  while ((p - str) < digits) *p++ = '0'; // 足りない桁を'0'で埋める
  *p = '\0';
  reverse(str, p - 1); // 反転して正しい順序に
}

/**
 * @brief 浮動小数点数を指定精度で四捨五入
 * @param value 四捨五入する値
 * @param precision 小数点以下の桁数
 * @return 四捨五入された値
 */
inline float roundFloat(float value, int precision) {
  float mul = 1.0f;
  for (int i = 0; i < precision; ++i) mul *= 10.0f;

  float rounded = value;
  if (value >= 0.0f) rounded = floorf(value * mul + 0.5f) / mul;
  else rounded = ceilf(value * mul - 0.5f) / mul;

  return rounded;
}

/**
 * @brief 整数部分を逆順でバッファに書き込む
 * @param value 整数値
 * @param buffer 出力先バッファ
 * @return 終端位置のポインタ
 */
inline char *writeIntPart(int value, char *buffer) {
  char *p = buffer;
  if (value == 0) {
    *p++ = '0';
  } else {
    while (value > 0) {
      *p++ = '0' + (value % 10);
      value /= 10;
    }
  }
  return p;
}

/**
 * @brief パディング（空白埋め）しながら逆順の整数部分をコピー
 * @param dest 出力先
 * @param srcStart 元データの開始位置
 * @param srcEnd 元データの終了位置
 * @param totalWidth 目標幅
 * @return 書き込み終了位置
 */
inline char *copyAndPad(char *dest, char *srcStart, char *srcEnd, int totalWidth) {
  int intLen  = (int)(srcEnd - srcStart);
  int padding = totalWidth - intLen;

  // 左側をスペースで埋める
  while (padding > 0) {
    *dest++ = ' ';
    padding--;
  }

  // 逆順になっているソースを正順でコピー
  while (srcEnd > srcStart) { *dest++ = *--srcEnd; }
  return dest;
}

/**
 * @brief 小数部分を文字列として書き込む
 * @param frac 小数部分（0.0〜1.0未満）
 * @param precision 小数点以下の桁数
 * @param dest 出力先
 * @return 書き込み終了位置
 */
inline char *writeFracPart(float frac, int precision, char *dest) {
  // 小数部分を整数化して桁を取り出す
  int intFrac = (int)(frac * powf(10.0f, precision) + 0.5f);

  // 指定桁数分の数字を取り出す（逆順）
  char  temp[10];
  char *t = temp;

  for (int i = 0; i < precision; ++i) {
    *t++ = '0' + (intFrac % 10);
    intFrac /= 10;
  }

  // 逆順で格納されているので正順に直しながら出力
  while (t > temp) { *dest++ = *--t; }
  return dest;
}

} // namespace Internal

/**
 * @brief 浮動小数点数を固定幅・固定精度の文字列に変換
 * @param value 変換する値（負数は0として扱う）
 * @param buffer 出力先バッファ
 * @param width 最小幅（この幅に満たない場合は左にスペースを追加）
 * @param precision 小数点以下の桁数
 *
 * 例: value=3.5, width=4, precision=1 → " 3.5"
 */
inline void ftoa_fixed(float value, char *buffer, int width, int precision) {
  if (value < 0) value = 0.0f;

  // まず四捨五入
  float mul = 1.0f;
  for (int i = 0; i < precision; ++i) mul *= 10.0f;

  float rounded  = floorf(value * mul + 0.5f);
  int   intPart  = (int)(rounded / mul);
  float rem      = rounded - (intPart * mul);
  float fracPart = rem / mul;

  // 整数部分を一時バッファに書き込み
  char  tempInt[16];
  char *t = Internal::writeIntPart(intPart, tempInt);

  // 必要な幅を計算
  // 幅 = 整数の桁数 + (小数点がある場合は 1 + 小数桁数)
  int contentWidth = (int)(t - tempInt);
  if (precision > 0) contentWidth += 1 + precision;

  char *p = buffer;

  // 左側のパディング（スペース埋め）
  while (contentWidth < width) {
    *p++ = ' ';
    contentWidth++;
  }

  // 整数部分をコピー（逆順から正順に）
  while (t > tempInt) *p++ = *--t;

  // 小数部分をコピー
  if (precision > 0) {
    *p++ = '.'; // 小数点
    p    = Internal::writeFracPart(fracPart, precision, p);
  }
  *p = '\0'; // 終端文字
}

/**
 * @brief 速度をフォーマット
 * @param speedKmh 速度（km/h）
 * @param buffer 出力先バッファ
 * @param size バッファサイズ（未使用、互換性のため）
 *
 * 出力例: "23.5" （幅4、小数1桁）
 */
inline void formatSpeed(float speedKmh, char *buffer, size_t size) {
  (void)size; // 未使用パラメータの警告を抑制
  ftoa_fixed(speedKmh, buffer, 4, 1);
}

/**
 * @brief 距離をフォーマット
 * @param distanceKm 距離（km）
 * @param buffer 出力先バッファ
 * @param size バッファサイズ（未使用）
 *
 * 出力例: "123.45" （幅5、小数2桁）
 */
inline void formatDistance(float distanceKm, char *buffer, size_t size) {
  (void)size;
  ftoa_fixed(distanceKm, buffer, 5, 2);
}

/**
 * @brief 経過時間（ミリ秒）を時:分:秒形式にフォーマット
 * @param millis 経過時間（ミリ秒）
 * @param buffer 出力先バッファ
 * @param size バッファサイズ（未使用）
 *
 * 出力例:
 * - 1時間以上: "1:23:45"
 * - 1時間未満: "23:45"
 */
inline void formatDuration(unsigned long millis, char *buffer, size_t size) {
  (void)size;
  const unsigned long seconds = millis / 1000;
  const unsigned long h       = seconds / 3600;        // 時
  const unsigned long m       = (seconds % 3600) / 60; // 分
  const unsigned long s       = seconds % 60;          // 秒

  char *p = buffer;

  if (h > 0) {
    // 1時間以上の場合: h:mm:ss 形式
    char  temp[10];
    char *t = Internal::itoa_impl((int)h, temp);
    while (t > temp) *p++ = *--t;

    *p++ = ':';
    Internal::itoa_pad((int)m, p, 2); // 分は2桁固定
    p += 2;
  } else {
    // 1時間未満の場合: mm:ss 形式
    Internal::itoa_pad((int)m, p, 2);
    p += 2;
  }

  *p++ = ':';
  Internal::itoa_pad((int)s, p, 2); // 秒は2桁固定
  p += 2;
  *p = '\0';
}

/**
 * @brief 時刻を hh:mm 形式にフォーマット
 * @param h 時（0〜23）
 * @param m 分（0〜59）
 * @param buffer 出力先バッファ
 *
 * 出力例: "09:30"
 */
inline void formatClock(int h, int m, char *buffer) {
  char *p = buffer;
  Internal::itoa_pad(h, p, 2); // 時は2桁固定
  p += 2;
  *p++ = ':';
  Internal::itoa_pad(m, p, 2); // 分は2桁固定
  p += 2;
  *p = '\0';
}

} // namespace Formatter
