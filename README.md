# chronos-esp32
A wrapper library for ESP32 to facilitate easy setup of a smartwatch like project. Supports syncing of notifications from the phone.
Setup your ESP32 as a smartwatch and connect to Chronos app over BLE.

[![arduino-library-badge](https://www.ardu-badge.com/badge/ChronosESP32.svg?)](https://www.arduinolibraries.info/libraries/chronos-esp32)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/fbiego/library/ChronosESP32.svg)](https://registry.platformio.org/libraries/fbiego/ChronosESP32)


## Features

- [x] Time (Auto sync via BLE)
- [x] Notifications (Receive notifications from connected phone)
- [x] Weather (Receive weather info)
- [x] Controls (Music, Find Phone, Camera)
- [x] Phone Battery (Level, Charging state) (Chronos app v3.5.1+)
- [x] Navigation (Chronos app v3.7.5+)
- [x] Contacts & QR codes
- [ ] Alarms

## Companion App

<a href='https://chronos.ke/app?id=esp32'><img alt='Download Chronos' height="100px" src='https://chronos.ke/img/chronos.png'/></a>

[`Chronos`](https://chronos.ke/app?id=esp32)

## API

See the [ChronosESP32 API reference](api.md).

## PlatformIO

Open the project folder in VS Code with PlatformIO installed to directly run the example sketches. This makes it easier to develop and test features

## Dependencies
- [`ESP32Time`](https://github.com/fbiego/ESP32Time)
- [`NimBLE-Arduino`](https://github.com/h2zero/NimBLE-Arduino)
