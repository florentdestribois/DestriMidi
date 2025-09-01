# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-based Bluetooth MIDI pedal controller project using Arduino framework. The system sends MIDI Control Change (CC) messages over Bluetooth Low Energy (BLE) and includes battery management, LCD display, and configurable MIDI channels.

## Development Environment

### Arduino IDE Configuration
**Board**: ESP32 Dev Module
- Upload Speed: 921600
- CPU Frequency: 240MHz (WiFi/BT)  
- Flash Frequency: 80MHz
- Flash Mode: QIO
- Flash Size: 4MB (32Mb)
- Partition Scheme: Default 4MB with spiffs

### Required Libraries
- **LiquidCrystal I2C** by Frank de Brabander (v1.1.2+)
- ESP32 BLE libraries (included with board package)

## Build and Upload Commands

Since this is an Arduino project, there are no traditional build commands. Development workflow:

1. **Compile**: Arduino IDE → Verify button (or Ctrl+R)
2. **Upload**: Arduino IDE → Upload button (or Ctrl+U)
   - May need to hold BOOT button on ESP32 during upload
3. **Monitor Serial**: Arduino IDE → Tools → Serial Monitor (115200 baud)

## Architecture

### Modular Design
The codebase uses a modular architecture with separate managers for each subsystem:

- **ESP32_MIDI_Pedal.ino**: Main application loop, BLE initialization, and event routing
- **MidiHandler**: Manages BLE MIDI packet formatting and transmission
- **ButtonManager**: Handles debouncing, long-press detection, and button combinations
- **DisplayManager**: Controls LCD output with custom characters for battery/BT icons
- **BatteryManager**: ADC voltage reading with moving average, charge detection
- **ConfigManager**: Persistent storage using ESP32 Preferences API
- **config.h**: Central configuration for pins, constants, and system settings

### Key Design Patterns

1. **Event-Driven Button Handling**: ButtonManager returns events (press/release/long-press) rather than states, allowing the main loop to handle complex combinations.

2. **BLE MIDI Protocol**: Uses standard MIDI service UUID (03B80E5A-EDE8-4B33-A751-6CE34EC4C700) with 5-byte packet format for compatibility with DAWs.

3. **Power Management**: Implements deep sleep with GPIO wake-up to maximize battery life when inactive.

### Critical Pin Assignments
- **I2C LCD**: SDA=GPIO21, SCL=GPIO22
- **Buttons**: GPIO32-33 (1-2), GPIO25-26 (3-4), GPIO27,14 (5-6)
- **Battery ADC**: GPIO34 (with 1:2 voltage divider)
- **Charge Status**: GPIO35 (from TC4056)

## Testing and Debugging

### I2C Scanner
If LCD issues occur, run the I2C scanner:
```
File → Examples → Wire → i2c_scanner
```
Update `LCD_ADDRESS` in config.h if needed (0x27 or 0x3F).

### Serial Debug Output
The code includes extensive Serial.print debugging at 115200 baud. Monitor for:
- MIDI messages sent
- BLE connection status
- Battery voltage readings
- Configuration changes

## Hardware Constraints

- **ADC**: Only use ADC1 pins (GPIO32-39) as ADC2 conflicts with BLE
- **Voltage Levels**: All GPIOs are 3.3V maximum
- **Battery Voltage**: 3.0-4.2V range, divided to 1.5-2.1V for ADC

## Common Modifications

### Changing MIDI CC Numbers
Edit `MIDI_CC_BUTTON_1` through `MIDI_CC_BUTTON_6` in config.h

### Adjusting Button Behavior
- Long press timing: `LONG_PRESS_TIME` in config.h
- Debounce delay: `DEBOUNCE_DELAY` in config.h
- Sleep timeout: `SLEEP_TIMEOUT` in config.h

### Button Function Mapping
All 6 buttons send MIDI CC on normal press. Buttons 5-6 additionally:
- Long press B5: Channel up
- Long press B6: Channel down
- B1+B2: Pairing mode
- B5+B6 long press: Factory reset