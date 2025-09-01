# ESP32 Bluetooth MIDI Pedal - Wiring Diagram

## Component List
- ESP32-WROOM-32 DevKit
- LCD 16x2 with I2C module (PCF8574)
- TC4056 Type-C USB charging module
- 3.7V 2000mAh LiPo battery with JST connector
- 6x Push buttons (momentary)
- 3x LEDs (Power, Charging, Bluetooth status)
- 2x 10kΩ resistors (battery voltage divider)
- 3x 220Ω resistors (LED current limiting)
- 6x 10kΩ resistors (button pull-up - optional if using internal pull-ups)

## Pin Assignments

### ESP32 GPIO Connections

| ESP32 Pin | Connected To | Function |
|-----------|--------------|----------|
| GPIO 21 (SDA) | LCD I2C SDA | I2C Data |
| GPIO 22 (SCL) | LCD I2C SCL | I2C Clock |
| GPIO 34 | Battery Voltage Divider | ADC for battery monitoring |
| GPIO 35 | TC4056 CHRG pin | Charging status |
| GPIO 32 | Button 1 | MIDI Control 1 |
| GPIO 33 | Button 2 | MIDI Control 2 |
| GPIO 25 | Button 3 | MIDI Control 3 |
| GPIO 26 | Button 4 | MIDI Control 4 |
| GPIO 27 | Button 5 (Config Up) | Channel Up / Settings |
| GPIO 14 | Button 6 (Config Down) | Channel Down / Settings |
| GPIO 12 | Power LED | Status indicator |
| GPIO 13 | Bluetooth LED | BT connection status |
| GPIO 15 | Charging LED | Charging indicator |
| 3V3 | LCD VCC, Pull-ups | Power supply |
| GND | Common Ground | Ground reference |
| VIN | TC4056 OUT+ | Power input (5V from USB or battery) |

## Detailed Connection Instructions

### 1. Power Supply Circuit
```
TC4056 Module:
- IN+ → USB Type-C VBUS (5V)
- IN- → USB Type-C GND
- BAT+ → LiPo Battery positive (JST red wire)
- BAT- → LiPo Battery negative (JST black wire)
- OUT+ → ESP32 VIN pin (5V input)
- OUT- → ESP32 GND
- CHRG → ESP32 GPIO35 (through 10kΩ pull-up to 3.3V)
```

### 2. Battery Voltage Monitoring
```
Battery Voltage Divider:
LiPo+ → R1 (10kΩ) → GPIO34 → R2 (10kΩ) → GND

This creates a 1:2 voltage divider:
- Max battery voltage: 4.2V
- Voltage at GPIO34: 2.1V (safe for 3.3V ADC)
```

### 3. LCD I2C Connection
```
LCD with I2C Module (PCF8574):
- VCC → ESP32 3V3
- GND → ESP32 GND
- SDA → ESP32 GPIO21
- SCL → ESP32 GPIO22
- I2C Address: 0x27 (or 0x3F, check with I2C scanner)
```

### 4. Button Connections
```
Each button connects:
Button Terminal 1 → ESP32 GPIO pin
Button Terminal 2 → GND

Internal pull-up resistors will be enabled in code.
For external pull-ups: GPIO → 10kΩ → 3V3
```

### 5. LED Connections
```
Each LED circuit:
ESP32 GPIO → 220Ω resistor → LED Anode (+)
LED Cathode (-) → GND
```

## Button Functions

| Button | Normal Press | Long Press | Combined |
|--------|-------------|------------|----------|
| Button 1 | MIDI CC#1 | - | With B2: Pairing mode |
| Button 2 | MIDI CC#2 | - | With B1: Pairing mode |
| Button 3 | MIDI CC#3 | - | - |
| Button 4 | MIDI CC#4 | - | - |
| Button 5 | MIDI CC#5 | Channel Up | With B6: Factory Reset |
| Button 6 | MIDI CC#6 | Channel Down | With B5: Factory Reset |

## Physical Layout Suggestion
```
    [LCD Display 16x2]
    
[B1]  [B2]  [B3]  [B4]
      
    [B5]      [B6]
    
[Power LED] [BT LED] [Charge LED]

    [USB Type-C Port]
```

## Important Notes
1. **Battery Protection**: The TC4056 module includes overcharge and over-discharge protection
2. **Voltage Levels**: ESP32 GPIOs are 3.3V logic - do not exceed this voltage
3. **ADC Pins**: Only use ADC1 pins (GPIO32-39) as ADC2 conflicts with WiFi/Bluetooth
4. **Power Consumption**: Enable deep sleep when inactive to maximize battery life
5. **Charging Current**: TC4056 default is 1A, suitable for 2000mAh battery