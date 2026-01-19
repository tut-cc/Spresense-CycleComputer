#include "Arduino.h"
#include "EEPROM.h"

unsigned long      _mock_millis = 0;
std::map<int, int> _mock_pin_states;
std::map<int, int> _mock_analog_values;
SerialMock         Serial;
EEPROMClass        EEPROM;
