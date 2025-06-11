#include <Arduino.h>



struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
    unsigned long pressStartTime;
};

Button button1 = {16, 0, false, 0}; // Taster Pin definieren


unsigned long last_button_time = 0;
uint8_t pressCount = 0;
unsigned long firstPressTime = 0;
const unsigned long multiPressTimeout = 800;
bool checkTimeout = false;


void IRAM_ATTR isr() {
    unsigned long current_time = millis();

    if (current_time - last_button_time > 50) { // Debounce-Zeit
        last_button_time = current_time;

        if (digitalRead(button1.PIN) == LOW) {
            if (!button1.pressed) {
                button1.pressed = true;
                button1.pressStartTime = current_time;
            }
        } else {
            if (button1.pressed) {
                button1.pressed = false;
                button1.numberKeyPresses++;

                if (pressCount == 0) {
                    firstPressTime = current_time;
                    checkTimeout = true;
                }
                pressCount++;
            }
        }
    }
}


void Taster_initialisieren () {
    pinMode(button1.PIN, INPUT_PULLUP);
    attachInterrupt(button1.PIN, isr, CHANGE);
}
