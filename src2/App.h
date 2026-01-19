#pragma once
/**
 * @file App.h
 * @brief サイクルコンピュータのメインアプリケーションクラス
 *
 * このクラスは、サイクルコンピュータのすべての機能を統合・制御します。
 * - GNSSからの位置情報取得
 * - 速度・距離・時間の計算
 * - ユーザー入力（ボタン操作）の処理
 * - OLED画面への表示
 * - EEPROMへのデータ永続化
 *
 * ダブルバッファリングを使用して、効率的なデータ更新と表示を実現しています。
 */

#include <Arduino.h>
#include <stddef.h>

#include "domain/DataStore.h"
#include "domain/DisplayLogic.h"
#include "domain/GnssAdapter.h"
#include "domain/InputLogic.h"
#include "domain/PersistenceLogic.h"
#include "domain/TripLogic.h"
#include "domain/VoltageMonitor.h"
#include "hardware/Clock.h"
#include "hardware/Gnss.h"
#include "ui/FrameLogic.h"
#include "ui/UI.h"

/**
 * @class App
 * @brief アプリケーションのメインクラス
 *
 * setup()でbegin()を呼び、loop()でupdate()を呼ぶだけで動作します。
 */
class App {
private:
  // ==================== ハードウェア関連 ====================
  Gnss           gnss;           ///< GNSSモジュール制御
  Clock          systemClock;    ///< RTC（リアルタイムクロック）
  DataStore      dataStore;      ///< EEPROMデータ管理
  VoltageMonitor voltageMonitor; ///< バッテリー電圧監視
  UI             userInterface;  ///< ユーザーインターフェース（ボタン + OLED）

  // ==================== 状態管理 ====================
  Mode currentMode = Mode::SPD_TIM; ///< 現在の表示モード

  /**
   * GNSSから取得した最新データ
   */
  GnssData gnssData;

  /**
   * 走行状態のダブルバッファ
   * [0]と[1]を交互に使用し、前回値との差分を効率的に検出します。
   */
  TripState tripState[2];

  /**
   * 表示フレームのダブルバッファ
   * 画面更新が必要かどうかを判定するために使用します。
   */
  DisplayFrame frames[2];

  /**
   * 保存データのダブルバッファ
   * 前回保存時からの変更有無を判定するために使用します。
   */
  SaveData saveBuffers[2];

  int currentIdx = 0; ///< 現在の走行状態バッファインデックス
  int frameIdx   = 0; ///< 現在の表示フレームバッファインデックス
  int saveIdx    = 0; ///< 現在の保存データバッファインデックス

  unsigned long lastSaveMs     = 0; ///< 最後にEEPROMに保存した時刻
  unsigned long lastUiUpdateMs = 0; ///< 最後にUI更新した時刻

public:
  App() = default;

  /**
   * @brief アプリケーションの初期化
   *
   * setup()で1回だけ呼び出してください。
   * - 各ハードウェアの初期化
   * - EEPROMからの保存データ読み込み
   * - 走行状態の初期化
   */
  void begin() {
    gnss.begin();
    systemClock.begin();
    voltageMonitor.begin();
    userInterface.begin();

    // EEPROMから前回の走行データを読み込む
    SaveData saved = dataStore.load();

    // 両方のバッファに同じ初期値を設定
    for (auto &state : tripState) {
      state.resetAll();
      state.distance.total = saved.totalDistance;
      state.distance.trip  = saved.tripDistance;
      state.time.moving    = saved.movingTimeMs;
      state.speed.max      = saved.maxSpeed;
    }

    saveBuffers[0] = saved;
    saveBuffers[1] = saved;

    lastSaveMs = millis();
  }

