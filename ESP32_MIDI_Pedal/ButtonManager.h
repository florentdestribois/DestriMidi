/*
 * Button Manager Module
 * Handles button debouncing, long press detection, and button combinations
 */

#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "config.h"

class Button {
private:
    uint8_t pin;
    bool lastState;
    bool currentState;
    unsigned long lastDebounceTime;
    unsigned long pressStartTime;
    bool longPressTriggered;
    
public:
    Button();
    void init(uint8_t buttonPin);
    ButtonEvent update();
    bool isPressed();
    bool isLongPressed();
};

class ButtonManager {
private:
    Button buttons[BUTTON_COUNT];
    uint8_t buttonPins[BUTTON_COUNT] = {
        PIN_BUTTON_1, 
        PIN_BUTTON_2, 
        PIN_BUTTON_3, 
        PIN_BUTTON_4, 
        PIN_BUTTON_5, 
        PIN_BUTTON_6
    };
    
public:
    ButtonManager();
    void begin();
    ButtonEvent update();
    bool isPairingCombo();
    bool isFactoryResetCombo();
    bool isChannelUpCombo();
    bool isChannelDownCombo();
};

#endif