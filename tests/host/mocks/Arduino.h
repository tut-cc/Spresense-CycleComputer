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

// Time mocks
extern unsigned long _mock_millis;
inline unsigned long millis() { return _mock_millis; }
inline void delay(unsigned long ms) { _mock_millis += ms; }

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
