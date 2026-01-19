#pragma once
/**
 * @file DataStructures.h
 * @brief アプリケーション全体で使用するデータ構造の定義
 *
 * サイクルコンピュータで使用する各種データ構造を定義しています。
 * - GnssData: GNSS（衛星測位）データ
 * - TripState: 走行状態（速度・距離・時間など）
 * - DisplayState: 画面表示用データ
 * - SaveData: EEPROM保存用データ
 * - DisplayFrame: 描画フレーム
 */

#include <GNSS.h>
#include <cstring>

/**
 * @brief データ更新状態を表す列挙型
 *
 * データの変更状態を追跡するために使用します。
 */
enum class UpdateStatus {
  NoChange,   // 変更なし
  Updated,    // 通常の更新
  ForceUpdate // 強制更新（リセット後など）
};

/**
 * @brief 表示モードを表す列挙型
 *
 * サイクルコンピュータの3つの表示モードを定義しています。
 */
enum class Mode {
  SPD_TIM, // 現在速度 + 経過時間
  AVG_ODO, // 平均速度 + 総走行距離
  MAX_CLK  // 最高速度 + 現在時刻
};

/**
 * @brief GNSS（衛星測位）データ構造
 *
 * GNSSモジュールから取得した位置情報を保持します。
 */
struct GnssData {
  SpNavData     navData;   ///< ナビゲーションデータ（位置・速度など）
  unsigned long timestamp; ///< データ取得時刻（ミリ秒）
  UpdateStatus  status;    ///< 更新状態

  /**
   * @brief データが更新されたかを確認
   * @return 更新された場合true
   */
  bool isUpdated() const { return status == UpdateStatus::Updated; }
};

/**
 * @brief 走行状態の基底構造体
 *
 * 走行中の各種データを保持します。継承して拡張可能です。
 */
struct TripStateBase {
  /**
   * @brief 走行状態を表す列挙型
   */
  enum class Status {
    Stopped, // 停止中
    Moving,  // 走行中
    Paused   // 一時停止中
  };

  /**
   * @brief 速度関連データ
   */
  struct Speed {
    float current; ///< 現在速度（km/h）
    float max;     ///< 最高速度（km/h）
    float avg;     ///< 平均速度（km/h）
  };

  /**
   * @brief 距離関連データ
   */
  struct Distance {
    float total; ///< ODO: 総走行距離（km）
    float trip;  ///< TRP: トリップ距離（km）
  };

  /**
   * @brief 時間関連データ
   */
  struct Time {
    unsigned long elapsed; ///< 経過時間（ミリ秒）
    unsigned long moving;  ///< 走行時間（ミリ秒）
  };

  Status    status;  ///< 現在の走行状態
  SpFixMode fixMode; ///< GPS受信状態

  Speed    speed;    ///< 速度データ
  Distance distance; ///< 距離データ
  Time     time;     ///< 時間データ

  unsigned long lastUpdateTime; ///< 最後に更新した時刻
  UpdateStatus  updateStatus;   ///< 更新状態フラグ

  /**
   * @brief 更新フラグをリセット（NoChangeに設定）
   */
  void resetMeta() { updateStatus = UpdateStatus::NoChange; }

  /**
   * @brief 強制更新フラグを設定
   */
  void forceUpdate() { updateStatus = UpdateStatus::ForceUpdate; }

  /**
   * @brief 一時停止中かどうかを確認
   * @return 一時停止中の場合true
   */
  bool isPaused() const { return status == Status::Paused; }

  /**
   * @brief 走行中かどうかを確認
   * @return 走行中の場合true
   */
  bool isMoving() const { return status == Status::Moving; }
};

/**
 * @brief 拡張版の走行状態構造体
 *
 * TripStateBaseを継承し、距離計算の精度向上のための残差を追加しています。
 */
struct TripState : public TripStateBase {
  /**
   * @brief 距離計算の残差（小さな距離の累積用）
   *
   * 短時間での距離増分が小さすぎて失われないよう、
   * 一定量に達するまで蓄積しておくための変数です。
   */
  float distanceResidue = 0.0f;

  /**
   * @brief すべてのデータを初期化
   *
   * ODO含むすべてのデータをゼロにリセットします。
   */
  void resetAll() {
    speed.current  = 0.0f;
    status         = Status::Stopped;
    time.elapsed   = 0;
    speed.max      = 0.0f;
    distance.total = 0.0f;
    distance.trip  = 0.0f;
    time.moving    = 0;
    speed.avg      = 0.0f;
    lastUpdateTime = 0;

    distanceResidue = 0.0f;
    forceUpdate();
  }

