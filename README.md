# Spresense Cycle Computer

Sony Spresense を使用した、サイクルコンピュータのプロジェクト。

## ハードウェア構成

- **マイコンボード**: Sony Spresense + 拡張ボード
- **ディスプレイ**: OLEDディスプレイ
- **入力**: タクトスイッチ x 2
- **LED**: 赤色LED x 1
- **電源**: 発電機

## ビルドと書き込み方法

### 前提条件

以下のツールが必要。

- [Arduino CLI](https://arduino.github.io/arduino-cli/latest/) もしくはArduino IDE
- CMake (3.14以上)
- Make

### ビルド手順

プロジェクトのルートで以下のコマンドを実行。

```bash
cmake -S . -B build                     # ビルド設定
cmake --build build --target spresense  # ビルド
```

### 書き込み手順

PC とデバイスを接続して実行。

```bash
cmake --build build --target upload # Spresense への書き込み
```

※ ポートはデフォルトで `/dev/ttyUSB0` ですが、変更する場合は以下のように設定します。

```bash
cmake -S . -B build -DPORT=/dev/ttyACM0
cmake --build build --target upload
```

Arduino IDE を使って `Spresense-CycleComputer.ino` を開き、ビルド・書き込みを行うことも可能。

### テスト手順

本プロジェクトには GoogleTest を使用したPC上で動作する Unit Test が含まれている。

```bash
cmake --build build --target run_tests # テストのビルド
./build/tests/host/run_tests # テストの実行
```
