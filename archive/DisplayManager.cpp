/*
 * Display Manager Module Implementation
 */

#include "DisplayManager.h"

// Custom character definitions
uint8_t batteryFull[8] = {
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
};

uint8_t batteryHalf[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b11111,
    0b11111,
    0b11111,
    0b11111
};

uint8_t batteryLow[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11111,
    0b11111
};

uint8_t bluetoothIcon[8] = {
    0b00100,
    0b00110,
    0b10101,
    0b01110,
    0b01110,
    0b10101,
    0b00110,
    0b00100
};

uint8_t chargingIcon[8] = {
    0b00100,
    0b00100,
    0b01110,
    0b01110,
    0b11111,
    0b11111,
    0b01110,
    0b00100
};

DisplayManager::DisplayManager(LiquidCrystal_I2C* display) {
    lcd = display;
    messageTimer = 0;
    hasTemporaryMessage = false;
}

void DisplayManager::begin() {
    createCustomCharacters();
}

void DisplayManager::createCustomCharacters() {
    lcd->createChar(0, batteryFull);
    lcd->createChar(1, batteryHalf);
    lcd->createChar(2, batteryLow);
    lcd->createChar(3, bluetoothIcon);
    lcd->createChar(4, chargingIcon);
}

void DisplayManager::updateDisplay(SystemState& state) {
    // Check if temporary message should be cleared
    if (hasTemporaryMessage && millis() - messageTimer > 2000) {
        clearTemporaryMessage();
        hasTemporaryMessage = false;
    }
    
    if (!hasTemporaryMessage) {
        lcd->clear();
        
        // Line 1: Channel and Status
        lcd->setCursor(0, 0);
        lcd->print("CH:");
        if (state.midiChannel < 10) lcd->print("0");
        lcd->print(state.midiChannel);
        
        // Bluetooth status
        lcd->setCursor(7, 0);
        if (state.isConnected) {
            lcd->write(3);  // Bluetooth icon
            lcd->print(" ON ");
        } else {
            lcd->print("  OFF");
        }
        
        // Battery icon
        lcd->setCursor(14, 0);
        if (state.isCharging) {
            lcd->write(4);  // Charging icon
        } else if (state.batteryLevel > 60) {
            lcd->write(0);  // Full battery
        } else if (state.batteryLevel > 30) {
            lcd->write(1);  // Half battery
        } else {
            lcd->write(2);  // Low battery
        }
        
        // Battery percentage
        lcd->setCursor(15, 0);
        if (state.batteryLevel == 100) {
            lcd->print("F");
        } else {
            // Don't show percentage, just icon
        }
        
        // Line 2: Status or Ready message
        lcd->setCursor(0, 1);
        if (state.isPairingMode) {
            lcd->print("Pairing Mode... ");
        } else if (state.isConnected) {
            lcd->print("Ready - 6xCC    ");
        } else {
            lcd->print("Not Connected   ");
        }
    }
}

void DisplayManager::showBootScreen() {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("MIDI Pedal v1.0");
    lcd->setCursor(0, 1);
    lcd->print("Initializing... ");
    delay(2000);
}

void DisplayManager::showMidiSent(uint8_t ccNumber, uint8_t channel) {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("MIDI Sent:");
    lcd->setCursor(0, 1);
    lcd->print("CC#");
    lcd->print(ccNumber);
    lcd->print(" CH:");
    lcd->print(channel);
    lcd->print(" Val:127");
    
    hasTemporaryMessage = true;
    messageTimer = millis();
}

void DisplayManager::showChannelChange(uint8_t channel) {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("Channel Changed:");
    lcd->setCursor(0, 1);
    lcd->print("    CH: ");
    if (channel < 10) lcd->print("0");
    lcd->print(channel);
    lcd->print("    ");
    
    hasTemporaryMessage = true;
    messageTimer = millis();
}

void DisplayManager::showPairingMode() {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("  PAIRING MODE  ");
    lcd->setCursor(0, 1);
    lcd->print("Discoverable... ");
}

void DisplayManager::showFactoryReset() {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print(" FACTORY RESET  ");
    lcd->setCursor(0, 1);
    lcd->print("  Restarting... ");
}

void DisplayManager::showSleepMode() {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("  Sleep Mode    ");
    lcd->setCursor(0, 1);
    lcd->print("Press to wake   ");
}

void DisplayManager::showLowBattery() {
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("!LOW BATTERY!   ");
    lcd->setCursor(0, 1);
    lcd->print("Please Charge   ");
    
    hasTemporaryMessage = true;
    messageTimer = millis();
}

void DisplayManager::clearTemporaryMessage() {
    hasTemporaryMessage = false;
}