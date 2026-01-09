#include "SevenSegDriver.h"

// Static member definitions
const int SevenSegDriver::placePin[] = {D1, D2, D3,
                                        D4};
const int SevenSegDriver::colonPin[] = {D1_COLON, D2_COLON,
                                        D3_COLON, D4_COLON,
                                        DOT_COLON};

// 7-segment data from Config
const unsigned char* SevenSegDriver::number = NUMBERS;
const unsigned char* SevenSegDriver::numdot = NUMBERS_WITH_DOT;

void SevenSegDriver::begin() {
    pinMode(SDI, OUTPUT);
    pinMode(RCLK, OUTPUT);
    pinMode(SRCLK, OUTPUT);

    // Initialize Digit Pins
    for (int i = 0; i < 4; i++) {
        pinMode(placePin[i], OUTPUT);
        digitalWrite(placePin[i], LOW);  // Active High -> LOW = OFF
    }

    // Initialize Colon/Dot Pins
    for (int i = 0; i < 5; i++) {
        pinMode(colonPin[i], OUTPUT);
        digitalWrite(colonPin[i], LOW);
    }
    
    clear();
    lastMultiplexTime = micros();
}

void SevenSegDriver::clear() {
    for (int i = 0; i < 4; i++) digitBuffer[i] = 0xFF; 
}

void SevenSegDriver::show(DisplayDataType type, const char* value) {
    // Parse string to number
    float f_val = atof(value);
    int molding = (int)(f_val * 100); // 12.34 -> 1234

    int temp = molding;
    
    // Extract digits (works for both >= 10.00 and < 10.00)
    int d1 = (temp / 1000) % 10;
    int d2 = (temp / 100) % 10;
    int d3 = (temp / 10) % 10;
    int d4 = temp % 10;

    digitBuffer[0] = number[d1];
    digitBuffer[1] = numdot[d2]; 
    digitBuffer[2] = number[d3];
    digitBuffer[3] = number[d4];
}

void SevenSegDriver::update() {
    unsigned long currentMicros = micros();
    if (currentMicros - lastMultiplexTime >= MULTIPLEX_INTERVAL_US) return;
    
    lastMultiplexTime = currentMicros;

    digitalWrite(placePin[currentDigit], LOW);  // Turn off current digit
    currentDigit = (currentDigit + 1) % 4;      // Move to next digit

    // Prepare new data
    byte data = digitBuffer[currentDigit];
    hc595_shift(data);

    // Turn on new digit
    digitalWrite(placePin[currentDigit], HIGH);
    
    // Update colons
    for (int i = 0; i < 5; i++) digitalWrite(colonPin[i], LOW);
    if (currentDigit < 4)       digitalWrite(colonPin[currentDigit], HIGH);
}

void SevenSegDriver::hc595_shift(byte data) {
    digitalWrite(RCLK, LOW);
    shiftOut(SDI, SRCLK, MSBFIRST, data);
    digitalWrite(RCLK, HIGH);
}
