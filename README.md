# ESP32 Bluetooth MIDI Pedal Controller

A professional-grade Bluetooth MIDI pedal controller based on ESP32 with battery management, LCD display, and 6 configurable MIDI CC buttons.

## Features

- **6 MIDI CC Buttons**: All buttons send MIDI Control Change messages (CC#1-6)
- **16 MIDI Channels**: Configurable channels (1-16) with long press on buttons 5/6
- **Bluetooth MIDI**: BLE MIDI compatible with Mac, PC, iOS, and Android
- **Battery Powered**: 3.7V LiPo with USB-C charging via TC4056 module
- **LCD Display**: Real-time status showing channel, battery, and connection
- **Auto-Reconnect**: Automatically reconnects to paired devices
- **Power Management**: Sleep mode for extended battery life
- **Factory Reset**: Hold buttons 5+6 for system reset

## Hardware Requirements

### Core Components
- ESP32-WROOM-32 DevKit
- LCD 16x2 with I2C module (PCF8574)
- TC4056 Type-C USB charging module with protection
- 3.7V 2000mAh LiPo battery with JST connector
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

### 3. Install Required Libraries
Go to **Tools → Manage Libraries** and install:
- **LiquidCrystal I2C** by Frank de Brabander (version 1.1.2+)

Note: BLE libraries are included with ESP32 board package.

### 4. Upload the Code
1. Connect ESP32 via USB
2. Open `ESP32_MIDI_Pedal.ino`
3. Verify all module files (.h and .cpp) are in the same folder
4. Click **Upload** button
5. Hold BOOT button on ESP32 if upload fails to start

## First Time Setup

### 1. I2C Address Configuration
If LCD doesn't display:
1. Run I2C Scanner sketch (File → Examples → Wire → i2c_scanner)
2. Note the address (usually 0x27 or 0x3F)
3. Update `LCD_ADDRESS` in `config.h`

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
| 5 | Send CC#5 | Channel Up | With B6: Factory Reset |
| 6 | Send CC#6 | Channel Down | With B5: Factory Reset |

### LCD Display Information
```
Line 1: CH:01     🔷 ON  🔋
        └─Channel  └─BT  └─Battery

Line 2: Ready - 6xCC
        └─Status message
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

### LCD Not Working
- Check I2C connections (SDA→GPIO21, SCL→GPIO22)
- Verify I2C address with scanner
- Adjust contrast potentiometer on I2C module

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