#pragma once

/**
 * @file DoubleBuffer.h
 * @brief ダブルバッファリング（Ping-Pong バッファ）の汎用実装
 *
 * 2つのバッファを交互に使用することで、前回の状態との比較を可能にします。
 * 主な用途:
 * - 状態変化の検出（UI更新のトリガー判定など）
 * - データの安全な更新（読み取り中の書き込み防止）
 */

template <typename T> class DoubleBuffer {
public:
  T   buffers[2];
  int idx = 0;

public:
  DoubleBuffer() = default;

  T       &current() { return buffers[idx]; }
  const T &current() const { return buffers[idx]; }
  const T &previous() const { return buffers[1 - idx]; }
  bool     hasChanged() const { return current() != previous(); }
  void     copyFromPrevious() { buffers[idx] = buffers[1 - idx]; }
  T       &operator[](int i) { return buffers[i]; }
  const T &operator[](int i) const { return buffers[i]; }
  void     swap() { idx = 1 - idx; }

  void initialize(const T &value) {
    buffers[0] = value;
    buffers[1] = value;
  }

  bool apply(const T &next) {
    swap();
    current() = next;
    return hasChanged();
  }

  void prepare() {
    swap();
    copyFromPrevious();
  }
};
