# ESP32 Bluetooth MIDI Pedal - Wiring Diagram

## Component List
- ESP32-WROOM-32 DevKit
- MAX7219 8x8 LED Matrix Module (integrated)
- TC4056 Type-C USB charging module
- 3.7V 2000mAh LiPo battery with JST connector
- 6x Push buttons (momentary)
- 1x Green LED (charging indicator)
- 1x Orange LED (activity indicator)
- 2x 10kΩ resistors (battery voltage divider)
- 2x 220Ω resistors (LED current limiting)
- 6x 10kΩ resistors (button pull-up - optional if using internal pull-ups)
- 5x Dupont wires (for MAX7219 module connection)

## Pin Assignments

### ESP32 GPIO Connections

| ESP32 Pin | Connected To | Function |
|-----------|--------------|----------|
| GPIO 21 | MAX7219 DIN | Data Input |
| GPIO 18 | MAX7219 CLK | Clock Signal |
| GPIO 19 | MAX7219 CS | Chip Select |
| GPIO 35 | Battery Voltage Divider | ADC for battery monitoring |
| GPIO 4 | Orange LED (Activity) | Button press indication (1 sec) |
| GPIO 32 | Button 1 | MIDI Control 1 |
| GPIO 33 | Button 2 | MIDI Control 2 |
| GPIO 25 | Button 3 | MIDI Control 3 |
| GPIO 26 | Button 4 | MIDI Control 4 |
| GPIO 27 | Button 5 (Config Up) | Channel Up / Settings |
| GPIO 14 | Button 6 (Config Down) | Channel Down / Settings |
| GPIO 15 | Green LED (Charging) | Charging/Charge complete indicator |
| 3V3 | Pull-ups | Power supply |
| GND | Common Ground | Ground reference |
| 3V3 | MAX7219 Module VCC | Power for display module |
| VIN | TC4056 OUT+ | Power input from battery/charger |

## Detailed Connection Instructions

### 1. Power Supply Circuit
```
TC4056 Module:
- IN+ → USB Type-C VBUS (5V)
- IN- → USB Type-C GND
- BAT+ → LiPo Battery positive (JST red wire)
- BAT- → LiPo Battery negative (JST black wire)
- OUT+ → ESP32 VIN pin (Power input)
- OUT- → ESP32 GND
- CHRG → Non disponible sur ce modèle TC4056

Note: Battery voltage (3.7V-4.2V) regulated by ESP32 internal LDO via VIN pin
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
- 4.2V (full) → 2.1V at GPIO35
- 3.7V (nominal) → 1.85V at GPIO35  
- 3.0V (low) → 1.5V at GPIO35
- 2.8V (critical) → 1.4V at GPIO35

ADC Reading: 12-bit (0-4095) at 3.3V reference
```

#### Physical Wiring
```
1. TC4056 OUT+ (battery voltage) → R1 (10kΩ) first terminal
2. R1 second terminal → R2 first terminal → GPIO35 (ADC)
3. R2 second terminal → ESP32 GND
4. Keep wires short and away from switching circuits
5. This monitors actual battery voltage for charge detection
```

### 3. MAX7219 8x8 LED Matrix Module Connection

#### Module Layout
```
 MAX7219 8x8 LED Matrix Module
 ┌─────────────────────────────┐
 │  ● ● ● ● ● ● ● ●            │
 │  ● ● ● ● ● ● ● ●   MAX7219  │
 │  ● ● ● ● ● ● ● ●            │
 │  ● ● ● ● ● ● ● ●            │
 │  ● ● ● ● ● ● ● ●            │
 │  ● ● ● ● ● ● ● ●            │
 │  ● ● ● ● ● ● ● ●            │
 │  ● ● ● ● ● ● ● ●            │
 └─────────────────────────────┘
   VCC GND DIN CS CLK
```

#### Pin Connections
```
MAX7219 Module → ESP32:
- VCC → ESP32 3.3V (or 5V)
- GND → ESP32 GND
- DIN → ESP32 GPIO21 (Data Input)
- CS  → ESP32 GPIO19 (Chip Select) 
- CLK → ESP32 GPIO18 (Clock)

Display Capabilities:
- 8x8 LED matrix (64 individual LEDs)
- Custom patterns for MIDI channels (C1-C9 currently)
- Battery level bargraph display (6 levels)
- Pairing mode indicator (blinking P)
- Software brightness control (MD_MAX72XX library)
- SPI communication (3 control wires + power)
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
Green LED Circuit (Charging Status):
ESP32 GPIO15 → 220Ω resistor → Green LED Anode (+)
Green LED Cathode (-) → GND

Orange LED Circuit (Activity Indicator):
ESP32 GPIO4 → 220Ω resistor → Orange LED Anode (+)
Orange LED Cathode (-) → GND

LED Behaviors:
Green LED:
- Blinking: Battery charging
- Solid (5 sec): Charge complete
- Off: Normal operation

Orange LED:
- Flash 1 sec: Button pressed (MIDI sent)
- Flash 1 sec: Channel change
- Off: No activity
```

