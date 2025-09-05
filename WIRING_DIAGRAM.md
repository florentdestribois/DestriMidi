# ESP32 Bluetooth MIDI Pedal - Wiring Diagram

## Component List
- ESP32-WROOM-32 DevKit
- 7-segment common cathode display (single digit)
- TC4056 Type-C USB charging module
- 3.7V 2000mAh LiPo battery with JST connector
- 6x Push buttons (momentary)
- 4x LEDs (Power, Charging, Bluetooth status, Activity)
- 2x 10kΩ resistors (battery voltage divider)
- 4x 220Ω resistors (LED current limiting)
- 6x 10kΩ resistors (button pull-up - optional if using internal pull-ups)

## Pin Assignments

### ESP32 GPIO Connections

| ESP32 Pin | Connected To | Function |
|-----------|--------------|----------|
| GPIO 21 | 7-Segment Pin A | Segment A |
| GPIO 22 | 7-Segment Pin B | Segment B |
| GPIO 19 | 7-Segment Pin C | Segment C |
| GPIO 23 | 7-Segment Pin D | Segment D |
| GPIO 18 | 7-Segment Pin E | Segment E |
| GPIO 5 | 7-Segment Pin F | Segment F |
| GPIO 17 | 7-Segment Pin G | Segment G |
| GPIO 16 | 7-Segment Pin DP | Decimal Point |
| GPIO 34 | Battery Voltage Divider | ADC for battery monitoring |
| GPIO 35 | Non utilisé | (TC4056 sans pin CHRG) |
| GPIO 32 | Button 1 | MIDI Control 1 |
| GPIO 33 | Button 2 | MIDI Control 2 |
| GPIO 25 | Button 3 | MIDI Control 3 |
| GPIO 26 | Button 4 | MIDI Control 4 |
| GPIO 27 | Button 5 (Config Down) | Channel Down / Settings |
| GPIO 0 | Button 6 (Config Up) | Channel Up / Settings |
| GPIO 12 | Power LED | Status indicator |
| GPIO 2 | Bluetooth LED | BT connection status |
| GPIO 15 | Charging LED | Charging indicator |
| GPIO 4 | Activity LED | MIDI activity indicator |
| 3V3 | Pull-ups | Power supply |
| GND | Common Ground | Ground reference |
| 3V3 | TC4056 OUT+ | Power input (3.3V direct - optimal for battery) |
| VIN | USB 5V | Power input (5V from USB only) |

## Detailed Connection Instructions

### 1. Power Supply Circuit
```
TC4056 Module:
- IN+ → USB Type-C VBUS (5V)
- IN- → USB Type-C GND
- BAT+ → LiPo Battery positive (JST red wire)
- BAT- → LiPo Battery negative (JST black wire)
- OUT+ → ESP32 3V3 pin (OPTIMAL - direct 3.3V for best BLE performance)
- OUT- → ESP32 GND
- CHRG → Non disponible sur ce modèle TC4056

Note: Battery voltage (3.7V-4.2V) works perfectly on 3.3V pin
```

### 2. Battery Voltage Monitoring

⚠️ **CRITICAL - VOLTAGE DIVIDER REQUIRED** ⚠️
**Never connect battery directly to GPIO34 - ESP32 maximum is 3.3V!**

#### Voltage Divider Circuit
```
Battery Voltage Monitoring Circuit:

        Battery +4.2V max
             │
             │
            ┌─────┐
            │ R1  │  10kΩ
            │10kΩ │
            └─────┘
             │
             ├──────── GPIO34 (ADC Input)
             │         Max: 2.1V (safe)
            ┌─────┐
            │ R2  │  10kΩ  
            │10kΩ │
            └─────┘
             │
            GND
```

#### Component Values & Calculations
```
Resistor Values: R1 = R2 = 10kΩ (1/4W, 5% tolerance)
Voltage Division: V_out = V_in × (R2 / (R1 + R2))
                 V_out = V_in × (10k / (10k + 10k))
                 V_out = V_in × 0.5

Battery Voltage Range:
- 4.2V (full) → 2.1V at GPIO34
- 3.7V (nominal) → 1.85V at GPIO34  
- 3.0V (low) → 1.5V at GPIO34
- 2.8V (critical) → 1.4V at GPIO34

ADC Reading: 12-bit (0-4095) at 3.3V reference
```

#### Physical Wiring
```
1. Battery+ → R1 (10kΩ) first terminal
2. R1 second terminal → R2 first terminal → GPIO34
3. R2 second terminal → ESP32 GND
4. Keep wires short and away from switching circuits
```

### 3. 7-Segment Display Connection

#### 7-Segment Layout
```
 AAA
F   B
F   B
 GGG
E   C
E   C
 DDD  DP
```

#### Pin Connections
```
7-Segment Display (Common Cathode):
- Pin A (top horizontal) → ESP32 GPIO21
- Pin B (top right vertical) → ESP32 GPIO22
- Pin C (bottom right vertical) → ESP32 GPIO19
- Pin D (bottom horizontal) → ESP32 GPIO23
- Pin E (bottom left vertical) → ESP32 GPIO18
- Pin F (top left vertical) → ESP32 GPIO5
- Pin G (middle horizontal) → ESP32 GPIO17
- Pin DP (decimal point) → ESP32 GPIO16
- Common Cathode → ESP32 GND

Note: No current limiting resistors needed for brief LED display usage.
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
    [7-Segment Display]
         (Shows Channel)
    
[B1]  [B2]  [B3]  [B4]
      
    [B5]      [B6]
    
[Power LED] [BT LED] [Charge LED]

    [USB Type-C Port]
```

## Important Notes
1. **Battery Protection**: The TC4056 module includes overcharge and over-discharge protection
2. **Voltage Levels**: ESP32 GPIOs are 3.3V logic - do not exceed this voltage
3. **ADC Pins**: Only use ADC1 pins (GPIO32-39) as ADC2 conflicts with WiFi/Bluetooth
4. **7-Segment Display**: Common cathode type required, segments light when GPIO is HIGH
4. **Power Consumption**: Enable deep sleep when inactive to maximize battery life
5. **Charging Current**: TC4056 default is 1A, suitable for 2000mAh battery
## 💡 OPTIMAL POWER CONFIGURATION
**Discovery**: Best Bluetooth performance with **direct 3.3V connection**
- **Battery → 3.3V pin**: Excellent BLE performance (3.7V-4.2V range perfect)
- **USB → VIN pin**: Standard 5V input (also works well)
- **Battery → VIN pin**: Not recommended (voltage regulation losses affect BLE)

**Recommended**: Connect battery OUTPUT+ to ESP32 **3.3V pin** for optimal performance and battery life.

**CRITICAL**: System operates down to 3.0V minimum - shutdown imminent below this voltage.