  /**
   * @brief トリップデータのみリセット
   *
   * ODO（総走行距離）以外をリセットします。
   */
  void resetTrip() {
    speed.current   = 0.0f;
    status          = Status::Stopped;
    time.elapsed    = 0;
    distance.trip   = 0.0f;
    time.moving     = 0;
    speed.avg       = 0.0f;
    distanceResidue = 0.0f;
    forceUpdate();
  }

  /**
   * @brief 最高速度のみリセット
   */
  void resetMaxSpeed() {
    speed.max = 0.0f;
    forceUpdate();
  }
};

/**
 * @brief 画面表示用のデータ構造
 *
 * FrameLogicで最終的なDisplayFrameを生成するための中間データです。
 */
struct DisplayState {
  /**
   * @brief サブ表示の種類
   */
  enum class SubType {
    Duration, // 経過時間（SPD_TIMモード用）
    Distance, // 距離（AVG_ODOモード用）
    Clock     // 時刻（MAX_CLKモード用）
  };

  SpFixMode   fixMode;        ///< GPS受信状態
  const char *modeSpeedLabel; ///< 速度ラベル: "SPD", "AVG", "MAX"
  const char *modeTimeLabel;  ///< 時間ラベル: "Time", "Odo", "Clock"

  float       mainValue; ///< メイン表示の速度値
  const char *mainUnit;  ///< メイン表示の単位: "km/h"

  SubType subType; ///< サブ表示の種類
  union {
    unsigned long durationMs; ///< SPD_TIMモード用: 経過時間（ミリ秒）
    float         distanceKm; ///< AVG_ODOモード用: 距離（km）
    struct {
      int hour;   ///< MAX_CLKモード用: 時
      int minute; ///< MAX_CLKモード用: 分
    } clockTime;
  } subValue;

  const char  *subUnit;      ///< サブ表示の単位
  bool         shouldBlink;  ///< 点滅フラグ（一時停止中に使用）
  UpdateStatus updateStatus; ///< 更新状態
};

/** @brief 保存データの識別用マジックナンバー */
constexpr uint32_t SAVE_DATA_MAGIC_NUMBER = 0xDEADBEEF;

/**
 * @brief EEPROM保存用データ構造
 *
 * 電源OFFでも保持したいデータを格納します。
 */
struct SaveData {
  uint32_t magicNumber; ///< データ有効性確認用マジックナンバー

  float         totalDistance; ///< 総走行距離（km）
  float         tripDistance;  ///< トリップ距離（km）
  unsigned long movingTimeMs;  ///< 走行時間（ミリ秒）
  float         maxSpeed;      ///< 最高速度（km/h）
  float         voltage;       ///< バッテリー電圧（V）

  UpdateStatus updateStatus; ///< 更新状態

  uint32_t crc; ///< データ整合性確認用CRC

  /**
   * @brief 等価比較演算子
   * @note CRCは比較に含めない
   */
  bool operator==(const SaveData &other) const {
    return magicNumber == other.magicNumber && totalDistance == other.totalDistance &&
           tripDistance == other.tripDistance && movingTimeMs == other.movingTimeMs &&
           maxSpeed == other.maxSpeed && voltage == other.voltage;
  }

  bool operator!=(const SaveData &other) const { return !(*this == other); }
};

/**
 * @brief OLED画面の描画フレーム
 *
 * 画面に描画する内容をすべて含む構造体です。
 * ダブルバッファリングで使用し、前回と比較して変更があった場合のみ描画します。
 */
struct DisplayFrame {
  /**
   * @brief ヘッダー部分（画面上部）
   */
  struct Header {
    const char *fixStatus; ///< GPS状態: "WAIT", "2D", "3D"
    const char *modeSpeed; ///< 速度モード: "SPD", "AVG", "MAX"
    const char *modeTime;  ///< 時間モード: "Time", "Odo", "Clock"

    Header() : fixStatus(""), modeSpeed(""), modeTime("") {}

    bool operator==(const Header &other) const {
      return fixStatus == other.fixStatus && modeSpeed == other.modeSpeed &&
             modeTime == other.modeTime;
    }

    bool operator!=(const Header &other) const { return !(*this == other); }
  };

  /**
   * @brief 表示項目（値と単位のペア）
   */
  struct Item {
    char        value[16]; ///< 表示する値（文字列として格納）
    const char *unit;      ///< 単位

    Item() : unit("") { memset(value, 0, sizeof(value)); }

    bool operator==(const Item &other) const {
      return strcmp(value, other.value) == 0 && unit == other.unit;
    }

    bool operator!=(const Item &other) const { return !(*this == other); }
  };

  Header header; ///< ヘッダー部分
  Item   main;   ///< メイン表示（速度）
  Item   sub;    ///< サブ表示（時間/距離/時刻）

  DisplayFrame() = default;

  bool operator==(const DisplayFrame &other) const {
    return header == other.header && main == other.main && sub == other.sub;
  }

  bool operator!=(const DisplayFrame &other) const { return !(*this == other); }
};