## Button Functions

| Button | GPIO | Normal Press | Long Press | Combined |
|--------|------|-------------|------------|----------|
| Button 1 | GPIO32 | MIDI CC#1 | - | With B2: Pairing mode |
| Button 2 | GPIO33 | MIDI CC#2 | - | With B1: Pairing mode |
| Button 3 | GPIO25 | MIDI CC#3 | - | With B4: Battery display |
| Button 4 | GPIO26 | MIDI CC#4 | - | With B3: Battery display |
| Button 5 | GPIO27 | MIDI CC#5 | Channel UP | With B6: Factory Reset |
| Button 6 | GPIO14 | MIDI CC#6 | Channel DOWN | With B5: Factory Reset |

## Physical Layout Suggestion
```
  ┌─────────────────────┐
  │ ● ● ● ● ● ● ● ●     │
  │ ● ● ● ● ● ● ● ●     │  ← 8x8 LED Matrix
  │ ● ● ● ● ● ● ● ●     │    (Shows Channel, Battery)
  │ ● ● ● ● ● ● ● ●     │
  └─────────────────────┘
    
[B1]  [B2]  [B3]  [B4]  ← MIDI Controls
      
    [B5]      [B6]       ← Config Buttons
    
   [Green LED] [Orange LED]  ← Status LEDs

    [USB Type-C Port]
```

## Important Notes
1. **Battery Protection**: The TC4056 module includes overcharge and over-discharge protection
2. **Voltage Levels**: ESP32 GPIOs are 3.3V logic - do not exceed this voltage
3. **ADC Pins**: Only use ADC1 pins (GPIO32-39) as ADC2 conflicts with WiFi/Bluetooth
4. **MAX7219 Module**: Pre-wired module eliminates complex matrix wiring (19 connections → 5 wires)
5. **Power Consumption**: Deep sleep enabled when inactive to maximize battery life  
6. **Charging Current**: TC4056 default 1A charging current, suitable for 2000mAh battery
7. **Display Library**: MD_MAX72XX@^3.3.0 library optimized for ESP32
8. **GPIO Savings**: 8 GPIO pins freed (GPIO 23,18,5,17,16,12,2,4) for expansions
9. **Button Pins**: All buttons use internal pull-up resistors, no external resistors needed
## 💡 OPTIMAL POWER CONFIGURATION
**Discovery**: Best Bluetooth performance with **direct 3.3V connection**
- **Battery → 3.3V pin**: Excellent BLE performance (3.7V-4.2V range perfect)
- **USB → VIN pin**: Standard 5V input (also works well)
- **Battery → VIN pin**: Not recommended (voltage regulation losses affect BLE)

**Recommended**: Connect TC4056 OUTPUT+ to ESP32 **VIN pin** for proper voltage regulation and battery life.

## Wiring Simplification Summary

**Before (7-segment + individual LEDs):**
- 8 pins for display segments (GPIO 21,22,19,23,18,5,17,16)
- 4 pins for status LEDs (GPIO 12,2,15,4)
- Total: 12 GPIO pins used for display

**After (MAX7219 module + 1 LED):**
- 3 pins for 8x8 matrix (GPIO 21,22,19)
- 1 pin for charging LED (GPIO 15)
- Total: 4 GPIO pins used for display

**GPIO pins freed: 8 pins now available for expansion!**

Freed pins: GPIO 0,23,22,5,17,16,12,2,34
Current usage: GPIO 32,33,25,26,27,14 (buttons), GPIO 35 (battery), GPIO 15,4 (LEDs), GPIO 21,18,19 (MAX7219)

**CRITICAL**: System operates down to 3.0V minimum - shutdown imminent below this voltage.

## Display Patterns

### MIDI Channel Display
```
C1, C2, C3... C9   ← Current: Channel 1-9 only
                     (Code limitation, not display)
```

### Battery Level Display
```
████████  100% (6/6 bars)
██████░░   80% (5/6 bars) 
████░░░░   60% (4/6 bars)
██░░░░░░   40% (3/6 bars)
█░░░░░░░   20% (2/6 bars)
░░░░░░░░    0% (1/6 bars)
```

### Pairing Mode
```
██ ██     ← Blinking "P" pattern
██ ██
████
██
██
```
