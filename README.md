# Arduino Electronic Keyboard PCB

This project is an Arduino-based electronic keyboard with a custom PCB layout and embedded audio synthesis code.

## Overview

The system uses 18 keys to control musical notes and generates audio through PWM output. The firmware implements four-channel synthesis, ADSR envelope control, and FM modulation to support multiple instrument timbres.

## Key Features

- 18-key note input using direct port reading for low-latency response
- 12 selectable instrument timbres
- Four-channel polyphonic sound synthesis
- ADSR envelope control for dynamic note shaping
- FM modulation for richer sound generation
- 9-bit fast PWM audio output using Timer1
- Custom PCB layout for the keyboard and control circuit

## Repository Contents

- `code/code.ino` - Arduino firmware for key scanning, synthesis, envelope control, and PWM audio output
- `电子琴.eprj` - JLC EDA project file
- `Screenshot 2026-10-01 173810.png` - PCB layout screenshot

## Tools Used

- Arduino IDE
- C/C++
- JLC EDA
- PCB layout and embedded systems design

## Skills Demonstrated

- Embedded programming on Arduino-compatible hardware
- Timer and PWM configuration
- Direct register-level GPIO operation
- PCB layout and routing
- Audio synthesis concepts including ADSR and FM modulation

## Preview

![PCB layout](./Screenshot%202026-10-01%20173810.png)
