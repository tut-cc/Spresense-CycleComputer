#include "Arduino.h"

unsigned long      _mock_millis = 0;
std::map<int, int> _mock_pin_states;
SerialMock         Serial;
