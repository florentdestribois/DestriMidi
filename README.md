# ESP32 Bluetooth MIDI Pedal Controller

A professional-grade Bluetooth MIDI pedal controller based on ESP32 with battery management, 7-segment display, and 6 configurable MIDI CC buttons.

## Features

- **6 MIDI CC Buttons**: All buttons send MIDI Control Change messages (CC#1-6)
- **9 MIDI Channels**: Configurable channels (1-9) with long press on buttons 5/6
- **Bluetooth MIDI**: BLE MIDI compatible with Mac, PC, iOS, and Android
- **Battery Powered**: 3.7V LiPo with USB-C charging via TC4056 module
- **7-Segment Display**: Real-time channel display (1-9) with battery indicator
- **Auto-Reconnect**: Automatically reconnects to paired devices
- **Power Management**: Sleep mode for extended battery life
- **Factory Reset**: Hold buttons 5+6 for system reset

## Hardware Requirements

### Core Components
- ESP32-WROOM-32 DevKit
- 7-segment common cathode display
- TC4056 Type-C USB charging module with protection
- 3.7V 2000mAh LiPo battery with JST connector  
- **💡 POWER TIP**: Connect battery to ESP32 **3.3V pin** (not VIN) for optimal Bluetooth performance
- 6x Momentary push buttons
- 3x LEDs (Power, Bluetooth, Charging)

### Additional Components
- 2x 10kΩ resistors (battery voltage divider)
- 3x 220Ω resistors (LED current limiting)
- Hookup wire
- Project enclosure

## Arduino IDE Setup

### 1. Install ESP32 Board Support
1. Open Arduino IDE (version 2.0+ recommended)
2. Go to **File → Preferences**
3. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "ESP32" and install "esp32 by Espressif Systems"

### 2. Select Board Configuration
- **Board**: "ESP32 Dev Module"
- **Upload Speed**: 921600
- **CPU Frequency**: 240MHz (WiFi/BT)
- **Flash Frequency**: 80MHz
- **Flash Mode**: QIO
- **Flash Size**: 4MB (32Mb)
- **Partition Scheme**: Default 4MB with spiffs
- **Core Debug Level**: None
- **Port**: Select your ESP32's COM port

### 3. No Additional Libraries Required
All required libraries are included with the ESP32 board package:
- BLE libraries for Bluetooth MIDI
- Standard Arduino GPIO functions for 7-segment display control

### 4. Upload the Code
1. Connect ESP32 via USB
2. Open `ESP32_MIDI_Pedal.ino`
3. Verify all module files (.h and .cpp) are in the same folder
4. Click **Upload** button
5. Hold BOOT button on ESP32 if upload fails to start

## First Time Setup

### 1. 7-Segment Display Check
If display doesn't work:
1. Verify all segment pins are connected correctly (A-G + DP)
2. Check common cathode connection to ground
3. Test individual segments by modifying pin assignments in code

### 2. Bluetooth Pairing
1. Power on the pedal
2. Press buttons 1+2 simultaneously for pairing mode
3. On your device:
   - **Mac**: System Settings → Bluetooth → Connect "ESP32 MIDI Pedal"
   - **PC**: Settings → Bluetooth → Add device
   - **iOS**: Settings → Bluetooth → Connect
4. Open your DAW and select "ESP32 MIDI Pedal" as MIDI input

## Usage Guide

### Button Functions

| Button | Normal Press | Long Press | Combination |
|--------|-------------|------------|-------------|
| 1 | Send CC#1 | - | With B2: Pairing mode |
| 2 | Send CC#2 | - | With B1: Pairing mode |
| 3 | Send CC#3 | - | - |
| 4 | Send CC#4 | - | - |
| 5 | Send CC#5 | Channel Down | - |
| 6 | Send CC#6 | Channel Up | - |

### 7-Segment Display Information

#### Display Layout
```
 AAA
F   B
F   B
 GGG
E   C
E   C
 DDD  DP
```

#### What the Display Shows
```
- Channels 1-9: Single digit (1, 2, 3... 9)
- Battery mode: 0-9 (with decimal point if charging)
- Pairing mode: Blinking 'P'
```

### LED Indicators
- **Power LED**: Device is on
- **Bluetooth LED**: Connected to device
- **Charging LED**: Battery charging

## DAW Configuration

### Ableton Live
1. Preferences → Link/Tempo/MIDI
2. Enable "ESP32 MIDI Pedal" in MIDI Input
3. Map CC messages to parameters using MIDI Learn

### Logic Pro
1. Control Surfaces → Setup
2. New → Install → MIDI Controller
3. Select "ESP32 MIDI Pedal"

### Reaper
1. Options → Preferences → Audio → MIDI Devices
2. Enable "ESP32 MIDI Pedal" for input
3. Use Actions → Learn to map CCs

## Battery Management

- **Charging**: Connect USB-C cable (5V, 1A max)
- **Battery Life**: ~10 hours continuous use
- **Sleep Mode**: Automatic after 5 minutes idle
- **Wake Up**: Press any button

## Troubleshooting

### 7-Segment Display Not Working
- Check all 8 pin connections (A-G segments + decimal point)
- Verify common cathode is connected to ground
- Test individual segments by setting pins HIGH/LOW manually

### Bluetooth Not Connecting
- Ensure BT is enabled on your device
- Try factory reset (buttons 5+6 long press)
- Remove device from BT settings and re-pair

### No MIDI Output
- Verify BT connection (LED should be on)
- Check MIDI channel matches DAW
- Confirm DAW MIDI input is enabled

### Compilation Errors
- Ensure all .cpp and .h files are in sketch folder
- Verify ESP32 board package is installed
- Check library dependencies

## Power Consumption

- **Active**: ~80mA @ 3.7V
- **Sleep**: <10µA
- **Charging**: 1000mA max (TC4056 default)
- **Minimum Voltage**: 3.0V (shutdown imminent below this)

## Customization

### Changing MIDI CC Numbers
Edit in `config.h`:
```cpp
#define MIDI_CC_BUTTON_1 1  // Change to desired CC
#define MIDI_CC_BUTTON_2 2
// etc...
```

### Adjusting Sleep Timeout
In `config.h`:
```cpp
#define SLEEP_TIMEOUT 300000  // milliseconds
```

### Custom Device Name
In `config.h`:
```cpp
#define DEVICE_NAME "Your Custom Name"
```

## Project Files Structure
```
ESP32_MIDI_Pedal/
├── ESP32_MIDI_Pedal.ino    # Main sketch
├── config.h                 # Configuration constants
├── MidiHandler.h/cpp        # MIDI communication
├── DisplayManager.h/cpp     # LCD control
├── ButtonManager.h/cpp      # Button handling
├── BatteryManager.h/cpp     # Battery monitoring
└── ConfigManager.h/cpp      # Settings storage
```

## Safety Notes

⚠️ **Battery Safety**:
- Use protected LiPo batteries only
- TC4056 provides overcharge/discharge protection
- Do not exceed 4.2V charging voltage
- Monitor battery temperature during first charge

⚠️ **Electrical Safety**:
- ESP32 GPIOs are 3.3V max
- Use appropriate resistors for voltage divider
- Ensure proper grounding
- Disconnect battery before modifying circuit

## License

This project is open source. Feel free to modify and share.

## Support

For issues or questions about the code, please check the wiring diagram (WIRING_DIAGRAM.md) for detailed connection instructions.

## Version History

- **v1.0.0** - Initial release with 6 MIDI CC buttons, BLE support, battery management