# Theoretical Background – RFID Access Control System

This document explains the core theory behind each module used in the Arduino-based RFID access control project. Understanding these concepts will help you develop, troubleshoot, and extend the system.

---

## RFID (Radio Frequency Identification)

**Principle:**  
RFID uses electromagnetic fields to identify and track tags attached to objects.  
The reader (RC522 module) sends a radio signal at 13.56 MHz, powering up the RFID tag (card or keychain).  
Each tag contains a **unique identifier (UID)** that can be read by Arduino and compared to an internal whitelist of authorized users.

**Key Components:**
- **RC522 Module:** Reads and writes data to 13.56 MHz RFID tags.
- **UID:** Unique hexadecimal identifier used for authentication.

**Arduino Library:** [`MFRC522`](https://github.com/miguelbalboa/rfid)

---

## 4x4 Keypad

**Function:**  
A 4x4 keypad consists of 16 buttons arranged in a 4-row × 4-column matrix.  
When a key is pressed, a connection between a row and a column is detected.  
This setup allows Arduino to identify which key was pressed.

**Use Case in Project:**  
After scanning a valid RFID card, the keypad is used to enter a PIN for two-step authentication.

**Arduino Library:** `Keypad.h`

---

## LCD 1602 with I2C Interface

**Purpose:**  
The LCD screen displays system messages such as “Scan your card” or “Enter PIN.”  
Using an I2C interface simplifies wiring to just 4 pins: VCC, GND, SDA, and SCL.

**Arduino Library:** `LiquidCrystal_I2C.h`

---

## DS1302 Real-Time Clock (RTC)

**Role (optional):**  
The DS1302 module keeps track of real-world time even when Arduino is powered off.  
It is useful for logging access timestamps or enforcing time-based access restrictions.

**Arduino Library:** `RTClib.h`

---

## Buzzer and LEDs

**Purpose:**  
Provide visual and auditory feedback:
- **Short beep + green LED:** Access granted  
- **Long beep + red LED:** Access denied  

**Connection:**  
Buzzer and LEDs are controlled via digital output pins.

---

## EEPROM (Optional Extension)

EEPROM allows the system to **store authorized card UIDs** and user PINs permanently, even after power loss.  
This enables offline functionality without reprogramming the Arduino.

---

## Arduino UNO

**Role:**  
Acts as the microcontroller brain of the system.  
It reads sensor inputs (RFID, keypad), performs authentication logic, and controls outputs (LCD, LEDs, buzzer).

**Key Concepts:**
- **Digital I/O:** Read and write HIGH/LOW signals.
- **Serial Communication:** Used for debugging and monitoring.
- **Libraries:** Extend Arduino with prebuilt drivers for peripherals.

---

## Recommended Learning Path

1. Understand **how RFID communication works** (MIFARE Classic standard).  
2. Practice **matrix keypad scanning** using the `Keypad` library.  
3. Learn to **display text and numbers** on an LCD via I2C.  
4. Combine multiple modules — manage timing, events, and serial debugging.  
5. Optionally, use EEPROM or SD card to log events and enhance persistence.  

- Modular software structure
- Integration of multiple I/O peripherals
