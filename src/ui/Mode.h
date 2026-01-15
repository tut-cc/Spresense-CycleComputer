#pragma once

class Mode {
public:
  enum class ID { SPEED, MAX_SPEED, AVG_SPEED, TIME, MOVING_TIME, ELAPSED_TIME, DISTANCE, Count };

private:
  ID currentID = ID::SPEED;

public:
  void next() {
    const int count = static_cast<int>(ID::Count);
    currentID       = static_cast<ID>((static_cast<int>(currentID) + 1) % count);
  }

  ID get() const {
    return currentID;
  }
};
