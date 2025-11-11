# RFID Access Control System

College educational project for Arduino UNO: a user authorization system using an RFID card and a PIN keypad.

Made as final project for microcontrollers assigment as two person team.

## Components used

- Arduino UNO-like board (r3)
- RFID RC522 Module
- 4x4 Keypad
- 1602 I2C LCD Display
- Buzzer
- LED (Green/Red)
- DS1302 RTC Module (Optional – for entry time logging)

## Functionality

- user presents an RFID card.
- board reads the card's UID and verifies it against a stored database.
- if the card is recognized, the system prompts for a PIN code via the keypad.
- correct PIN activates a relay (simulating a door unlock), while an incorrect one triggers an alarm.
- LCD displays system status messages throughout the process.

## Used Arduino IDE libraries

- MFRC522 (for RFID)
- Keypad
- LiquidCrystal_I2C

## How to run

1. Clone repo
2. Open the .ino file in Arduino IDE.
3. Upload the code to your Arduino board.
4. Connect all components according to the wiring diagram in /docs/wiring_diagram.png.

## Made by

[JustFiesta](https://github.com/JustFiesta/)
[gubbl3bum](https://github.com/gubbl3bum/)

## Possible future extensions

- Logging entry time to an SD card.
- Adding a web server (e.g., with ESP8266) for remote monitoring.
- Logging unauthorized access attempts.
