/*
 * ESP32 Bluetooth MIDI Pedal Controller
 * 
 * Features:
 * - Bluetooth MIDI communication
 * - 4 MIDI control buttons
 * - 16 MIDI channels selection
 * - Battery monitoring with charging indication
 * - I2C LCD display with custom battery/bluetooth icons
 * - Deep sleep power management
 * 
 * Hardware:
 * - ESP32 (Upesy WROOM)
 * - I2C LCD 16x2
 * - 6 tactile buttons
 * - Battery voltage divider on GPIO34
 * - Charging status from TP4056 on GPIO35
 * - Power/Status LEDs
 * 
 * Author: Florent DESTRIBOIS with Claude Code assistance
 * Version: 2.0.0
 */

#include "config.h"
#include "MidiHandler.h"
#include "ButtonManager.h"
#include "DisplayManager.h"
#include "BatteryManager.h"
#include "ConfigManager.h"

// System state structure
SystemState systemState = {
    .isConnected = false,
    .currentChannel = 1,
    .batteryLevel = 0,
    .isCharging = false,
    .lastActivityTime = 0,
    .isInDeepSleep = false
};

// Global objects
MidiHandler midiHandler;
ButtonManager buttonManager;
DisplayManager displayManager;
BatteryManager batteryManager;
ConfigManager configManager;

// BLE connection state
volatile bool deviceConnected = false;

// BLE Server Callbacks
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
        systemState.isConnected = true;
        digitalWrite(PIN_LED_BLUETOOTH, HIGH);
        Serial.println("BLE Client Connected");
    }

    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
        systemState.isConnected = false;
        digitalWrite(PIN_LED_BLUETOOTH, LOW);
        Serial.println("BLE Client Disconnected");
        // Start advertising again
        delay(500);
        pServer->startAdvertising();
    }
};

/**
 * Initialize GPIO pins for LEDs, buttons, and other peripherals
 */
void initializePins() {
    // Initialize LED pins
    pinMode(PIN_LED_POWER, OUTPUT);
    pinMode(PIN_LED_BLUETOOTH, OUTPUT);
    pinMode(PIN_LED_CHARGING, OUTPUT);
    pinMode(PIN_LED_ACTIVITY, OUTPUT);
    
    // Set initial LED states
    digitalWrite(PIN_LED_POWER, HIGH);        // Power on
    digitalWrite(PIN_LED_BLUETOOTH, LOW);     // BLE off initially
    digitalWrite(PIN_LED_CHARGING, LOW);      // Charging indicator
    digitalWrite(PIN_LED_ACTIVITY, LOW);      // Activity LED
    
    // Initialize analog input pins
    pinMode(PIN_BATTERY_VOLTAGE, INPUT);
    pinMode(PIN_CHARGING_STATUS, INPUT_PULLUP);
    
    Serial.println("GPIO pins initialized");
}

/**
 * Handle MIDI messages to be sent
 */
void handleMidiMessage(uint8_t channel, uint8_t control, uint8_t value) {
    if (systemState.isConnected) {
        midiHandler.sendControlChange(channel, control, value);
        digitalWrite(PIN_LED_ACTIVITY, HIGH);
        delay(ACTIVITY_LED_DURATION);
        digitalWrite(PIN_LED_ACTIVITY, LOW);
        Serial.printf("MIDI CC: Ch%d CC%d Val%d\n", channel + 1, control, value);
    }
}

/**
 * Handle button events from ButtonManager
 */
