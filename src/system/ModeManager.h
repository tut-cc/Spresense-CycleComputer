/*
 * ファイル: ModeManager.h
 * 説明: ディスプレイモード（速度、時間、距離など）を管理する。
 */

#pragma once

enum Mode {
    MODE_SPEED,
    MODE_TIME,
    MODE_MAX_SPEED,
    MODE_DISTANCE,
    MODE_MOVING_TIME,
    MODE_ELAPSED_TIME,
    MODE_AVG_SPEED,
    TOTAL_MODES
};

class ModeManager {
   private:
    Mode currentMode = MODE_SPEED;

   public:
    void nextMode();

    Mode getMode();
};
