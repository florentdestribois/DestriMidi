/*
 * ESP32 Bluetooth MIDI Pedal Controller
 * 
 * Features:
 * - Bluetooth MIDI communication
 * - 4 MIDI control buttons
 * - 16 MIDI channels selection
 * - Battery monitoring with charging indication
 * - LCD display for status
 * - Auto-reconnect to paired devices
 * - Sleep mode for power saving
 * 
 * Author: DestriMidi Project
 * Version: 1.0.0
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// Module includes
#include "config.h"
#include "MidiHandler.h"
#include "DisplayManager.h"
#include "ButtonManager.h"
#include "BatteryManager.h"
#include "ConfigManager.h"

// Global objects
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
Preferences preferences;
MidiHandler midiHandler;
DisplayManager displayManager(&lcd);
ButtonManager buttonManager;
BatteryManager batteryManager;
ConfigManager configManager(&preferences);

// BLE objects
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// System state
SystemState systemState = {
    .midiChannel = 1,
    .isConnected = false,
    .batteryLevel = 100,
    .isCharging = false,
    .isPairingMode = false,
    .lastActivity = 0
};

// BLE Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        systemState.isConnected = true;
        digitalWrite(PIN_LED_BLUETOOTH, HIGH);
        Serial.println("BLE Client Connected");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        systemState.isConnected = false;
        digitalWrite(PIN_LED_BLUETOOTH, LOW);
        Serial.println("BLE Client Disconnected");
        // Start advertising again
        delay(500);
        pServer->startAdvertising();
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 MIDI Pedal Starting...");
    
    // Initialize I2C for LCD
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    
    // Initialize components
    initializePins();
    
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    displayManager.showBootScreen();
    
    // Load configuration
    configManager.begin();
    systemState.midiChannel = configManager.getMidiChannel();
    
    // Initialize battery manager
    batteryManager.begin();
    
    // Initialize button manager
    buttonManager.begin();
    
    // Initialize BLE MIDI
    initializeBLE();
    
    // Initialize MIDI handler with BLE characteristic
    midiHandler.begin(pCharacteristic);
    
    // Update display
    displayManager.updateDisplay(systemState);
    
    Serial.println("Setup complete!");
}

void loop() {
    // Update battery status
    batteryManager.update();
    systemState.batteryLevel = batteryManager.getBatteryPercentage();
    systemState.isCharging = batteryManager.isCharging();
    
    // Process button events
    ButtonEvent event = buttonManager.update();
    handleButtonEvent(event);
    
    // Handle BLE connection changes
    if (deviceConnected != oldDeviceConnected) {
        if (deviceConnected) {
            Serial.println("Device connected - ready to send MIDI");
        } else {
            Serial.println("Device disconnected");
        }
        oldDeviceConnected = deviceConnected;
        displayManager.updateDisplay(systemState);
    }
    
    // Update display periodically
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 1000) {
        lastDisplayUpdate = millis();
        displayManager.updateDisplay(systemState);
    }
    
    // Check for inactivity and enter sleep mode
    checkSleepMode();
    
    // Small delay to prevent watchdog issues
    delay(10);
}

void initializePins() {
    // LED pins
    pinMode(PIN_LED_POWER, OUTPUT);
    pinMode(PIN_LED_BLUETOOTH, OUTPUT);
    pinMode(PIN_LED_CHARGING, OUTPUT);
    
    // Turn on power LED
    digitalWrite(PIN_LED_POWER, HIGH);
    digitalWrite(PIN_LED_BLUETOOTH, LOW);
    digitalWrite(PIN_LED_CHARGING, LOW);
    
    // Battery monitoring
    pinMode(PIN_BATTERY_VOLTAGE, INPUT);
    pinMode(PIN_CHARGING_STATUS, INPUT_PULLUP);
}

void initializeBLE() {
    // Create BLE Device
    BLEDevice::init(DEVICE_NAME);
    
    // Create BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // Create BLE Service
    BLEService *pService = pServer->createService(BLEUUID(MIDI_SERVICE_UUID));
    
    // Create BLE Characteristic for MIDI
    pCharacteristic = pService->createCharacteristic(
        BLEUUID(MIDI_CHARACTERISTIC_UUID),
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_WRITE_NR
    );
    
    // Add descriptor
    pCharacteristic->addDescriptor(new BLE2902());
    
    // Start the service
    pService->start();
    
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLEUUID(MIDI_SERVICE_UUID));
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE MIDI Service started, waiting for connections...");
}

void handleButtonEvent(ButtonEvent event) {
    if (event.type == BUTTON_NONE) return;
    
    // Update last activity time
    systemState.lastActivity = millis();
    
    // Handle pairing mode (Button 1 + Button 2)
    if (buttonManager.isPairingCombo()) {
        enterPairingMode();
        return;
    }
    
    // Handle factory reset (Button 5 + Button 6 long press)
    if (buttonManager.isFactoryResetCombo()) {
        performFactoryReset();
        return;
    }
    
    // Handle button events based on button number
    switch (event.buttonNumber) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            // All buttons send MIDI CC on normal press
            if (event.type == BUTTON_PRESS && deviceConnected) {
                uint8_t ccNumber = event.buttonNumber; // CC#1-6
                midiHandler.sendControlChange(systemState.midiChannel, ccNumber, 127);
                displayManager.showMidiSent(ccNumber, systemState.midiChannel);
            }
            // Buttons 5 and 6 also handle channel change on long press
            else if (event.type == BUTTON_LONG_PRESS) {
                if (event.buttonNumber == 5) {
                    // Channel Up
                    systemState.midiChannel++;
                    if (systemState.midiChannel > 16) systemState.midiChannel = 1;
                    configManager.setMidiChannel(systemState.midiChannel);
                    displayManager.showChannelChange(systemState.midiChannel);
                } else if (event.buttonNumber == 6) {
                    // Channel Down
                    systemState.midiChannel--;
                    if (systemState.midiChannel < 1) systemState.midiChannel = 16;
                    configManager.setMidiChannel(systemState.midiChannel);
                    displayManager.showChannelChange(systemState.midiChannel);
                }
            }
            break;
    }
}

void enterPairingMode() {
    Serial.println("Entering pairing mode...");
    systemState.isPairingMode = true;
    
    // Clear any existing bonds
    // Note: ESP32 doesn't have a simple way to clear bonds in Arduino
    // Restart advertising with a different name temporarily
    
    displayManager.showPairingMode();
    
    // Blink Bluetooth LED
    for (int i = 0; i < 10; i++) {
        digitalWrite(PIN_LED_BLUETOOTH, HIGH);
        delay(200);
        digitalWrite(PIN_LED_BLUETOOTH, LOW);
        delay(200);
    }
    
    systemState.isPairingMode = false;
    displayManager.updateDisplay(systemState);
}

void performFactoryReset() {
    Serial.println("Performing factory reset...");
    
    displayManager.showFactoryReset();
    
    // Reset all settings
    configManager.factoryReset();
    systemState.midiChannel = 1;
    
    // Visual feedback
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_LED_POWER, LOW);
        digitalWrite(PIN_LED_BLUETOOTH, LOW);
        digitalWrite(PIN_LED_CHARGING, LOW);
        delay(200);
        digitalWrite(PIN_LED_POWER, HIGH);
        digitalWrite(PIN_LED_BLUETOOTH, HIGH);
        digitalWrite(PIN_LED_CHARGING, HIGH);
        delay(200);
    }
    
    digitalWrite(PIN_LED_POWER, HIGH);
    digitalWrite(PIN_LED_BLUETOOTH, LOW);
    digitalWrite(PIN_LED_CHARGING, LOW);
    
    // Restart
    delay(1000);
    ESP.restart();
}

void checkSleepMode() {
    // Enter sleep mode after 5 minutes of inactivity
    if (millis() - systemState.lastActivity > SLEEP_TIMEOUT) {
        if (!systemState.isCharging) {
            enterSleepMode();
        }
    }
}

void enterSleepMode() {
    Serial.println("Entering sleep mode...");
    
    // Show sleep message
    displayManager.showSleepMode();
    
    // Turn off all LEDs
    digitalWrite(PIN_LED_POWER, LOW);
    digitalWrite(PIN_LED_BLUETOOTH, LOW);
    digitalWrite(PIN_LED_CHARGING, LOW);
    
    // Turn off LCD backlight
    lcd.noBacklight();
    
    // Configure wake up sources (any button press)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, 0); // Wake on Button 1 press
    
    // Enter deep sleep
    esp_deep_sleep_start();
}