#pragma once
/**
 * @file Gnss.h
 * @brief GNSSモジュールの制御クラス
 *
 * Sony SpresenseのGNSSライブラリをラップして、
 * シンプルなインターフェースを提供します。
 */

#include <GNSS.h>

/**
 * @class Gnss
 * @brief GNSSモジュールの制御
 */
class Gnss {
private:
  SpGnss    gnss;      ///< Spresense GNSSオブジェクト
  SpNavData navData{}; ///< 最新のナビゲーションデータ

public:
  Gnss() {}

  /**
   * @brief GNSS初期化
   * @return 成功時true
   */
  bool begin() {
    if (gnss.begin() != 0) return false;
    selectSatellites();
    if (gnss.start(COLD_START) != 0) return false;
    return true;
  }

  /**
   * @brief データ更新を試行
   * @return 新しいデータがあればtrue
   */
  bool update() {
    if (gnss.waitUpdate(0) != 1) return false; // ノンブロッキング
    gnss.getNavData(&navData);
    return true;
  }

  /** @brief 最新のナビゲーションデータを取得 */
  SpNavData getNavData() const { return navData; }

private:
  /**
   * @brief 使用する衛星システムを選択
   *
   * 日本で使用するために、以下を有効化:
   * - GPS: 米国の衛星測位システム
   * - GLONASS: ロシアの衛星測位システム
   * - Galileo: EUの衛星測位システム
   * - QZSS L1C/A, L1S: 日本の準天頂衛星システム
   */
  void selectSatellites() {
    gnss.select(GPS);
    gnss.select(GLONASS);
    gnss.select(GALILEO);
    gnss.select(QZ_L1CA);
    gnss.select(QZ_L1S);
  }
};
