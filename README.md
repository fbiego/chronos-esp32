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

## Functions

```
// library
ChronosESP32();
ChronosESP32(String name, ChronosScreen screen = CS_240x240_128_CTF); // set the BLE name
void begin();														  // initializes BLE server
void stop(bool clearAll = true);									  // stop the BLE server
void loop();														  // handles routine functions
bool isRunning();													  // check whether BLE server is inited and running
void setName(String name);											  // set the BLE name (call before begin)
void setScreen(ChronosScreen screen);								  // set the screen config (call before begin)
void setChunkedTransfer(bool chunked);
bool isSubscribed();

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
HourlyForecast getForecastHour(int hour);

// extras
RemoteTouch getTouch();
String getQrAt(int index);
void setQr(int index, String qr);

// alarms
Alarm getAlarm(int index);
void setAlarm(int index, Alarm alarm);

// control
void sendCommand(uint8_t *command, size_t length);
void musicControl(Control command);
void setVolume(uint8_t level);
bool capturePhoto();
void findPhone(bool state);

// phone battery status
void setNotifyBattery(bool state);
bool isPhoneCharging();
uint8_t getPhoneBattery();

// app info
int getAppCode();
String getAppVersion();

// navigation
Navigation getNavigation();

// contacts
void setContact(int index, Contact contact);
Contact getContact(int index);
int getContactCount();
Contact getSoSContact();
void setSOSContactIndex(int index);
int getSOSContactIndex();

// helper functions for ESP32Time
int getHourC();					   // return hour based on 24-hour variable (0-12 or 0-23)
String getHourZ();				   // return zero padded hour string based on 24-hour variable (00-12 or 00-23)
String getAmPmC(bool caps = true); // return (no caps)am/pm or (caps)AM/PM for 12 hour mode or none for 24 hour mode

// callbacks
void setConnectionCallback(void (*callback)(bool));
void setNotificationCallback(void (*callback)(Notification));
void setRingerCallback(void (*callback)(String, bool));
void setConfigurationCallback(void (*callback)(Config, uint32_t, uint32_t));
void setDataCallback(void (*callback)(uint8_t *, int));
void setRawDataCallback(void (*callback)(uint8_t *, int));
```

## PlatformIO

Open the project folder in VS Code with PlatformIO installed to directly run the example sketches. This makes it easier to develop and test features

## Dependencies
- [`ESP32Time`](https://github.com/fbiego/ESP32Time)
- [`NimBLE-Arduino`](https://github.com/h2zero/NimBLE-Arduino)
