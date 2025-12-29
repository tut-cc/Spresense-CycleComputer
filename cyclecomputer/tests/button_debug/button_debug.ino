/*
 * Button Debug Sketch
 * 
 * Simple tool to verify button hardware connections.
 * Prints the state of Pin 8 and Pin 9 to the Serial Monitor.
 */

#define BTN_A_PIN 8
#define BTN_B_PIN 9

void setup() {
    Serial.begin(115200);
    while (!Serial); // Wait for Serial to connect

    pinMode(BTN_A_PIN, INPUT_PULLUP);
    pinMode(BTN_B_PIN, INPUT_PULLUP);

    Serial.println("Button Debug Start");
    Serial.println("Pin 8 (Btn A) | Pin 9 (Btn B)");
    Serial.println("---------------------------");
}

void loop() {
    int valA = digitalRead(BTN_A_PIN);
    int valB = digitalRead(BTN_B_PIN);

    Serial.print("A: ");
    Serial.print(valA == LOW ? "PRESSED " : "OPEN    ");
    Serial.print("| B: ");
    Serial.println(valB == LOW ? "PRESSED " : "OPEN    ");

    delay(100);
}
