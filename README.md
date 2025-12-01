# RFID Access Control System

College educational project for Arduino UNO: a user authorization system using an RFID card and a PIN keypad.

Made as final project for microcontrollers assigment as two person team.

## CI/CD

This repo also has simple workflows to watch over code and to give us intel if it is healthy.

- simple compile with Arduino CLI (composite Action) on main/PR
- release on tag push (with 'v') to GH Realeases

### Status

[![CI](https://github.com/JustFiesta/arduino-rfid-access-control/actions/workflows/ci.yml/badge.svg)](https://github.com/JustFiesta/arduino-rfid-access-control/actions/workflows/ci.yml) [![CD](https://github.com/JustFiesta/arduino-rfid-access-control/actions/workflows/cd.yaml/badge.svg)](https://github.com/JustFiesta/arduino-rfid-access-control/actions/workflows/cd.yaml)

## Components used

- Arduino UNO-like board (r3)
- RFID RC522 Module
- LCD Display with I2C conventer module
- Buzzer
- LED (Green/Red)
- Servo

## Functionality

- user presents an RFID card.
- board reads the card's UID and verifies it against a stored credentials.
- if the card is recognized, the system opens servo with LCD info.
- LCD displays system status messages throughout the process.

## Used Arduino IDE libraries

Check libraries-list.txt file.

## How to run

1. Clone repo
2. Open the .ino file in Arduino IDE.
3. Upload the code to your Arduino board.
4. Connect all components according to the wiring diagram in /docs/wiring_diagram.png.

## Made by

[JustFiesta](https://github.com/JustFiesta/)  
[gubbl3bum](https://github.com/gubbl3bum/)
