# Echo Stream ESP32

ESP32-S3 USB Audio Class 2 (UAC2) example project for streaming PCM audio to a host device.

## Overview

This repository demonstrates a minimal UAC2 pipeline using ESP-IDF and I2S input capture (PCM1808-style wiring).

## Hardware Focus

- Target MCU: ESP32-S3
- Audio path: I2S input to USB audio stream
- Example pin mapping is defined in `main/main.c`

## Build (ESP-IDF)

```bash
idf.py set-target esp32s3
idf.py build
```

## Flash

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

Adjust serial device path as needed.

## Project Layout

- `main/`: firmware source
- `managed_components/`: ESP-IDF managed dependencies
- `sdkconfig*`: project configuration
