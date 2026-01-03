/*
 * ファイル: LCDDriver.h
 * 説明: 標準的な 16x2 テキスト LCD 用のディスプレイドライバー。
 *       サイクルコンピューターデータを表示するための IDisplay
 * インターフェースを実装します。
 */

#pragma once

#include <LiquidCrystal.h>

#include "../interfaces/IDisplay.h"

class LCDDriver : public IDisplay {
   private:
    LiquidCrystal lcd;

   public:
    LCDDriver();

    void begin() override;

    void clear() override;

    void show(DisplayDataType type, const char* value) override;
};
