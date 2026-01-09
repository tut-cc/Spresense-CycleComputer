# Spresense Cycle Computer

Sony Spresense を使用した、サイクルコンピュータのプロジェクト。

## ハードウェア構成

- **マイコンボード**: Sony Spresense + 拡張ボード
- **ディスプレイ**: LCD または 7 セグメント LED
  - `src/Config.h` で切り替え可能
- **入力**: タクトスイッチ x 2
- **電源**: 発電機

## ソフトウェア構成

本プロジェクトは Arduino IDE / Arduino CLI で開発されている。
ソースコードは `src` ディレクトリ配下に機能ごとに整理されている。

### ディレクトリ構成

```
Spresense-CycleComputer.ino                  # Arduinoスケッチのエントリーポイント
docs/                    # ドキュメント (要件定義書など)
src/
 ├── Config.h             # ディスプレイタイプ等の全体設定
 ├── interfaces/          # インターフェース定義 (IDisplay等)
 ├── drivers/             # ハードウェア制御 (LCD, Button, GPS)
 └── system/              # アプリケーションロジック (CycleComputer, InputManager等)
```

## ビルドと書き込み方法

### 前提条件

以下のツールが必要。

- [Arduino CLI](https://arduino.github.io/arduino-cli/latest/)
- CMake (3.14以上)
- Ninja または Make (ビルドツール)

### 必要なコアのインストール

ビルドには Arduino Core のインストールが必要。以下のコマンドでインストール。

```bash
# Spresense 用のパッケージURLを追加
arduino-cli config init
arduino-cli config add board_manager.additional_urls https://github.com/sonydevworld/spresense-arduino-compatible/releases/download/generic/package_spresense_index.json

# インデックスの更新
arduino-cli core update-index

# Spresense と Arduino AVR コアのインストール
arduino-cli core install SPRESENSE:spresense
```

### 必要なライブラリのインストール

以下のコマンドで必要なライブラリをインストール。

```bash
# OLEDディスプレイ用ライブラリ
arduino-cli lib install "Adafruit SSD1306" "Adafruit GFX Library"
```

### ビルド手順

プロジェクトのルートで以下のコマンドを実行。

```bash
# ビルド設定 (初回のみ)
cmake -S . -B build

# Spresense 向けビルド
cmake --build build
```

### 書き込み手順

PC とデバイスを接続し、ポートを指定して実行。

```bash
# Spresense への書き込み
cmake --build build --target upload_spresense
```

※ ポートはデフォルトで `/dev/ttyACM0` ですが、変更する場合は以下のように設定します：
```bash
cmake -S . -B build -DPORT=/dev/ttyUSB0
cmake --build build --target upload_spresense
```

Arduino IDE を使って `Spresense-CycleComputer.ino` を開き、ビルド・書き込みを行うことも可能。

### ユニットテスト

本プロジェクトには GoogleTest を使用したホスト環境でのユニットテストが含まれている。

```bash
# テストのビルド
cmake --build build --target run_tests

# テストの実行
./build/tests/host/run_tests
```
