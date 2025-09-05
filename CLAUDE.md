# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-based Bluetooth MIDI pedal controller project using Arduino framework. The system sends MIDI Control Change (CC) messages over Bluetooth Low Energy (BLE) and includes battery management, 7-segment display, and configurable MIDI channels.

## Development Environment

### PlatformIO Configuration
**Platform**: espressif32
**Board**: esp32dev
**Framework**: Arduino

### Required Libraries
- ESP32 BLE libraries (included with framework)
- Standard Arduino GPIO libraries (included)
- No additional libraries required for 7-segment display

## Build and Upload Commands

### PlatformIO Commands
1. **Build**: `pio run` or `platformio run`
2. **Upload**: `pio run -t upload` or `platformio run --target upload`
   - May need to hold BOOT button on ESP32 during upload
3. **Monitor Serial**: `pio device monitor` or `platformio device monitor` (115200 baud)
4. **Clean**: `pio run -t clean`
5. **Build & Upload**: `pio run -t upload --upload-port /dev/ttyUSB0` (adjust port as needed)

## Architecture

### Modular Design
The codebase uses a modular architecture with separate managers for each subsystem:

- **src/main.cpp**: Main application with 7-segment display control and BLE MIDI
- **7-Segment Display Functions**: Direct GPIO control for segments A-G and decimal point
- **Button Handling**: Debouncing, long-press detection, and button combinations
- **Battery Management**: ADC voltage reading with charging status detection
- **BLE MIDI Protocol**: Standard MIDI service with 5-byte packet format
- **Configuration Storage**: ESP32 Preferences API for persistent settings

### Key Design Patterns

1. **Event-Driven Button Handling**: ButtonManager returns events (press/release/long-press) rather than states, allowing the main loop to handle complex combinations.

2. **7-Segment Display Control**: Direct GPIO manipulation with digit patterns for 0-F, special patterns for pairing mode and battery display.

3. **BLE MIDI Protocol**: Uses standard MIDI service UUID (03B80E5A-EDE8-4B33-A751-6CE34EC4C700) with 5-byte packet format for compatibility with DAWs.

4. **Power Management**: Implements deep sleep with GPIO wake-up to maximize battery life when inactive.

### Critical Pin Assignments
- **7-Segment Display**: A=GPIO21, B=GPIO22, C=GPIO19, D=GPIO23, E=GPIO18, F=GPIO5, G=GPIO17, DP=GPIO16
- **Buttons**: GPIO32-33 (1-2), GPIO25-26 (3-4), GPIO27,14 (5-6)
- **LEDs**: Power=GPIO12, Bluetooth=GPIO13, Charging=GPIO15, Activity=GPIO4
- **Battery ADC**: GPIO34 (with 1:2 voltage divider)
- **Charge Status**: GPIO35 (from TC4056)

## Testing and Debugging

### 7-Segment Display Testing
If display issues occur, test individual segments by setting GPIO pins HIGH/LOW.
Verify common cathode connection to ground and check pin assignments in main.cpp.

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
Edit the `ccNumbers[6]` array initialization in src/main.cpp (default: {1, 2, 3, 4, 5, 6})

### Adjusting Button Behavior
- Long press timing: `LONG_PRESS_MS` in src/main.cpp
- Debounce delay: `BUTTON_DEBOUNCE_MS` in src/main.cpp
- Sleep timeout: `SLEEP_TIMEOUT_MS` in src/main.cpp
- Battery read interval: `BATTERY_READ_INTERVAL_MS` in src/main.cpp

### Button Function Mapping
All 6 buttons send MIDI CC on normal press. Buttons 5-6 additionally:
- Long press B5: Channel up
- Long press B6: Channel down
- B1+B2: Pairing mode
- B5+B6 long press: Factory reset