#pragma once
/**
 * @file DataStore.h
 * @brief EEPROMへのデータ永続化を管理するクラス
 *
 * 走行データ（距離・時間・最高速度など）をEEPROMに保存し、
 * 電源を切っても失われないようにします。
 *
 * データ整合性機能：
 * - マジックナンバーによる有効性チェック
 * - CRC32によるデータ破損検出
 * - 書き込み中断対策（マジックナンバーを先に無効化）
 */

#include "../common/DataStructures.h"
#include <EEPROM.h>
#include <math.h>
#include <stddef.h>

/** @brief CRC32計算用の多項式（IEEE 802.3準拠） */
constexpr uint32_t CRC_POLY = 0xEDB88320;

/** @brief 総走行距離の最大有効値（km） - これを超えるデータは破損とみなす */
constexpr float MAX_VALID_KM = 1000000.0f;

/** @brief EEPROMの保存開始アドレス */
constexpr unsigned long EEPROM_ADDR = 0;

/**
 * @class DataStore
 * @brief EEPROMへのデータ保存・読み込みを管理
 *
 * 使用例:
 * @code
 * DataStore store;
 * SaveData data = store.load();  // 起動時に読み込み
 * // ... データ更新 ...
 * store.save(data);              // 定期的に保存
 * store.clear();                 // リセット時にクリア
 * @endcode
 */
class DataStore {
public:
  /**
   * @brief 保存間隔（ミリ秒）
   *
   * EEPROMの書き込み寿命を考慮して、30秒間隔で保存します。
   * 通常のEEPROMは約10万回の書き込みに耐えられるため、
   * 30秒間隔なら約38日間連続使用可能です。
   */
  static constexpr float SAVE_INTERVAL_MS = 30000.0f;

  /**
   * @brief EEPROMからデータを読み込む
   * @return 読み込んだデータ（無効な場合はデフォルト値）
   *
   * 以下の場合はデフォルト値を返します：
   * - マジックナンバーが一致しない
   * - CRCが一致しない（データ破損）
   * - 距離値がNaNまたは範囲外
   */
  SaveData load() {
    SaveData savedData;
    EEPROM.get(EEPROM_ADDR, savedData);

    const uint32_t calculatedCrc = calculateDataCRC(savedData);

    // データが有効な場合はそのまま返す
    if (isValid(savedData, calculatedCrc)) return savedData;

    // 無効な場合はデフォルト値を生成して返す
    SaveData defaultData;
    defaultData.magicNumber   = SAVE_DATA_MAGIC_NUMBER;
    defaultData.totalDistance = 0.0f;
    defaultData.tripDistance  = 0.0f;
    defaultData.movingTimeMs  = 0;
    defaultData.maxSpeed      = 0.0f;
    defaultData.voltage       = 0.0f;
    defaultData.updateStatus  = UpdateStatus::NoChange;
    defaultData.crc           = calculateDataCRC(defaultData);

    return defaultData;
  }

  /**
   * @brief データをEEPROMに保存
   * @param currentData 保存するデータ
   *
   * 書き込み中断対策として、以下の手順で保存します：
   * 1. マジックナンバーを無効値（0）に設定
   * 2. データ全体を書き込み
   * 3. マジックナンバーが正しく書き込まれる
   *
   * これにより、書き込み中に電源が切れても、
   * 次回起動時に破損データを検出できます。
   */
  void save(const SaveData &currentData) {
    SaveData nextData    = currentData;
    nextData.magicNumber = SAVE_DATA_MAGIC_NUMBER;
    nextData.crc         = calculateDataCRC(nextData);

    // まずマジックナンバーを無効化（書き込み中断対策）
    uint32_t  invalidMagic = 0;
    const int magicAddr    = EEPROM_ADDR + offsetof(SaveData, magicNumber);
    EEPROM.put(magicAddr, invalidMagic);

    // データ全体を書き込み（マジックナンバーも含む）
    EEPROM.put(EEPROM_ADDR, nextData);
  }

  /**
   * @brief 保存データをクリア（全リセット時に使用）
   *
   * すべての値を0にリセットします。
   */
  void clear() {
    // まずマジックナンバーを無効化
    const int magicAddr = EEPROM_ADDR + offsetof(SaveData, magicNumber);
    EEPROM.put(magicAddr, (uint32_t)0);

    // クリーンなデータを生成して保存
    SaveData cleanData;
    cleanData.magicNumber   = SAVE_DATA_MAGIC_NUMBER;
    cleanData.totalDistance = 0.0f;
    cleanData.tripDistance  = 0.0f;
    cleanData.movingTimeMs  = 0;
    cleanData.maxSpeed      = 0.0f;
    cleanData.voltage       = 0.0f;
    cleanData.updateStatus  = UpdateStatus::NoChange;
    cleanData.crc           = calculateDataCRC(cleanData);

    EEPROM.put(EEPROM_ADDR, cleanData);
  }

private:
  /**
   * @brief CRC32を計算
   * @param data データのポインタ
   * @param length データ長（バイト）
   * @return 計算したCRC32値
   *
   * IEEE 802.3準拠のCRC32アルゴリズムを使用しています。
   */
  static uint32_t calcCRC32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF; // 初期値
    for (size_t i = 0; i < length; i++) {
      crc ^= data[i];
      for (int j = 0; j < 8; j++) {
        if (crc & 1) crc = (crc >> 1) ^ CRC_POLY;
        else crc >>= 1;
      }
    }
    return ~crc; // 最終反転
  }

  /**
   * @brief SaveDataのCRCを計算
   * @param data 計算対象のデータ
   * @return CRC32値
   *
   * crcフィールド自体は計算に含めません。
   */
  static uint32_t calculateDataCRC(const SaveData &data) {
    return calcCRC32((const uint8_t *)&data, offsetof(SaveData, crc));
  }

  /**
   * @brief データの有効性を検証
   * @param data 検証するデータ
   * @param calculatedCrc 計算済みのCRC
   * @return 有効な場合true
   *
   * 以下をすべてチェックします：
   * - CRCの一致
   * - マジックナンバーの一致
   * - 距離値がNaNでない
   * - 距離値が0以上
   * - 距離値が最大有効値以下
   */
  static bool isValid(const SaveData &data, uint32_t calculatedCrc) {
    if (calculatedCrc != data.crc) return false;
    if (data.magicNumber != SAVE_DATA_MAGIC_NUMBER) return false;
    if (isnan(data.totalDistance)) return false;
    if (data.totalDistance < 0.0f) return false;
    if (MAX_VALID_KM < data.totalDistance) return false;
    return true;
  }
};
