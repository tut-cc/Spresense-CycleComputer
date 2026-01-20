#pragma once

template <typename T> class DoubleBuffer {
public:
  T   buffers[2];
  int idx = 0;

public:
  DoubleBuffer() = default;

  T       &current() { return buffers[idx]; }
  const T &current() const { return buffers[idx]; }
  const T &previous() const { return buffers[1 - idx]; }

  void swap() { idx = 1 - idx; }

  void initialize(const T &value) {
    buffers[0] = value;
    buffers[1] = value;
  }

  /**
   * 新しい値を適用し、以前の値から変更があったかどうかを返します。
   * (出力抑制型バッファ用)
   */
  bool apply(const T &next) {
    swap();
    current() = next;
    return hasChanged();
  }

  /**
   * 前回の値をコピーして次の更新の準備をします。
   * (状態累積型バッファ用)
   */
  void prepare() {
    swap();
    copyFromPrevious();
  }

  bool hasChanged() const { return current() != previous(); }

  void copyFromPrevious() { buffers[idx] = buffers[1 - idx]; }

  T       &operator[](int i) { return buffers[i]; }
  const T &operator[](int i) const { return buffers[i]; }
};
