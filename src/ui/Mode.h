#pragma once


class Mode {
public:
  enum class ID {
    SPEED,
    MAX_SPEED,
    AVG_SPEED,
    TIME,
    MOVING_TIME,
    ELAPSED_TIME,
    DISTANCE,
  };

private:
  ID currentID;

public:
  Mode() : currentID(ID::SPEED) {}

  void next() {
    currentID = static_cast<ID>((static_cast<int>(currentID) + 1) % 7);
  }

  void prev() {
    currentID = static_cast<ID>((static_cast<int>(currentID) + 6) % 7);
  }

  ID get() const {
    return currentID;
  }
};

