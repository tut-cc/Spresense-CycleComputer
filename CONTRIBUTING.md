# 開発ガイドライン (Contributing Guidelines)

コードの一貫性と品質を保つため、以下のルールに従ってください。

## 1. 命名規則 (Naming Conventions)

| 項目                | 規則             | 例                                |
| :------------------ | :--------------- | :-------------------------------- |
| **変数・関数名**    | camelCase        | `readSensorData()`, `sensorValue` |
| **クラス名**        | PascalCase       | `DisplayManager`                  |
| **定数**            | UPPER_SNAKE_CASE | `LED_PIN`, `MAX_BUFFER_SIZE`      |
| **構造体 (struct)** | PascalCase       | `SensorData`                      |
| **列挙型 (enum)**   | PascalCase       | `SystemMode`                      |
| **列挙型の要素**    | UPPER_SNAKE_CASE | `MODE_IDLE`                       |

## 2. 基本スタイル (Basic Style)

`clang-format` を使用します。設定ファイル (`.clang-format`) がリポジトリに含まれています。
コミット前にフォーマットを適用することを推奨します。

## 3. ハードウェア定義 (Hardware Definitions)

- **ピン定義**: `#define` マクロの使用は避け、`constexpr` または `enum` を使用してください。
  ```cpp
  // OK
  constexpr uint8_t LED_PIN = 13;
  // または
  enum Pin : uint8_t { LED_RED = 13 };
  ```
- **マジックナンバーの禁止**: ピン番号や設定値を `digitalWrite(13, HIGH)` のように直接数値で書かないでください。必ず名前付き定数を使用してください。

## 4. 設計方針 (Design Guidelines)

- **非ブロッキング処理**: `delay()` の使用は最小限に留め、可能な限り `millis()` を使用した非ブロッキング処理を実装してください。
- **ライブラリ** : 使用するライブラリのバージョン依存がある場合は、コメント等でバージョンを明記してください。
