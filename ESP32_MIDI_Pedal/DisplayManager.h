/*
 * Display Manager Module
 * Handles all LCD display operations
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

class DisplayManager {
private:
    LiquidCrystal_I2C* lcd;
    unsigned long messageTimer;
    bool hasTemporaryMessage;
    
    // Custom characters for battery and BT icons
    void createCustomCharacters();
    
public:
    DisplayManager(LiquidCrystal_I2C* display);
    void begin();
    void updateDisplay(SystemState& state);
    void showBootScreen();
    void showMidiSent(uint8_t ccNumber, uint8_t channel);
    void showChannelChange(uint8_t channel);
    void showPairingMode();
    void showFactoryReset();
    void showSleepMode();
    void showLowBattery();
    void clearTemporaryMessage();
};

#endif