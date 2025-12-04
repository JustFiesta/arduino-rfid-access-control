# System Logic – RFID Access Control

This document describes the logic flow of the Arduino-based RFID access control system.

---

## Overview

The system grants or denies access based on RFID card authentication.

It uses the following components:

- **RFID module (RC522)** - Card identification
- **OLED Display (SSD1306 128x64)** - User interface display
- **Servo motor (SG90)** - Physical lock mechanism
- **Buzzer (Passive)** - Audio feedback

---

## System Flow

### 1. Initialization

**Actions:**

- Initialize RFID module and verify connection
- Scan I2C bus for OLED display
- Initialize OLED display (SSD1306)
- Set servo to locked position (0°)
- Display "Access Control Ready..." then "Scan your card to enter..."

**Serial Output:**

```
=== RFID Access Control ===
--- RFID Status ---
RFID OK - Firmware: 0x92
--- I2C Scan ---
I2C device at 0x3C
--- OLED Status ---
OLED OK at 0x3C!
--- Servo Status ---
Servo: LOCKED (0°)
--- Buzzer Status ---
Buzzer: Ready
=== System Ready ===
```

### 2. Card Scan

**Wait for RFID card:**

- System continuously polls RFID reader
- OLED displays "Scan your card to enter..."

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

cardEmojis[] = {
  "(=^w^=)",  // Admin emoji
  "(^ω^)",    // User emoji
};
```

**If card is NOT authorized:**

1. Display "ACCESS DENIED!" on OLED (centered, large text)
2. Display sad emoji "(T_T)" below
3. Sound buzzer (sad descending melody - ~1000ms)
4. Log denial to Serial Monitor
5. Wait 1.5 seconds
6. Return to idle state

**If card IS authorized:**

1. Display "~ Welcome ~" on OLED (centered)
2. Wait 1 second
3. Display "[Name]-san" with custom emoji (e.g., "Admin-san" with "(=^w^=)")
4. Play welcome melody (cheerful ascending tones)
5. Display ">> Unlocking <<"
6. Play door open fanfare (victory melody)
7. Move servo from 0° to 90° (unlock)
8. Keep door unlocked for 3 seconds
9. Display ">> Locking <<"
10. Play door close melody (gentle descending tones)
11. Move servo from 90° to 0° (lock)
12. Return to idle state

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

**Welcome Melody (cute sparkle notification):**
- E6 (60ms) → G6 (60ms) → B6 (60ms) → E7 (100ms) → C7 (80ms) → E7 (120ms)
- Anime-style "kyun~" sound effect

**Door Open Fanfare (victory melody):**
- C6 (120ms) × 3 → C6 sustained (200ms) → E6 (120ms) → G6 (300ms)
- Final Fantasy-style victory theme

**Door Close Melody (gentle goodbye):**
- G6 (150ms) → E6 (150ms) → D6 (150ms) → C6 (250ms) → B5 (150ms)
- "Mata ne~" (see you later) style

### Access Denied Pattern

**Error Melody (sad descending tones):**
- D5 (200ms) → B4 (200ms) → A4 (200ms) → E4 (400ms) → pause → D4 (150ms) → C4 (300ms)
- "Dame dame" (nope nope) style with dramatic defeated ending

---

## Display Layout (OLED 128x64)

### Main Screen (Idle)
```
┌────────────────────┐
│                    │
│  Scan your card    │
│                    │
│    to enter...     │
│                    │
│                    │
└────────────────────┘
```

### Welcome Screen
```
┌────────────────────┐
│   ~ Welcome ~      │
│                    │
│     Admin-san      │
│                    │
│     (=^w^=)        │
│                    │
└────────────────────┘
```

### Unlocking Screen
```
┌────────────────────┐
│                    │
│ >> Unlocking <<    │
│                    │
│     Door...        │
│                    │
│     ( -_-)>        │
└────────────────────┘
```

### Locking Screen
```
┌────────────────────┐
│                    │
│  >> Locking <<     │
│                    │
│   Stay safe!       │
│                    │
│     (^_^)b         │
└────────────────────┘
```

### Access Denied Screen
```
┌────────────────────┐
│                    │
│     ACCESS         │
│     DENIED!        │
│                    │
│     (T_T)          │
│                    │
└────────────────────┘
```

---

## Hardware Connections

### RFID RC522

- **RST** → Pin 9
- **SDA/SS** → Pin 10
- **MOSI** → Pin 11
- **MISO** → Pin 12
- **SCK** → Pin 13
- **3.3V** → 3.3V (CRITICAL: NOT 5V!)
- **GND** → GND

### OLED Display SSD1306 (128x64, I2C)

- **VCC** → 5V
- **GND** → GND
- **SDA** → A4
- **SCL** → A5

**I2C Address:** 0x3C or 0x3D (auto-detected during setup)

### Servo Motor SG90

- **Signal (Yellow/Orange)** → Pin 6
- **VCC (Red)** → 5V
- **GND (Brown/Black)** → GND

### Buzzer (Passive)

- **Positive (+)** → Pin 5
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
Play Welcome       Play Error Sound
   ↓                    ↓
Display Unlock     Show Sad Emoji
Play Fanfare            ↓
   ↓               Wait 1.5s
Open Servo (90°)        ↓
   ↓                    │
Display Locking         │
Play Close Melody       │
   ↓                    │
Close Servo (0°)        │
   ↓                    ↓
[RETURN TO IDLE] ←──────┘
```

---

## Timing Specifications

