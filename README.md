# chronos-esp32
A wrapper library for ESP32 to facilitate easy setup of a smartwatch like project. Supports syncing of notifications from the phone.
Setup your ESP32 as a smartwatch and connect to Chronos app over BLE.

[![arduino-library-badge](https://www.ardu-badge.com/badge/ChronosESP32.svg?)](https://www.arduinolibraries.info/libraries/chronos-esp32)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/fbiego/library/ChronosESP32.svg)](https://registry.platformio.org/libraries/fbiego/ChronosESP32)


## Features

- Time
- Notifications
- Weather
- Controls (Music, Find Phone, Camera)
- Alarms (only received & stored)

## App

<a href='https://fbiego.com/chronos/app?id=esp32'><img alt='Download Chronos' height="100px" src='https://fbiego.com/chronos/img/chronos.png'/></a>
> Chronos

## Functions

```
ChronosESP32();
ChronosESP32(String name); // set the BLE name
ChronosESP32(String name, ChronosScreen screen); // set the BLE name and screen configuration
void begin(); // initializes BLE
void loop(); // handles routine functions

// watch
bool isConnected();
void set24Hour(bool mode);
bool is24Hour();
String getAddress();
void setBattery(uint8_t level, bool charging = false);
bool isCameraReady();

// notifications
int getNotificationCount();
Notification getNotificationAt(int index);
void clearNotifications();

// weather
int getWeatherCount();
String getWeatherCity();
String getWeatherTime();
Weather getWeatherAt(int index);

// alarms
Alarm getAlarm(int index);
void setAlarm(int index, Alarm alarm);

// control
void sendCommand(uint8_t *command, size_t length);
void musicControl(uint16_t command);
void setVolume(uint8_t level);
bool capturePhoto();
void findPhone(bool state);

// helper functions for ESP32Time
int getHourC(); // return hour based on hour 24 variable
String getHourZ(); // return zero padded hour string based on hour 24 variable
String getAmPmC(bool caps = true); // return (no caps)am/pm or (caps)AM/PM for 12 hour mode or none for 24 hour mode

// callbacks
void setConnectionCallback(void (*callback)(bool));
void setNotificationCallback(void (*callback)(Notification));
void setConfigurationCallback(void (*callback)(Config, uint32_t, uint32_t));
void setDataCallback(void (*callback)(uint8_t *, int));
void setRawDataCallback(void (*callback)(uint8_t *, int));
```

## Dependencies
- [`ESP32Time`](https://github.com/fbiego/ESP32Time)
- [`NimBLE-Arduino`](https://github.com/h2zero/NimBLE-Arduino)
