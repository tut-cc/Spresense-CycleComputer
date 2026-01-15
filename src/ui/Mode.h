#pragma once

class Mode {
public:
  enum class ID { SPD_TIME, AVG_ODO, MAX_CLOCK, Count };

private:
  ID currentID = ID::SPD_TIME;

public:
  void next() {
    auto nextVal = static_cast<int>(currentID) + 1;
    if (nextVal >= static_cast<int>(ID::Count)) nextVal = 0;
    currentID = static_cast<ID>(nextVal);
  }

  ID get() const {
    return currentID;
  }
};
