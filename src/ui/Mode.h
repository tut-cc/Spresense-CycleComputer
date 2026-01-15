#pragma once

class Mode {
public:
  enum class ID { SPD_TIME, AVG_ODO, MAX_CLOCK, Count };

private:
  ID currentID = ID::SPD_TIME;

public:
  void next() {
    const int count = static_cast<int>(ID::Count);
    currentID       = static_cast<ID>((static_cast<int>(currentID) + 1) % count);
  }

  ID get() const {
    return currentID;
  }
};