| Action | Duration |
|--------|----------|
| Welcome message | 1.0s |
| Username + emoji display | 1.0s |
| Door open time | 3.0s |
| Denied message | 1.5s |
| Anti-bounce delay | 0.5s |
| Welcome melody | 0.6s |
| Door open fanfare | 1.0s |
| Door close melody | 0.8s |
| Error melody | 1.2s |

**Total access granted sequence:** ~8 seconds  
**Total access denied sequence:** ~2.3 seconds

---

## Troubleshooting

### RFID not detected

- Check 3.3V power (NOT 5V!)
- Verify SPI connections (MOSI, MISO, SCK, SS)
- Ensure RST pin is connected
- Check Serial Monitor for "RFID OK - Firmware: 0x92"

### OLED not working

- Run I2C scanner (included in setup diagnostics)
- Try both addresses: 0x3C and 0x3D
- Check SDA/SCL connections (A4/A5)
- Verify 5V power supply
- Ensure sufficient RAM available (~1KB needed for framebuffer)
- Check Serial Monitor for "OLED OK at 0x3C!" or "OLED OK at 0x3D!"

### OLED displays garbled text

- Shorten I2C wires (keep under 15cm)
- Check common ground connection
- Verify no loose connections on breadboard
- Check for memory issues (free RAM should be >700 bytes)

### Servo not moving

- Verify 5V power supply can handle servo current (100-250mA)
- Check signal wire connection to Pin 6
- Test servo with simple sweep code
- Consider external 5V/2A power supply if Arduino resets

### Buzzer not sounding

- Check polarity (+ to Pin 5, - to GND)
- Verify pinMode() is set to OUTPUT in setup
- Test with simple tone() function
- Ensure passive buzzer (not active type)
- Try different frequencies (2000-4000 Hz are loudest)

### System resets when servo moves

- **Cause:** Servo current draw causes voltage drop
- **Solution:** Use external 5V/2A power supply
- Connect external power to breadboard rail
- **CRITICAL:** Ensure common ground between Arduino and external supply
- Alternative: Add 100μF capacitor across servo power pins

---

## Memory Considerations

**ATmega328P SRAM:** 2KB total

**Memory Usage:**
- OLED framebuffer: 1024 bytes (128×64÷8)
- RFID library: ~50 bytes
- Servo library: ~20 bytes
- Display library: ~30 bytes
- Card arrays: ~100 bytes
- Other variables: ~50 bytes
- **Total static:** ~1300 bytes
- **Available for stack:** ~700 bytes

**Optimization techniques used:**
- `const char*` arrays instead of `String` objects
- `F()` macro for serial strings (stored in flash, not RAM)
- Character arrays instead of String concatenation
- Minimal global variables

---

## OLED Display Technical Details

**Controller:** SSD1306  
**Resolution:** 128 pixels (width) × 64 pixels (height)  
**Technology:** Monochrome OLED (Organic LED)  
**Communication:** I2C (2-wire)  
**I2C Speed:** 400 kHz (Fast Mode)  
**Viewing Angle:** >160°  
**Power:** 3.3-5V supply (onboard regulator)

**Advantages over LCD:**
- No backlight needed (self-emitting)
- Higher contrast ratio
- Wider viewing angles
- Faster refresh rate (~15ms for full screen)
- Lower power consumption per pixel
- Smaller physical size
- Only 2 pins required (I2C vs 6+ for parallel LCD)

**Library Used:** Adafruit SSD1306 + Adafruit GFX

**Display Initialization:**
```cpp
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// In setup():
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
display.setTextSize(1);      // Normal 1:1 pixel scale
display.setTextColor(SSD1306_WHITE);
display.clearDisplay();
display.display();
```

---

## User Experience Design

The system uses a "weeb-style" (anime-inspired) aesthetic with:

- **Cute emojis** for each user (customizable)
- **Anime-style sound effects** (sparkle notifications, victory fanfares)
- **Friendly Japanese suffixes** ("-san" added to usernames)
- **Playful messages** ("Stay safe!", "Mata ne~")

This design makes the system more engaging for educational purposes and demonstrates attention to user experience beyond pure functionality.

**Customization:**
Users can easily modify emojis and sounds in the code to match their preferred style (professional, playful, minimal, etc.).

---

## Security Considerations

**Current Implementation:**
- UID-based authentication (simple but vulnerable)
- Hardcoded card list (limited flexibility)
- No encryption (UID transmitted in plaintext)

**Vulnerabilities:**
1. UID cloning - RFID UIDs can be copied
2. Replay attacks - captured signals can be replayed
3. Physical access - exposed Arduino can be reprogrammed

**Recommended Improvements:**
1. Challenge-response authentication using RFID memory blocks
2. Encrypted data storage on cards
3. Two-factor authentication (RFID + PIN via keypad)
4. Tamper detection (magnetic sensors)
5. Access logging with timestamps (RTC + SD card)
6. Remote monitoring (WiFi/Ethernet module)

**Suitable For:**
- Educational projects 
- Home hobby projects 
- Low-security applications 
- Commercial security 
- High-value asset protection 

---

## Key Takeaways

- **Modular hardware** - Each component serves a specific purpose
- **Multiple protocols** - SPI, I2C, PWM, UART working together
- **Event-driven logic** - React to RFID detection
- **User feedback** - Visual (OLED), audio (buzzer), physical (servo)
- **Scalability** - Easy to add more features (keypad, RTC, network)
- **Memory efficient** - Optimized for 2KB SRAM constraint

This project teaches practical embedded systems development:
- Peripheral interfacing
- Protocol implementation
- Real-time response
- User experience design
- Security fundamentals