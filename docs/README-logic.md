# System Logic – RFID Access Control

This document describes the logic flow of the Arduino-based RFID + PIN access control system.

---

## Overview

The system grants or denies access based on two-factor authentication:
1. **RFID card identification**
2. **PIN code verification**

It uses the following components:
- RFID module (RC522)
- 4x4 Keypad
- LCD I2C 1602
- LEDs (green/red)
- Buzzer
- Optional DS1302 RTC

---

## System Flow

### 1. Initialization
- Display “System Ready” or “Scan your card” on LCD.  
- Wait for an RFID tag to be detected.  
- Clear previous states (PIN buffer, flags, LEDs).

### 2. Card Scan
- Read the RFID card UID.  
- Compare it with the list of authorized UIDs stored in code or EEPROM.  

**If card is not recognized:**  
- Display “Access Denied”  
- Activate red LED and buzzer (long beep)  
- Return to idle state  

**If card is valid:**  
- Display “Enter PIN”  
- Proceed to PIN input mode  

### 3. PIN Entry
- Read numeric input from the 4x4 keypad.  
- Display `*` for each entered digit on LCD.  
- After a fixed number of digits (e.g., 4), check the PIN.

**If PIN is incorrect:**  
- Display “Wrong PIN”  
- Sound long beep + red LED  
- Return to idle state  

**If PIN is correct:**  
- Display “Access Granted”  
- Sound short beep + green LED  
- Optionally trigger relay to simulate door unlock  
- (Optional) Log timestamp using DS1302  

### 4. Idle Timeout
If no input is detected for a given time (e.g., 15 seconds), reset to the initial “Scan your card” state.

---

## Optional Enhancements
- **EEPROM storage** for dynamic user registration.  
- **RTC timestamps** for access logs.  
- **SD card or serial logging** for audit trails.  
- **Network module (ESP8266)** for online monitoring.  
- **Admin mode** for adding or removing cards.