  /**
   * @brief メインループ処理
   *
   * loop()で繰り返し呼び出してください。
   * 以下の処理を順番に実行します：
   * 1. GNSSデータの取得
   * 2. ボタン入力の処理
   * 3. RTCの同期（GPS時刻から）
   * 4. 走行データの計算
   * 5. データの保存（一定間隔で）
   * 6. 画面の更新（必要な場合のみ）
   */
  void update() {
    const unsigned long now = millis();

    // ダブルバッファのインデックスを切り替え
    // prevIdx: 前回の状態, currIdx: 今回更新する状態
    const int prevIdx = currentIdx;
    const int currIdx = 1 - currentIdx;

    // 前回の状態をコピーしてから更新
    tripState[currIdx] = tripState[prevIdx];
    tripState[currIdx].resetMeta(); // 更新フラグをリセット

    // GNSSデータを取得
    gnssData           = GnssAdapter::collect(gnss);
    Input::Event event = userInterface.getInputEvent();

    // GPS信号が有効なら、RTCをGPS時刻で同期
    if (gnssData.status == UpdateStatus::Updated &&
        (SpFixMode)gnssData.navData.posFixMode != FixInvalid) {
      systemClock.sync(gnssData.navData.time);
    }

    // ボタンイベントの処理
    if (event != Input::Event::NONE) {
      auto result = InputLogic::handleEvent(tripState[currIdx], currentMode, event);
      currentMode = result.newMode;

      // 全データリセットが要求された場合
      if (result.shouldClearStorage) {
        dataStore.clear();
        userInterface.showResetMessage();

        // 表示フレームをクリア
        frames[0] = DisplayFrame();
        frames[1] = DisplayFrame();

        // 保存バッファもクリア
        TripState emptyState;
        emptyState.resetAll();
        SaveData emptySave = PersistenceLogic::create(emptyState, 0.0f);
        saveBuffers[0]     = emptySave;
        saveBuffers[1]     = emptySave;
      }
    }

    // 走行データを計算（速度・距離・時間）
    TripLogic::computeTrip(tripState[currIdx], gnssData, now);

    // 定期的にデータを保存
    handleSave(tripState[currIdx], now);

    // 必要に応じて画面を更新
    handleUI(tripState[prevIdx], tripState[currIdx], now);

    // 現在のバッファインデックスを更新（次回は逆のバッファを使う）
    currentIdx = currIdx;
  }

private:
  /**
   * @brief データ保存処理
   * @param state 現在の走行状態
   * @param now 現在時刻（ミリ秒）
   *
   * 以下の条件がすべて満たされた場合にのみ保存します：
   * - 前回の保存から一定時間（30秒）経過
   * - GNSSがアイドル状態（更新処理中でない）
   * - 前回保存時からデータに変更がある
   */
  void handleSave(const TripState &state, unsigned long now) {
    // 保存間隔のチェック
    if (now - lastSaveMs < DataStore::SAVE_INTERVAL_MS) return;

    // GNSS更新中は保存しない（CPUリソースを節約）
    if (gnssData.status != UpdateStatus::NoChange) return;

    // 現在のバッテリー電圧を取得
    float    v     = voltageMonitor.update();
    SaveData pData = PersistenceLogic::create(state, v);

    // ダブルバッファで変更を検出
    const int prevSaveIdx = saveIdx;
    saveIdx               = 1 - saveIdx;
    saveBuffers[saveIdx]  = pData;

    // 変更があった場合のみ実際に保存（EEPROM書き込み回数を削減）
    if (saveBuffers[saveIdx] != saveBuffers[prevSaveIdx]) dataStore.save(saveBuffers[saveIdx]);
    lastSaveMs = now;
  }

  /**
   * @brief UI更新処理
   * @param prev 前回の走行状態
   * @param curr 現在の走行状態
   * @param now 現在時刻（ミリ秒）
   *
   * 以下のいずれかの条件で画面を更新します：
   * - 走行データに変更があった
   * - 強制更新フラグが設定されている
   * - GNSSデータが更新された
   * - 前回の更新から500ms以上経過（定期更新）
   */
  void handleUI(const TripState &prev, const TripState &curr, unsigned long now) {
    bool periodic = (now - lastUiUpdateMs >= 500);                    // 500ms間隔の定期更新
    bool changed  = TripLogic::isChanged(prev, curr);                 // データ変更検出
    bool forced   = (curr.updateStatus == UpdateStatus::ForceUpdate); // 強制更新
    bool gnssUpd  = (gnssData.status == UpdateStatus::Updated);       // GNSS更新

    if (changed || forced || gnssUpd || periodic) {
      // RTCから現在時刻を取得
      SpGnssTime currentTime = systemClock.now();

      // 表示用データを生成
      DisplayState dData = DisplayLogic::create(curr, gnssData, currentTime, currentMode);

      // フレームを生成（ダブルバッファリング）
      const int prevFrameIdx = frameIdx;
      frameIdx               = 1 - frameIdx;
      frames[frameIdx]       = FrameLogic::buildFrame(dData);

      // フレームに変更があった場合のみ描画
      if (frames[frameIdx] != frames[prevFrameIdx]) {
        userInterface.draw(frames[frameIdx]);
        lastUiUpdateMs = now;
      }
    }
  }
};
