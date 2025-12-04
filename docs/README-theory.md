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

## OLED Display (SSD1306 - 128x64)

### Purpose

The OLED displays system messages such as "Scan your card" or "Access Granted."

### OLED Technology

**What is OLED?**
- **O**rganic **L**ight **E**mitting **D**iode
- Each pixel is self-emitting (no backlight needed)
- Organic compounds emit light when electric current applied
- Black pixels are truly OFF (no power consumption)

**Resolution:** 128 columns × 64 rows = 8,192 pixels

**Controller:** SSD1306 chip - industry standard OLED driver

### Advantages Over LCD

| Feature | OLED (SSD1306) | LCD (1602A) |
|---------|----------------|-------------|
| Backlight | Not needed (self-emitting) | Required |
| Contrast | Very high (∞:1) | Moderate (~10:1) |
| Viewing angle | >160° | ~45° |
| Power | Lower (black = off) | Higher (constant backlight) |
| Pins required | 2 (I2C) | 6-12 (parallel) or 4 (I2C) |
| Refresh rate | Fast (~15ms) | Slow (~50ms) |
| Resolution | 128×64 pixels | 16×2 characters |
| Graphics | Full graphics capable | Character-only (or custom) |
| Lifespan | 10,000-50,000 hours | 50,000+ hours |

### I2C Communication

Instead of using 12+ pins for parallel communication, I2C uses only:

- **SDA** (Serial Data) - Bidirectional data line
- **SCL** (Serial Clock) - Clock signal
- **VCC** and **GND** - Power

### I2C Protocol

- **Multi-device bus** - can connect up to 127 devices
- **7-bit addressing** - each device has unique address (e.g., 0x3C or 0x3D)
- **Two-wire interface** - SDA and SCL lines
- **Pull-up resistors** required (usually built into OLED module)

### I2C Communication Flow

1. **START condition:** SDA goes LOW while SCL is HIGH
2. **Address byte:** 7-bit device address + R/W bit
3. **ACK/NACK:** Slave acknowledges reception
4. **Data bytes:** 8-bit data + ACK after each byte
5. **STOP condition:** SDA goes HIGH while SCL is HIGH

### Arduino Library

**Adafruit_SSD1306** + **Adafruit_GFX** - provides functions to:

```cpp
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Initialize
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

// Basic drawing
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(SSD1306_WHITE);
display.setCursor(0, 0);
display.println("Hello!");
display.display();  // Push to screen
```

### Memory Requirements

**Framebuffer:** 128 × 64 ÷ 8 = **1024 bytes**

