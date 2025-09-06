# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-based Bluetooth MIDI pedal controller project using Arduino framework. The system sends MIDI Control Change (CC) messages over Bluetooth Low Energy (BLE) and includes battery management, 8x8 LED matrix display (MAX7219), and configurable MIDI channels.

## Development Environment

### PlatformIO Configuration
**Platform**: espressif32
**Board**: esp32dev
**Framework**: Arduino

### Required Libraries
- ESP32 BLE libraries (included with framework)
- Standard Arduino GPIO libraries (included)
- MD_MAX72XX@^3.3.0 (8x8 LED matrix display control)

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

- **src/main.cpp**: Main application with 8x8 LED matrix display control and BLE MIDI
- **8x8 Matrix Display Functions**: MAX7219 controlled via MD_MAX72XX library (3 GPIO: DIN, CLK, CS)
- **Button Handling**: Debouncing, long-press detection, and button combinations
- **Battery Management**: ADC voltage reading with charging status detection
- **BLE MIDI Protocol**: Standard MIDI service with 5-byte packet format
- **Configuration Storage**: ESP32 Preferences API for persistent settings

### Key Design Patterns

1. **Event-Driven Button Handling**: ButtonManager returns events (press/release/long-press) rather than states, allowing the main loop to handle complex combinations.

2. **8x8 Matrix Display Control**: MAX7219 driver with custom 8x8 patterns for digits 0-9, battery level bargraph, and pairing mode display.

3. **BLE MIDI Protocol**: Uses standard MIDI service UUID (03B80E5A-EDE8-4B33-A751-6CE34EC4C700) with 5-byte packet format for compatibility with DAWs.

4. **Power Management**: Implements deep sleep with GPIO wake-up to maximize battery life when inactive.

### Critical Pin Assignments
- **8x8 Matrix Display**: DIN=GPIO21, CLK=GPIO18, CS=GPIO19 (MAX7219)
- **Buttons**: GPIO32-33 (1-2), GPIO25-26 (3-4), GPIO27,14 (5-6) 
- **LEDs**: Charging=GPIO15 (green), Activity=GPIO4 (orange, 1s flash)
- **Battery ADC**: GPIO35 (with 1:2 voltage divider)  
- **Available GPIOs**: 0,23,22,5,17,16,12,2,34 (freed from old 7-segment + LEDs)

## Testing and Debugging

### 8x8 Matrix Display Testing
If display issues occur:
1. Check SPI connections: DIN=GPIO21, CLK=GPIO18, CS=GPIO19
2. Verify MAX7219 power supply (3.3V or 5V)
3. Test with simple pattern: `mx.setRow(0, 0xFF)` should light up top row

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

### Display Patterns
- **MIDI Channel**: Shows "C" + digit (e.g., "C1", "C5")
- **Battery Level**: Horizontal bargraph with 6 levels (0-100%)
- **Pairing Mode**: Blinking "P" pattern + alternating LEDs (green/orange)
- **Connection Show**: Musical light sequence (5 seconds) - yellow 1x/sec, green 3x/sec
- **Charge Status**: Green LED blinks during charge, solid when complete

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