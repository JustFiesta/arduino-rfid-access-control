# Theoretical Background – RFID Access Control System

This document explains the core theory behind each module used in the Arduino-based RFID access control project. Understanding these concepts will help you develop, troubleshoot, and extend the system.

It was created as our development guidelines and notes on given technology.

---

## RFID (Radio Frequency Identification)

### Principle

RFID uses electromagnetic fields to identify and track tags attached to objects. The reader (RC522 module) sends a radio signal at **13.56 MHz**, powering up the RFID tag (card or keychain).

### How It Works

1. **Reader emits RF field** at 13.56 MHz
2. **Tag antenna** receives energy and powers the chip
3. **Tag responds** by modulating the RF field with its data
4. **Reader decodes** the modulated signal
5. **Arduino receives** the UID via SPI communication

### Key Components

- **RC522 Module:** Reads and writes data to 13.56 MHz RFID tags (MIFARE protocol)
- **UID (Unique Identifier):** 4, 7, or 10-byte hexadecimal identifier
- **MIFARE Classic:** Most common tag type (1KB or 4KB memory)

### Communication Protocol

The RC522 uses **SPI (Serial Peripheral Interface)** to communicate with Arduino:

- **MOSI** (Master Out Slave In) - Arduino sends data
- **MISO** (Master In Slave Out) - RC522 sends data
- **SCK** (Serial Clock) - Synchronization signal
- **SS/SDA** (Slave Select) - Chip selection
- **RST** (Reset) - Module reset

### Arduino Library

**MFRC522** by GithubCommunity - provides functions to:

- Initialize the module
- Detect card presence
- Read UID
- Read/write data blocks
- Halt card communication

**Example Usage:**

```cpp
MFRC522 mfrc522(SS_PIN, RST_PIN);
mfrc522.PCD_Init();
if (mfrc522.PICC_IsNewCardPresent()) {
  // Card detected
}
```

---

## Servo Motor (SG90)

### Purpose

The servo motor acts as a physical lock mechanism, rotating between two positions:
- **0°** - Door locked
- **90°** - Door unlocked

### How Servos Work
Servos contain:
1. **DC motor** - provides rotation
2. **Gearbox** - reduces speed, increases torque
3. **Potentiometer** - measures shaft position
4. **Control circuit** - adjusts motor to reach target angle

### PWM Control

Servos are controlled by **Pulse Width Modulation (PWM)**:
- **Pulse period:** 20ms (50 Hz)
- **Pulse width determines angle:**
  - 1.0ms pulse = 0°
  - 1.5ms pulse = 90°
  - 2.0ms pulse = 180°

### Arduino Implementation

```cpp
#include <Servo.h>
Servo myservo;
myservo.attach(6);      // Control pin
myservo.write(90);      // Set angle
```

### Power Considerations

- SG90 draws 100-250mA under load
- Arduino's 5V pin can supply ~400mA total
- For multiple servos or heavy loads, use external 5V power supply

---

## LCD 1602 with I2C Interface

### Purpose

The LCD displays system messages such as "Scan your card" or "Access Granted."

### LCD Technology

- **1602** means: 16 columns × 2 rows
- **HD44780 controller** - industry standard LCD driver
- **Character-based** - displays ASCII characters, not pixels

### I2C Advantage

Instead of using 12+ pins for parallel communication, I2C uses only:

- **SDA** (Serial Data) - Bidirectional data line
- **SCL** (Serial Clock) - Clock signal
- **VCC** and **GND** - Power

### I2C Protocol

- **Multi-device bus** - can connect up to 127 devices
- **7-bit addressing** - each device has unique address (e.g., 0x27)
- **Two-wire interface** - SDA and SCL lines
- **Pull-up resistors** required (usually built into I2C backpack)

### PCF8574 I2C Expander

The I2C backpack uses PCF8574 chip to convert:

- I2C signals → Parallel LCD signals
- Reduces Arduino pins needed from 12 to 2

### Arduino Library

**LiquidCrystal_I2C** - provides functions to:

```cpp
LiquidCrystal_I2C lcd(0x27, 16, 2);
lcd.init();
lcd.backlight();
lcd.setCursor(0, 0);    // Column, Row
lcd.print("Hello!");
lcd.clear();
```

### Finding I2C Address

Use I2C scanner to detect device address:
```cpp
Wire.begin();
Wire.beginTransmission(address);
if (Wire.endTransmission() == 0) {
  // Device found
}
```

---

## Buzzer (Passive vs Active)

### Types

**Passive Buzzer:**

- Requires **frequency signal** to produce sound
- Can generate different **tones/melodies**
- Controlled with `tone()` function
- More versatile but slightly more complex

**Active Buzzer:**

- Has internal oscillator
- Simple **ON/OFF control** (digitalWrite)
- Fixed frequency (~2kHz)
- Easier to use for simple beeps

