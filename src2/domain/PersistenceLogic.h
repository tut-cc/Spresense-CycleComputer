#pragma once
/**
 * @file PersistenceLogic.h
 * @brief 永続化用データの生成ロジック
 *
 * TripStateからSaveData（EEPROM保存用）を生成します。
 */

#include "../common/DataStructures.h"

namespace PersistenceLogic {

/**
 * @brief 保存用データを生成
 * @param state 走行状態
 * @param voltage 現在のバッテリー電圧
 * @return SaveData 保存用データ
 *
 * CRCはDataStoreが設定するため、ここでは0を設定しています。
 */
inline SaveData create(const TripStateBase &state, float voltage) {
  SaveData data;
  data.magicNumber   = SAVE_DATA_MAGIC_NUMBER;
  data.totalDistance = state.distance.total;
  data.tripDistance  = state.distance.trip;
  data.movingTimeMs  = state.time.moving;
  data.maxSpeed      = state.speed.max;
  data.voltage       = voltage;
  data.updateStatus  = state.updateStatus;
  data.crc           = 0; // DataStoreで計算される
  return data;
}

} // namespace PersistenceLogic