void handleButtonEvent(ButtonEvent event) {
    systemState.lastActivityTime = millis();
    
    switch (event.type) {
        case BUTTON_PRESS:
            if (event.buttonIndex < 6) {
                // Send MIDI control change for button press
                uint8_t ccValue = 127; // Full velocity
                handleMidiMessage(systemState.currentChannel - 1, event.buttonIndex + 1, ccValue);
                displayManager.showActivity();
            }
            break;
            
        case BUTTON_LONG_PRESS:
            if (event.buttonIndex == 4) { // Button 5 - Channel up
                systemState.currentChannel = (systemState.currentChannel % 16) + 1;
                configManager.saveChannel(systemState.currentChannel);
                displayManager.updateChannel(systemState.currentChannel);
                Serial.printf("Channel up: %d\n", systemState.currentChannel);
            } else if (event.buttonIndex == 5) { // Button 6 - Channel down
                systemState.currentChannel = (systemState.currentChannel == 1) ? 16 : systemState.currentChannel - 1;
                configManager.saveChannel(systemState.currentChannel);
                displayManager.updateChannel(systemState.currentChannel);
                Serial.printf("Channel down: %d\n", systemState.currentChannel);
            }
            break;
            
        case BUTTON_COMBO_PAIRING:
            Serial.println("Entering pairing mode...");
            displayManager.showPairingMode();
            midiHandler.startAdvertising();
            break;
            
        case BUTTON_COMBO_BATTERY:
            Serial.println("Showing battery level...");
            float voltage = batteryManager.getVoltage();
            uint8_t percentage = batteryManager.getPercentage();
            bool charging = batteryManager.isCharging();
            displayManager.showBattery(percentage, charging);
            Serial.printf("Battery: %.2fV (%d%%) %s\n", voltage, percentage, charging ? "Charging" : "");
            break;
            
        case BUTTON_COMBO_FACTORY_RESET:
            Serial.println("Factory reset requested...");
            configManager.factoryReset();
            ESP.restart();
            break;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 MIDI Pedal Starting...");
    
    // Initialize I2C for LCD
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    
    // Initialize components
    initializePins();
    
    // Initialize managers in order
    configManager.begin();
    systemState.currentChannel = configManager.loadChannel();
    
    displayManager.begin();
    displayManager.updateChannel(systemState.currentChannel);
    
    batteryManager.begin();
    buttonManager.begin();
    
    // Initialize MIDI handler with callback
    midiHandler.begin("DestriMidi", new ServerCallbacks());
    
    Serial.println("System initialized successfully");
    Serial.printf("Current MIDI channel: %d\n", systemState.currentChannel);
    
    systemState.lastActivityTime = millis();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Update button states and handle events
    ButtonEvent event = buttonManager.update();
    if (event.type != BUTTON_NONE) {
        handleButtonEvent(event);
    }
    
    // Update battery status periodically
    if (currentTime - batteryManager.getLastReadTime() > BATTERY_READ_INTERVAL) {
        batteryManager.update();
        systemState.batteryLevel = batteryManager.getPercentage();
        systemState.isCharging = batteryManager.isCharging();
        
        // Update charging LED
        digitalWrite(PIN_LED_CHARGING, systemState.isCharging ? HIGH : LOW);
    }
    
    // Update display
    displayManager.update();
    
    // Handle BLE connection changes
    bool currentConnectionState = midiHandler.isConnected();
    if (currentConnectionState != systemState.isConnected) {
        systemState.isConnected = currentConnectionState;
        digitalWrite(PIN_LED_BLUETOOTH, systemState.isConnected ? HIGH : LOW);
        
        if (systemState.isConnected) {
            Serial.println("BLE Connected");
            displayManager.showConnected();
        } else {
            Serial.println("BLE Disconnected");
            displayManager.showDisconnected();
        }
    }
    
    // Check for sleep timeout
    if (currentTime - systemState.lastActivityTime > SLEEP_TIMEOUT) {
        Serial.println("Entering deep sleep...");
        
        // Show sleep message
        displayManager.showSleep();
        delay(1000);
        
        // Turn off all LEDs except power
        digitalWrite(PIN_LED_BLUETOOTH, LOW);
        digitalWrite(PIN_LED_CHARGING, LOW);
        digitalWrite(PIN_LED_ACTIVITY, LOW);
        
        // Turn off LCD backlight
        displayManager.turnOff();
        
        // Configure wake up sources (any button press)
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, 0); // Wake on Button 1 press
        
        // Enter deep sleep
        esp_deep_sleep_start();
    }
    
    delay(10); // Small delay for stability
}