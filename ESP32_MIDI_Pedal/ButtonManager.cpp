/*
 * Button Manager Module Implementation
 */

#include "ButtonManager.h"

// Button implementation
Button::Button() {
    pin = 0;
    lastState = HIGH;
    currentState = HIGH;
    lastDebounceTime = 0;
    pressStartTime = 0;
    longPressTriggered = false;
}

void Button::init(uint8_t buttonPin) {
    pin = buttonPin;
    pinMode(pin, INPUT_PULLUP);
    lastState = digitalRead(pin);
    currentState = lastState;
}

ButtonEvent Button::update() {
    ButtonEvent event;
    event.type = BUTTON_NONE;
    event.buttonNumber = 0;
    event.timestamp = millis();
    
    bool reading = digitalRead(pin);
    
    // Check if button state has changed
    if (reading != lastState) {
        lastDebounceTime = millis();
    }
    
    // Debounce check
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        // If the button state has changed
        if (reading != currentState) {
            currentState = reading;
            
            if (currentState == LOW) {
                // Button pressed
                pressStartTime = millis();
                longPressTriggered = false;
                event.type = BUTTON_PRESS;
            } else {
                // Button released
                if (!longPressTriggered) {
                    event.type = BUTTON_RELEASE;
                }
                pressStartTime = 0;
            }
        }
        
        // Check for long press
        if (currentState == LOW && !longPressTriggered && pressStartTime > 0) {
            if (millis() - pressStartTime >= LONG_PRESS_TIME) {
                longPressTriggered = true;
                event.type = BUTTON_LONG_PRESS;
            }
        }
    }
    
    lastState = reading;
    return event;
}

bool Button::isPressed() {
    return currentState == LOW;
}

bool Button::isLongPressed() {
    return isPressed() && (millis() - pressStartTime >= LONG_PRESS_TIME);
}

// ButtonManager implementation
ButtonManager::ButtonManager() {
}

void ButtonManager::begin() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttons[i].init(buttonPins[i]);
    }
    Serial.println("Button Manager initialized");
}

ButtonEvent ButtonManager::update() {
    ButtonEvent event;
    event.type = BUTTON_NONE;
    event.buttonNumber = 0;
    
    // Check each button for events
    for (int i = 0; i < BUTTON_COUNT; i++) {
        ButtonEvent buttonEvent = buttons[i].update();
        if (buttonEvent.type != BUTTON_NONE) {
            event = buttonEvent;
            event.buttonNumber = i + 1;  // Button numbers are 1-based
            break;  // Return first event found
        }
    }
    
    return event;
}

bool ButtonManager::isPairingCombo() {
    // Button 1 and Button 2 pressed simultaneously
    return buttons[0].isPressed() && buttons[1].isPressed();
}

bool ButtonManager::isFactoryResetCombo() {
    // Button 5 and Button 6 long pressed simultaneously
    return buttons[4].isLongPressed() && buttons[5].isLongPressed();
}

bool ButtonManager::isChannelUpCombo() {
    // Button 5 long press (when not combined with button 6)
    return buttons[4].isLongPressed() && !buttons[5].isPressed();
}

bool ButtonManager::isChannelDownCombo() {
    // Button 6 long press (when not combined with button 5)
    return buttons[5].isLongPressed() && !buttons[4].isPressed();
}