### Our Implementation (Active Buzzer)

```cpp
#define BUZZER_PIN 8
pinMode(BUZZER_PIN, OUTPUT);

// Short beep
digitalWrite(BUZZER_PIN, HIGH);
delay(200);
digitalWrite(BUZZER_PIN, LOW);

// Long beep
digitalWrite(BUZZER_PIN, HIGH);
delay(1000);
digitalWrite(BUZZER_PIN, LOW);
```

### Passive Buzzer Alternative

```cpp
tone(BUZZER_PIN, 1000);    // 1000 Hz
delay(200);
noTone(BUZZER_PIN);
```

### Audio Feedback Design

- **Short beep (200ms)** - Success/confirmation
- **Long beep (1000ms)** - Error/denial
- **Multiple beeps** - Warnings/alerts
- **Different frequencies** - Different event types

---

## Arduino UNO

### Role

Acts as the **microcontroller brain** coordinating all components:

- Reads RFID data via SPI
- Controls LCD via I2C
- Moves servo via PWM
- Activates buzzer via digital output
- Logs events via serial communication

### ATmega328P Specifications

- **Clock speed:** 16 MHz
- **Flash memory:** 32 KB (program storage)
- **SRAM:** 2 KB (runtime variables)
- **EEPROM:** 1 KB (persistent storage)
- **Digital I/O pins:** 14 (6 PWM-capable)
- **Analog input pins:** 6
- **Operating voltage:** 5V

### Pin Usage in This Project

| Pin | Function | Protocol |
|-----|----------|----------|
| 0-1 | Serial (USB) | UART |
| 6 | Servo control | PWM |
| 8 | Buzzer | Digital Out |
| 9 | RFID RST | Digital Out |
| 10 | RFID SS/SDA | SPI |
| 11 | RFID MOSI | SPI |
| 12 | RFID MISO | SPI |
| 13 | RFID SCK | SPI |
| A4 | LCD SDA | I2C |
| A5 | LCD SCL | I2C |

### Memory Management

```cpp
String uidString = "";              // Uses heap memory
const char* cardName = "Admin";     // Stored in flash
byte buffer[18];                    // Stack allocation
```

**Best practices:**

- Use `const` for strings stored in flash (saves SRAM)
- Avoid `String` class for memory-constrained projects
- Use `F()` macro for serial strings: `Serial.println(F("Text"));`

---

## Communication Protocols

### SPI (Serial Peripheral Interface)

**Used by:** RFID RC522

**Characteristics:**

- **Full-duplex** - simultaneous send/receive
- **High speed** - up to 10 Mbps
- **Master-slave** architecture
- **4-wire interface** - MOSI, MISO, SCK, SS

**Advantages:**

- Fast data transfer
- Simple hardware implementation
- No addressing needed (SS pin selects device)

### I2C (Inter-Integrated Circuit)

**Used by:** LCD with PCF8574 backpack

**Characteristics:**

- **Half-duplex** - one direction at a time
- **Moderate speed** - 100 kHz (standard) or 400 kHz (fast)
- **Multi-master, multi-slave**
- **2-wire interface** - SDA, SCL

**Advantages:**

- Minimal pin usage
- Multiple devices on same bus
- Built-in addressing

### PWM (Pulse Width Modulation)

**Used by:** Servo motor

**Principle:**

- Rapid HIGH/LOW switching
- **Duty cycle** determines average voltage
- **Frequency** determines smoothness

**Arduino PWM:**

- Pins 3, 5, 6, 9, 10, 11 support PWM
- `analogWrite(pin, 0-255)` sets duty cycle
- `Servo` library handles PWM timing automatically

---

## Security Considerations

### Current Implementation

- **UID-based authentication** - simple but vulnerable
- **Hardcoded card list** - limited flexibility
- **No encryption** - UID transmitted in plaintext

### Vulnerabilities

1. **UID cloning** - RFID UIDs can be copied
2. **Replay attacks** - captured signals can be replayed
3. **Physical access** - exposed Arduino can be reprogrammed

### Improvements

1. **Challenge-response authentication**
2. **Encrypted data blocks** on RFID tags
3. **Two-factor authentication** (RFID + PIN)
4. **Tamper detection** (magnetic sensors)
5. **Access logging** with timestamps
6. **Remote monitoring** via network module

---

## Key Takeaways

- **Modular hardware** - Each component serves specific purpose
- **Multiple protocols** - SPI, I2C, PWM, UART working together
- **Event-driven logic** - React to RFID detection
- **User feedback** - Visual (LCD), audio (buzzer), physical (servo)
- **Scalability** - Easy to add more features (keypad, RTC, network)

This project teaches practical embedded systems development:

- Peripheral interfacing
- Protocol implementation
- Real-time response
- User experience design
- Security fundamentals