This is the largest single memory consumer in the project (50% of ATmega328P's 2KB SRAM).

The framebuffer is a bitmap in RAM that represents the entire screen. Changes are made to this buffer, then pushed to the OLED with `display.display()`.

### Finding I2C Address

OLED modules may use 0x3C or 0x3D depending on hardware configuration:

```cpp
Wire.begin();
for (byte address = 1; address < 127; address++) {
  Wire.beginTransmission(address);
  if (Wire.endTransmission() == 0) {
    Serial.print("Device found at 0x");
    Serial.println(address, HEX);
  }
}
```

The project includes automatic I2C scanning in setup with fallback to try both addresses.

### Display Rendering

**How pixels are organized:**
- Screen divided into 8 pages (rows of 8 pixels each)
- Each page is 128 bytes (one per column)
- Each byte represents 8 vertical pixels

**Example:**
```
Page 0: ████████ (top 8 rows)
Page 1: ████████
Page 2: ████████
...
Page 7: ████████ (bottom 8 rows)
```

This organization makes vertical lines and text efficient to render.

---

## Buzzer (Passive vs Active)

### Types

**Passive Buzzer:**

- Requires **frequency signal** to produce sound
- Can generate different **tones/melodies**
- Controlled with `tone()` function
- More versatile but slightly more complex
- **This is what we use**

**Active Buzzer:**

- Has internal oscillator
- Simple **ON/OFF control** (digitalWrite)
- Fixed frequency (~2kHz)
- Easier to use for simple beeps

### Our Implementation (Passive Buzzer)

```cpp
#define BUZZER_PIN 5

// Play tone at specific frequency
tone(BUZZER_PIN, 1000);    // 1000 Hz
delay(200);
noTone(BUZZER_PIN);

// Or with duration
tone(BUZZER_PIN, 1000, 200);  // 1000Hz for 200ms
```

### How Passive Buzzers Work

**Physical Principle:**
1. Electromagnetic coil inside
2. PWM signal creates alternating magnetic field
3. Membrane vibrates at signal frequency
4. Vibration produces sound waves

**Frequency Response:**
- **Low frequency** (200-500 Hz): Deep, bass tones
- **Mid frequency** (500-2000 Hz): Clear, audible
- **High frequency** (2000-8000 Hz): Piercing, loud
- **Above 8000 Hz**: Inaudible to most humans

### Audio Feedback Design in Our Project

**Access Granted (Welcome Melody):**
```cpp
void playAccessGranted() {
  tone(BUZZER_PIN, 1319, 60);  // E6 - cheerful
  delay(70);
  tone(BUZZER_PIN, 1568, 60);  // G6 - ascending
  delay(70);
  tone(BUZZER_PIN, 1976, 60);  // B6
  delay(70);
  tone(BUZZER_PIN, 2637, 100); // E7 - sparkle!
  delay(120);
  // ... (anime-style notification sound)
}
```

**Access Denied (Error Melody):**
```cpp
void playAccessDenied() {
  tone(BUZZER_PIN, 587, 200);  // D5 - sad start
  delay(220);
  tone(BUZZER_PIN, 494, 200);  // B4 - descending
  delay(220);
  tone(BUZZER_PIN, 440, 200);  // A4
  delay(220);
  tone(BUZZER_PIN, 330, 400);  // E4 - defeated
  // ... (sad descending tones)
}
```

### Musical Theory Applied

**Note Frequencies (A4 = 440 Hz standard):**
- C4: 262 Hz
- E4: 330 Hz
- G4: 392 Hz
- C5: 523 Hz
- E5: 659 Hz
- G5: 784 Hz

**Our Melodies Use:**
- **Ascending tones** = positive, happy (access granted)
- **Descending tones** = negative, sad (access denied)
- **Major chords** = pleasant, resolved
- **Minor intervals** = tense, unresolved

---

## Arduino UNO

### Role

Acts as the **microcontroller brain** coordinating all components:

- Reads RFID data via SPI
- Controls OLED via I2C
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
| 5 | Buzzer | Digital Out |
| 6 | Servo control | PWM |
| 9 | RFID RST | Digital Out |
| 10 | RFID SS/SDA | SPI |
| 11 | RFID MOSI | SPI |
| 12 | RFID MISO | SPI |
| 13 | RFID SCK | SPI |
| A4 | OLED SDA | I2C |
| A5 | OLED SCL | I2C |

### Memory Management

```cpp
String uidString = "";              // Uses heap memory (avoid!)
const char* cardName = "Admin";     // Stored in flash (good!)
byte buffer[18];                    // Stack allocation (temporary)
```

**Best practices:**

- Use `const` for strings stored in flash (saves SRAM)
- Avoid `String` class for memory-constrained projects
- Use `F()` macro for serial strings: `Serial.println(F("Text"));`
- Use character arrays instead of String objects

**Why this matters:**

With only 2KB SRAM and OLED using 1KB for framebuffer, we have ~1KB left for:
- Stack (function calls)
- Global variables
- Local variables
- Library overhead

Careful memory management prevents crashes and unexpected behavior.

---

## Communication Protocols

### SPI (Serial Peripheral Interface)

**Used by:** RFID RC522

**Characteristics:**

- **Full-duplex** - simultaneous send/receive
- **High speed** - up to 10 Mbps
- **Master-slave** architecture
- **4-wire interface** - MOSI, MISO, SCK, SS

**How it works:**

1. Master (Arduino) pulls SS LOW to select slave (RC522)
2. Master generates clock signal on SCK
3. Data sent on MOSI (Master → Slave)
4. Data received on MISO (Slave → Master)
5. Both happen simultaneously (full-duplex)
6. Master pulls SS HIGH to deselect

**Advantages:**

- Fast data transfer (critical for RFID reading)
- Simple hardware implementation
- No addressing needed (SS pin selects device)
- Supports multiple slaves on same bus

**Disadvantages:**

- Requires 4+ pins per device
- No built-in error checking
- One master only (usually)

### I2C (Inter-Integrated Circuit)

**Used by:** OLED SSD1306

**Characteristics:**

- **Half-duplex** - one direction at a time
- **Moderate speed** - 100 kHz (standard) or 400 kHz (fast)
- **Multi-master, multi-slave** capable
- **2-wire interface** - SDA, SCL

**How it works:**

1. Master sends START condition
2. Master sends 7-bit slave address + R/W bit
3. Addressed slave sends ACK
4. Data transfer (one byte at a time)
5. Each byte followed by ACK/NACK
6. Master sends STOP condition

**Pull-up Resistors:**

Both SDA and SCL require pull-up resistors (typically 4.7kΩ):
- Lines are normally HIGH (pulled up to VCC)
- Devices pull lines LOW to communicate
- This is called "open-drain" or "open-collector"

Most I2C modules (including our OLED) have built-in pull-ups.

**Advantages:**

- Minimal pin usage (only 2 pins for many devices)
- Built-in addressing (up to 127 devices)
- Error checking via ACK/NACK
- Standardized protocol

**Disadvantages:**

- Slower than SPI
- More complex protocol
- Limited by capacitance (wire length ~30cm max)

### PWM (Pulse Width Modulation)

**Used by:** Servo motor

**Principle:**

PWM creates analog-like behavior from digital signals by rapidly switching between HIGH and LOW.

**Key Parameters:**

- **Frequency:** How many pulses per second (Hz)
- **Duty cycle:** Percentage of time signal is HIGH
- **Pulse width:** Duration of HIGH state

**For Servos:**

- Frequency: 50 Hz (20ms period)
- Pulse width: 1-2ms
  - 1.0ms = 0°
  - 1.5ms = 90°
  - 2.0ms = 180°

**How it controls servo angle:**

1. Servo's control circuit measures pulse width
2. Compares with potentiometer position
3. Drives motor until position matches desired angle
4. Holds position with continuous corrections

**Arduino PWM:**

- Pins 3, 5, 6, 9, 10, 11 support PWM
- `analogWrite(pin, 0-255)` sets duty cycle
- `Servo` library handles timing automatically for servo control

**Example:**

```cpp
analogWrite(6, 128);  // 50% duty cycle (128/255)
// For LEDs, motors, etc.

myservo.write(90);    // Servo library handles PWM
// For servo motors
```

### UART (Universal Asynchronous Receiver-Transmitter)

**Used by:** Serial communication (USB to computer)

**Characteristics:**

- **Asynchronous** - no shared clock
- **Point-to-point** - one sender, one receiver
- **2-wire** - TX (transmit), RX (receive)

**Settings:**

- **Baud rate:** 9600 bps (bits per second) in our project
- **Data bits:** 8
- **Parity:** None
- **Stop bits:** 1

**Our Usage:**

```cpp
Serial.begin(9600);
Serial.println("Access Granted!");
```

Used for debugging and logging to Serial Monitor.

---

## Security Considerations

### Current Implementation

- **UID-based authentication** - simple but vulnerable
- **Hardcoded card list** - limited flexibility
- **No encryption** - UID transmitted in plaintext

### Vulnerabilities

1. **UID cloning** - RFID UIDs can be copied with ~$50 devices
2. **Replay attacks** - captured signals can be replayed
3. **Physical access** - exposed Arduino can be reprogrammed
4. **No audit trail** - access attempts not permanently logged

### RFID Security Levels

**Level 1: UID only (our current implementation)**
- Reads unique ID from read-only memory
- Cannot be changed
- Easily cloned
- Suitable for: Education, low-security hobby projects

**Level 2: UID + Memory blocks**
- Uses writable memory sectors on card
- Can store encrypted data
- Requires correct key to access
- Suitable for: Home security, small businesses

**Level 3: Challenge-response**
- Card proves it knows secret without revealing it
- Uses cryptographic algorithms
- Clone-resistant
- Suitable for: Commercial security, payment systems

### Improvements for Higher Security

1. **Challenge-Response Authentication:**
```cpp
// Pseudocode
byte challenge[16];
generateRandomChallenge(challenge);
byte response[16];
mfrc522.readAuthenticatedBlock(4, response, challenge);
if (verifyResponse(challenge, response)) {
  // Grant access
}
```

2. **Two-Factor Authentication:**
- RFID card (something you have)
- PIN code (something you know)
- Requires keypad addition

3. **Time-Based Access:**
- RTC module for timestamp
- Allow access only during specific hours
- Log all attempts with timestamps

4. **Tamper Detection:**
- Magnetic sensor on enclosure
- Alert if case opened
- Disable system until authorized reset

5. **Network Monitoring:**
- WiFi/Ethernet module
- Remote monitoring and alerts
- Cloud-based authentication
- Centralized logging

---

## System Integration Concepts

### Event-Driven Programming

Our system follows event-driven architecture:

1. **Wait** for event (card detection)
2. **Read** event data (UID)
3. **Process** event (check authorization)
4. **Respond** to event (grant/deny access)
5. **Return** to waiting state

This differs from sequential programming where actions happen in fixed order.

### State Machine

The system operates as a finite state machine:

- **States:** IDLE, READING, AUTHORIZED, DENIED, UNLOCKING, LOCKING
- **Transitions:** Triggered by events (card detected, timer expired)
- **Actions:** Associated with states or transitions

### Real-Time Constraints

- **Hard real-time:** Must respond within deadline (not our case)
- **Soft real-time:** Should respond quickly but not critical (our case)

Our system has soft real-time requirements:
- User expects <1 second response to card scan
- But nothing breaks if it takes 2 seconds

### Resource Constraints

Embedded systems must work within limits:

- **Processing:** 16 MHz CPU (slow by modern standards)
- **Memory:** 2 KB RAM (tiny!)
- **Power:** Limited by USB or battery
- **Size:** Must fit in enclosure

These constraints drive design decisions:
- Use efficient algorithms
- Minimize memory allocation
- Optimize for size, not features
- Choose appropriate data structures

---

## Key Takeaways

- **Modular hardware** - Each component serves specific purpose
- **Multiple protocols** - SPI, I2C, PWM, UART working together
- **Event-driven logic** - React to RFID detection
- **User feedback** - Visual (OLED), audio (buzzer), physical (servo)
- **Scalability** - Easy to add more features (keypad, RTC, network)
- **Memory efficiency** - Critical with only 2KB SRAM
- **Security awareness** - Understand vulnerabilities and improvements

This project teaches practical embedded systems development:

- Peripheral interfacing
- Protocol implementation
- Real-time response
- User experience design
- Security fundamentals
- Resource management

**OLED Display adds:**
- Rich visual feedback (graphics, emojis, animations)
- Intuitive user interface
- Professional appearance
- Educational value (I2C protocol, memory management)
- Only 2 additional pins required vs LCD's 6-12 pins