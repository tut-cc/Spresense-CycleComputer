#pragma once

#include <GNSS.h>

/**
 * @file BaseTypes.h
 * @brief 基本的なデータ型の定義
 */

/// 表示モードを示す列挙型
enum class Mode { SPD_TIM, AVG_ODO, MAX_CLK };

/// GNSSから取得したデータを保持する構造体
struct GnssData {
  SpNavData     navData;
  unsigned long timestamp;
  bool          updated;

  bool isUpdated() const { return updated; }
};
