# System Logic – RFID Access Control

This document describes the logic flow of the Arduino-based RFID access control system.

---

## Overview

The system grants or denies access based on RFID card authentication.

It uses the following components:

- **RFID module (RC522)** - Card identification
- **LCD 1602A with I2C module** - User interface display
- **Servo motor** - Physical lock mechanism
- **Buzzer** - Audio feedback
- **LEDs (optional)** - Visual feedback (green/red)

---

## System Flow

### 1. Initialization

**Actions:**

- Initialize RFID module and verify connection
- Scan I2C bus for LCD
- Initialize LCD display
- Set servo to locked position (0°)
- Display "Access Control Ready..." then "Scan your card to enter..."

**Serial Output:**

```
=== RFID Access Control ===
--- RFID Status ---
RFID OK - Firmware: 0x92
--- I2C Scan ---
I2C device at 0x27
--- Servo Status ---
Servo: LOCKED (0°)
=== System Ready ===
```

### 2. Card Scan

**Wait for RFID card:**

- System continuously polls RFID reader
- LCD displays "Scan your card to enter..."

**When card detected:**

- Read card UID (Unique Identifier)
- Convert UID to uppercase hexadecimal format (e.g., "07:7B:2A:25")
- Compare with authorized cards list

### 3. Authorization Check

**Card Comparison:**

```cpp
authorizedCards[] = {
  "07:7B:2A:25",  // Admin card
  "69:51:45:14",  // User card
};

cardNames[] = {
  "Admin",
  "User"
};
```

**If card is NOT authorized:**

1. Display "ACCESS DENIED!" on LCD line 1
2. Display "Unknown card" on LCD line 2
3. Sound buzzer (long beep pattern - 1000ms)
4. Activate red LED (optional)
5. Log denial to Serial Monitor
6. Wait 2 seconds
7. Return to idle state

**If card IS authorized:**

1. Display "Welcome, [Name]!" (e.g., "Welcome, Admin!")
2. Wait 1.5 seconds
3. Display "Opening door..."
4. Sound buzzer (short success beep - 200ms)
5. Activate green LED (optional)
6. Move servo from 0° to 90° (unlock)
7. Keep door unlocked for 3 seconds
8. Display "Closing door..."
9. Move servo from 90° to 0° (lock)
10. Return to idle state

### 4. Serial Logging

**Every card scan produces log:**
```
==================
Card detected! UID: 07:7B:2A:25
ACCESS GRANTED for: Admin
>>> Opening lock (90°)
>>> Closing lock (0°)
==================
```

Or for denied access:

```
==================
Card detected! UID: AA:BB:CC:DD
ACCESS DENIED!
Unknown card: AA:BB:CC:DD
==================
```

---

## Audio Feedback (Buzzer)

### Access Granted Pattern

- **Short beep:** 200ms HIGH
- Indicates successful authentication

### Access Denied Pattern

- **Long beep:** 1000ms HIGH
- Indicates failed authentication

---

## Hardware Connections

### RFID RC522

- **RST** → Pin 9
- **SDA/SS** → Pin 10
- **MOSI** → Pin 11
- **MISO** → Pin 12
- **SCK** → Pin 13
- **3.3V** → 3.3V
- **GND** → GND

### LCD 1602 with I2C module

- **VCC** → 5V
- **GND** → GND
- **SDA** → A4
- **SCL** → A5

### Servo Motor

- **Signal** → Pin 6
- **VCC** → 5V
- **GND** → GND

### Buzzer

- **Positive (+)** → Pin 8
- **Negative (-)** → GND

---

## State Diagram

```
[IDLE] → Waiting for card
   ↓
[CARD DETECTED] → Read UID
   ↓
[CHECK AUTHORIZATION]
   ↓                    ↓
[AUTHORIZED]      [NOT AUTHORIZED]
   ↓                    ↓
Display Welcome    Display Denied
Short Beep         Long Beep
Open Servo (90°)   Red LED
Wait 3s            Wait 2s
Close Servo (0°)
   ↓                    ↓
[RETURN TO IDLE] ←―――――┘
```

---

## Timing Specifications

| Action | Duration |
|--------|----------|
| Welcome message | 1.5s |
| Door open time | 3.0s |
| Denied message | 2.0s |
| Debounce delay | 1.0s |
| Success beep | 0.2s |
| Denied beep | 1.0s |

---

## Troubleshooting

### RFID not detected

- Check 3.3V power (NOT 5V!)
- Verify SPI connections (MOSI, MISO, SCK, SS)
- Ensure RST pin is connected

### LCD not working

- Messed up groud connection - not enough ground, breadboard fails, etc
- Run I2C scanner to find correct address
- Try addresses: 0x27, 0x3F, 0x20
- Check SDA/SCL connections (A4/A5)

### Servo not moving

- Messed up groud connection - not enough ground, breadboard fails, etc
- Verify 5V power supply can handle servo current
- Check signal wire connection to Pin 6
- Test servo with simple sweep code

### Buzzer not sounding

- Check polarity (+ to Pin 8, - to GND)
- Verify pinMode() is set to OUTPUT
- Test with simple tone() function
