#pragma once

// Fix for libc++ availability macros error on Linux
#define _LIBCPP_HAS_NO_VENDOR_AVAILABILITY_ANNOTATIONS

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <iostream>
#include <algorithm>

// Mock basic types
using std::max;
using std::min;
using std::abs;

// dtostrf mock
inline char *dtostrf(double val, signed char width, unsigned char prec, char *s) {
    char fmt[20];
    sprintf(fmt, "%%%d.%df", width, prec);
    sprintf(s, fmt, val);
    return s;
}

// Time mocks
extern unsigned long _mock_millis;
inline unsigned long millis() { return _mock_millis; }
inline void delay(unsigned long ms) { _mock_millis += ms; }

// Pin mocks

// Pin mocks
#define PIN_D00 0
#define PIN_D01 1
#define PIN_D02 2
#define PIN_D03 3
#define PIN_D04 4
#define PIN_D05 5
#define PIN_D06 6
#define PIN_D07 7
#define PIN_D08 8
#define PIN_D09 9
#define PIN_D10 10
#define PIN_D11 11
#define PIN_D12 12
#define PIN_D13 13
#define PIN_D14 14
#define PIN_D15 15

// Pin Modes
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Pin States
#define LOW 0
#define HIGH 1

#include <map>

// GPIO State Map (Pin -> State)
extern std::map<int, int> _mock_pin_states;

inline void pinMode(int pin, int mode) {
    (void)pin;
    (void)mode;
    // No-op for now
}

inline int digitalRead(int pin) {
    if (_mock_pin_states.find(pin) != _mock_pin_states.end()) {
        return _mock_pin_states[pin];
    }
    return HIGH; // Default to HIGH (pullup behavior)
}

inline void digitalWrite(int pin, int val) {
    _mock_pin_states[pin] = val;
}

// Helper to set pin state for tests
inline void setPinState(int pin, int state) {
    _mock_pin_states[pin] = state;
}


// Serial Mock
class SerialMock {
public:
    void begin(int baud) {}
    void print(const char* s) { std::cout << s; }
    void print(int n) { std::cout << n; }
    void print(double n) { std::cout << n; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(int n) { std::cout << n << std::endl; }
    void println(float n) { std::cout << n << std::endl; }
    void println() { std::cout << std::endl; }
};

extern SerialMock Serial;
