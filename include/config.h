/*
 * Configuration file for ESP32 MIDI Pedal
 */

#ifndef CONFIG_H
#define CONFIG_H

// Device Information
#define DEVICE_NAME "ESP32 MIDI Pedal"
#define DEVICE_VERSION "1.0.0"

// Pin Definitions
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#define PIN_BATTERY_VOLTAGE 34
#define PIN_CHARGING_STATUS 35
#define PIN_BUTTON_1 32
#define PIN_BUTTON_2 33
#define PIN_BUTTON_3 25
#define PIN_BUTTON_4 26
#define PIN_BUTTON_5 27
#define PIN_BUTTON_6 14
#define PIN_LED_POWER 12
#define PIN_LED_BLUETOOTH 13
#define PIN_LED_CHARGING 15

// LCD Configuration
#define LCD_ADDRESS 0x27  // or 0x3F, check with I2C scanner

// Button Configuration
#define DEBOUNCE_DELAY 50
#define LONG_PRESS_TIME 1000
#define BUTTON_COUNT 6

// MIDI Configuration
#define DEFAULT_MIDI_CHANNEL 1
#define MIDI_CC_BUTTON_1 1
#define MIDI_CC_BUTTON_2 2
#define MIDI_CC_BUTTON_3 3
#define MIDI_CC_BUTTON_4 4
#define MIDI_CC_BUTTON_5 5
#define MIDI_CC_BUTTON_6 6

// BLE MIDI UUIDs
#define MIDI_SERVICE_UUID "03B80E5A-EDE8-4B33-A751-6CE34EC4C700"
#define MIDI_CHARACTERISTIC_UUID "7772E5DB-3868-4112-A1A9-F2669D106BF3"

// Battery Configuration
#define BATTERY_MIN_VOLTAGE 3.0
#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_LOW_THRESHOLD 20
#define BATTERY_CRITICAL_THRESHOLD 10
#define ADC_RESOLUTION 4095
#define ADC_REFERENCE_VOLTAGE 3.3

// Power Management
#define SLEEP_TIMEOUT 300000  // 5 minutes in milliseconds

// System State Structure
struct SystemState {
    uint8_t midiChannel;
    bool isConnected;
    uint8_t batteryLevel;
    bool isCharging;
    bool isPairingMode;
    unsigned long lastActivity;
};

// Button Event Types
enum ButtonEventType {
    BUTTON_NONE,
    BUTTON_PRESS,
    BUTTON_RELEASE,
    BUTTON_LONG_PRESS
};

// Button Event Structure
struct ButtonEvent {
    uint8_t buttonNumber;
    ButtonEventType type;
    unsigned long timestamp;
};

#endif