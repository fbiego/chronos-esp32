# ChronosESP32 API Reference

ChronosESP32 lets an ESP32 act like a smartwatch and communicate with the Chronos companion app over BLE. The library uses the Arduino framework, NimBLE-Arduino, and ESP32Time.

Include the library:

```cpp
#include <ChronosESP32.h>
```

Create a watch object:

```cpp
ChronosESP32 watch;
ChronosESP32 namedWatch("Chronos Watch");
ChronosESP32 configuredWatch("Chronos Watch", CS_360x360_130_CTF);
```

Call `begin()` once in `setup()`, then call `loop()` repeatedly from `loop()`.

```cpp
void setup() {
  Serial.begin(115200);

  watch.setConnectionCallback(connectionCallback);
  watch.setNotificationCallback(notificationCallback);
  watch.setConfigurationCallback(configCallback);

  watch.begin();
  watch.setBattery(80);
}

void loop() {
  watch.loop();
}
```

Pair from the Chronos app, not from the phone Bluetooth settings:

Chronos app > Watches tab > Watches button > Pair New Devices > Search > select your board.

## Version

The public header defines:

```cpp
CS_VERSION_MAJOR 1
CS_VERSION_MINOR 9
CS_VERSION_PATCH 1
```

## Constants

| Constant | Value | Meaning |
| --- | ---: | --- |
| `CS_NOTIF_SIZE` | 10 | Notification ring buffer size. Define this before including the header, or pass it as a build flag, to choose a different size. |
| `CS_WEATHER_SIZE` | 7 | Daily weather forecast slots |
| `CS_ALARM_SIZE` | 8 | Alarm slots |
| `CS_DATA_SIZE` | 512 | Internal packet buffer size |
| `CS_FORECAST_SIZE` | 24 | Hourly forecast slots |
| `CS_QR_SIZE` | 9 | QR link slots |
| `CS_ICON_SIZE` | 48 | Navigation icon width and height |
| `CS_ICON_DATA_SIZE` | 288 | 48x48 1-bit icon byte count |
| `CS_CONTACTS_SIZE` | 255 | Contact slots |

To override the notification buffer size:

```cpp
#define CS_NOTIF_SIZE 20
#include <ChronosESP32.h>
```

With PlatformIO, you can also set it in `platformio.ini`:

```ini
build_flags = -D CS_NOTIF_SIZE=20
```

BLE UUIDs:

```cpp
CS_SERVICE_UUID           "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
CS_CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
CS_CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
```

## Data Types

### `Notification`

```cpp
struct Notification {
  int icon;
  String app;
  String time;
  String title;
  String message;
};
```

`getNotificationAt(0)` returns a reference to the latest notification. The buffer holds up to `CS_NOTIF_SIZE` notifications.

### `Weather`

```cpp
struct Weather {
  int icon;
  int day;
  int temp;
  int high;
  int low;
  int pressure;
  int uv;
};
```

`day` is day of week, `0` to `6`. Index `0` is today.

Weather icon codes:

| Code | Meaning |
| ---: | --- |
| 0 | Partly cloudy |
| 1 | Sunny |
| 2 | Snow |
| 3 | Rain |
| 4 | Cloudy |
| 5 | Tornado / extreme |
| 6 | Wind |
| 7 | Haze / fog |

### `WeatherLocation`

```cpp
struct WeatherLocation {
  String city;
  String region;
  String country;
  float latitude;
  float longitude;
};
```

### `HourlyForecast`

```cpp
struct HourlyForecast {
  int day;
  int hour;
  int icon;
  int temp;
  int uv;
  int humidity;
  int wind;
};
```

`wind` is in km/h. `humidity` is a percentage.

### `Alarm`

```cpp
struct Alarm {
  uint8_t hour;
  uint8_t minute;
  uint8_t repeat;
  bool enabled;
};
```

`repeat` uses bits for weekdays. Monday to Saturday use bits `0` to `5`; Sunday uses bit `6`. `0x80` and `0x7F` are treated as always active when the hour and minute match.

### `RemoteTouch`

```cpp
struct RemoteTouch {
  bool state;
  uint32_t x;
  uint32_t y;
};
```

### `Navigation`

```cpp
struct Navigation {
  bool active;
  bool isNavigation;
  bool hasIcon;
  String distance;
  String duration;
  String eta;
  String title;
  String directions;
  String speed;
  uint8_t icon[CS_ICON_DATA_SIZE];
  uint32_t iconCRC;
};
```

The icon is a 48x48 1-bit-per-pixel bitmap. Use `iconCRC` to detect when the icon has changed.

### `Contact`

```cpp
struct Contact {
  String name;
  String number;
};
```

### `DateTime`

```cpp
struct DateTime {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint32_t year;
};
```

Used by health record overloads.

### `PhoneInfo`

```cpp
struct PhoneInfo {
  bool isCharging;
  uint8_t batteryLevel;
  int appCode;
  int sdkVersion;
  String appVersion;
  String manufacturer;
  String model;
};
```

### `MusicInfo`

```cpp
struct MusicInfo {
  uint8_t state;
  uint32_t textColor;
  uint32_t backgroundColor;
  String title;
  String artist;
  String appName;
  String packageName;
};
```

## Enums

### `Control`

```cpp
MUSIC_PLAY
MUSIC_PAUSE
MUSIC_PREVIOUS
MUSIC_NEXT
MUSIC_TOGGLE
VOLUME_UP
VOLUME_DOWN
VOLUME_MUTE
```

Pass these to `musicControl()`.

### `SleepType`

```cpp
SLEEP_AWAKE = 0
SLEEP_LIGHT = 1
SLEEP_DEEP = 2
```

Pass these to `sendSleepRecord()`.

### `Config`

Configuration callback event types:

| Value | Meaning |
| --- | --- |
| `CF_TIME` | Time sync |
| `CF_RTW` | Raise to wake |
| `CF_HR24` | 24-hour mode |
| `CF_LANG` | Watch language |
| `CF_RST` | Reset request |
| `CF_CLR` | Clear data |
| `CF_HOURLY` | Hourly measurement |
| `CF_FIND` | Find watch request |
| `CF_USER` | User details |
| `CF_ALARM` | Alarm data |
| `CF_FONT` | Font settings |
| `CF_SED` | Sedentary reminder |
| `CF_SLEEP` | Sleep time |
| `CF_QUIET` | Quiet hours |
| `CF_WATER` | Water reminder |
| `CF_WEATHER` | Weather data |
| `CF_CAMERA` | Camera ready state |
| `CF_PBAT` | Phone battery |
| `CF_APP` | Chronos app version |
| `CF_QR` | QR links |
| `CF_NAV_DATA` | Navigation data |
| `CF_NAV_ICON` | Navigation icon data |
| `CF_CONTACT` | Contacts |
| `CF_SYNCED` | Data sync completed |
| `CF_MUSIC` | Music info |

### `HealthRequest`

```cpp
HR_STEPS_RECORDS
HR_SLEEP_RECORDS
HR_HEART_RATE_MEASURE
HR_BLOOD_OXYGEN_MEASURE
HR_BLOOD_PRESSURE_MEASURE
HR_MEASURE_ALL
```

These values are passed to `setHealthRequestCallback()`.

### `ChronosScreen`

`ChronosScreen` identifies the watch screen to the Chronos app. It helps the app choose compatible watchfaces.

Examples:

```cpp
CS_240x240_130_STF
CS_240x240_128_CTF
CS_360x360_130_CTF
CF_ESP32_240x240
CF_ESP32_466x466
CF_WATCHY_200x200
```

See `src/ChronosESP32.h` for the full list.

## Class API

### Lifecycle

```cpp
ChronosESP32();
ChronosESP32(String name, ChronosScreen screen = CF_ESP32_240x240);
void begin();
void stop(bool clearAll = true);
void loop();
bool isRunning();
void setName(String name);
void setScreen(ChronosScreen screen);
void setChunkedTransfer(bool chunked);
bool isSubscribed();
```

`setName()` and `setScreen()` should be called before `begin()`. `loop()` handles delayed info sync, battery sync, find-phone timeout, and other internal work.

`stop(clearAll)` calls `BLEDevice::deinit(clearAll)`.

`setChunkedTransfer(true)` enables splitting outgoing packets larger than 20 bytes. The app can also configure this automatically.

### Watch State

```cpp
bool isConnected();
void set24Hour(bool mode);
bool is24Hour();
String getAddress();
void setBattery(uint8_t level, bool charging = false);
bool isCameraReady();
void syncRequest();
```

`getAddress()` is available after `begin()`.

`setBattery()` updates the watch battery level sent to the app. The value is sent from `loop()` when connected.

`syncRequest()` asks the app to sync settings.

### Notifications

```cpp
int getNotificationCount();
Notification &getNotificationAt(int index);
void clearNotifications();
```

Index `0` is the latest notification. `clearNotifications()` resets the count to zero, but old buffer data remains until overwritten.

### Weather

```cpp
int getWeatherCount();
String getWeatherCity();
String getWeatherTime();
Weather &getWeatherAt(int index);
HourlyForecast &getForecastHour(int hour);
WeatherLocation &getWeatherLocation();
```

`getWeatherAt(index)` wraps by `CS_WEATHER_SIZE`. `getForecastHour(hour)` wraps by `CS_FORECAST_SIZE`.

### Extras

```cpp
RemoteTouch &getTouch();
String getQrAt(int index);
void setQr(int index, String qr);
```

`getQrAt()` and `setQr()` wrap by `CS_QR_SIZE`.

### Alarms

```cpp
Alarm &getAlarm(int index);
void setAlarm(int index, Alarm alarm);
bool isAlarmActive(int index);
bool isAlarmActive(Alarm alarm);
bool isAnyAlarmActive();
int getActiveAlarms(Alarm *alarms, int maxCount = CS_ALARM_SIZE);
```

Alarm indices wrap by `CS_ALARM_SIZE`. `getActiveAlarms()` writes matching active alarms into the caller-provided buffer and returns the number written. Alarms received from the app are stored in RAM; save them yourself if they must survive restart.

### Controls

```cpp
void sendCommand(uint8_t *command, size_t length, bool force_chunked = false);
void musicControl(Control command);
void setVolume(uint8_t level);
bool capturePhoto();
void findPhone(bool state);
```

`musicControl()` accepts `Control` values such as `MUSIC_TOGGLE` and `VOLUME_UP`.

`setVolume(level)` expects `0` to `100`.

`capturePhoto()` returns `false` if the camera is not ready in the Chronos app.

`findPhone(true)` starts ringing the phone. It automatically stops after about 30 seconds when `loop()` is running.

### Phone Battery

```cpp
void setNotifyBattery(bool state);
bool isPhoneCharging();
uint8_t getPhoneBattery();
```

Phone battery notifications are enabled by default. Phone battery status requires Chronos app support.

### App Info

```cpp
int getAppCode();
String getAppVersion();
PhoneInfo &getPhoneInfo();
```

Values are updated when the app sends `CF_APP`.

### Navigation

```cpp
Navigation &getNavigation();
const uint8_t *getNavigationIcon();
MusicInfo &getMusicInfo();
```

`getNavigation()` returns the stored navigation state. Navigation icon data arrives in chunks through `CF_NAV_ICON`.

Note: `getNavigationIcon()` is declared in the public header, but this repository version does not include its implementation.

### Contacts

```cpp
void setContact(int index, Contact contact);
Contact &getContact(int index);
int getContactCount();
Contact &getSoSContact();
void setSOSContactIndex(int index);
int getSOSContactIndex();
```

Contact indices wrap by `CS_CONTACTS_SIZE`.

### Health Data

Realtime values:

```cpp
void sendRealtimeSteps(uint32_t steps, uint32_t calories);
void sendRealtimeHeartRate(uint8_t heartRate);
void sendRealtimeBloodPressure(uint8_t systolic, uint8_t diastolic);
void sendRealtimeBloodOxygen(uint8_t bloodOxygen);
void sendRealtimeHealthData(uint8_t heartRate, uint8_t bloodOxygen, uint8_t systolic, uint8_t diastolic);
```

Historical records:

```cpp
void sendStepsRecord(uint32_t steps, uint32_t calories, uint8_t hour, uint8_t day, uint8_t month, uint32_t year, uint8_t heartRate = 0, uint8_t bloodOxygen = 0, uint8_t systolic = 0, uint8_t diastolic = 0);
void sendHeartRateRecord(uint8_t heartRate, uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint32_t year);
void sendBloodPressureRecord(uint8_t systolic, uint8_t diastolic, uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint32_t year);
void sendBloodOxygenRecord(uint8_t bloodOxygen, uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint32_t year);
void sendSleepRecord(uint16_t sleepTime, SleepType type, uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint32_t year);
void sendTemperatureRecord(float temperature, uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint32_t year);
```

`DateTime` overloads:

```cpp
void sendStepsRecord(uint32_t steps, uint32_t calories, DateTime dateTime, uint8_t heartRate = 0, uint8_t bloodOxygen = 0, uint8_t systolic = 0, uint8_t diastolic = 0);
void sendHeartRateRecord(uint8_t heartRate, DateTime dateTime);
void sendBloodPressureRecord(uint8_t systolic, uint8_t diastolic, DateTime dateTime);
void sendBloodOxygenRecord(uint8_t bloodOxygen, DateTime dateTime);
void sendTemperatureRecord(float temperature, DateTime dateTime);
void sendSleepRecord(uint16_t sleepTime, SleepType type, DateTime dateTime);
```

Use realtime health methods in response to `setHealthRequestCallback()`. Steps and calories records are typically grouped by hour and cumulative through the day.

### Time Helpers

ChronosESP32 inherits from `ESP32Time`, so ESP32Time methods such as `getTime()`, `getTimeDate()`, `getHour()`, `getMinute()`, `getDay()`, `getMonth()`, and `getYear()` are also available.

Convenience methods:

```cpp
int getHourC();
String getHourZ();
String getAmPmC(bool caps = true);
```

`getHourC()` and `getHourZ()` follow the current 12/24-hour mode. `getAmPmC()` returns an empty string in 24-hour mode.

## Callbacks

Register callbacks before `begin()`.

```cpp
void setConnectionCallback(void (*callback)(bool));
void setNotificationCallback(void (*callback)(Notification));
void setRingerCallback(void (*callback)(String, bool));
void setConfigurationCallback(void (*callback)(Config, uint32_t, uint32_t));
void setDataCallback(void (*callback)(uint8_t *, int));
void setRawDataCallback(void (*callback)(uint8_t *, int));
void setHealthRequestCallback(void (*callback)(HealthRequest, bool));
```

### Connection Callback

```cpp
void connectionCallback(bool state) {
  Serial.println(state ? "Connected" : "Disconnected");
}
```

Called when the phone connects or disconnects.

### Notification Callback

```cpp
void notificationCallback(Notification notification) {
  Serial.println(notification.app);
  Serial.println(notification.title);
  Serial.println(notification.message);
}
```

Called when a notification is received.

### Ringer Callback

```cpp
void ringerCallback(String caller, bool state) {
  Serial.println(state ? "Incoming call" : "Ringer dismissed");
}
```

`state == true` means an incoming call is active. `state == false` means it was dismissed.

### Configuration Callback

```cpp
void configCallback(Config config, uint32_t a, uint32_t b) {
  switch (config) {
    case CF_TIME:
      Serial.println("Time updated");
      break;
    case CF_CAMERA:
      Serial.println(b ? "Camera active" : "Camera inactive");
      break;
  }
}
```

Many configuration events pack their details into `a` and `b`:

| Event | `a` represents | `b` represents |
| --- | --- | --- |
| `CF_TIME` | `0` before setting the RTC, `1` after setting it | `0` |
| `CF_RTW` | `0` | Raise-to-wake state |
| `CF_HR24` | `0` | 24-hour mode state, where `1` means 24-hour mode |
| `CF_LANG` | `0` | Language id |
| `CF_RST` | `0` | `0` |
| `CF_CLR` | Not emitted by the current parser | Not emitted by the current parser |
| `CF_HOURLY` | `0` | Hourly measurement state |
| `CF_FIND` | `0` | `0` |
| `CF_USER` | Age, height, weight, and step length packed into bytes 3..0 | Units, target steps divided by 1000, temperature unit, and step length packed into bytes 3..0 |
| `CF_ALARM` | Alarm index | Hour, minute, repeat mask, and enabled state packed into bytes 3..0 |
| `CF_FONT` | RGB color as `0xRRGGBB` | Style in the high 16 bits, position in the low 16 bits |
| `CF_SED` | Interval in the high 16 bits, enabled state in the low byte | Start hour, start minute, end hour, and end minute packed into bytes 3..0 |
| `CF_SLEEP` | Enabled state | Start hour, start minute, end hour, and end minute packed into bytes 3..0 |
| `CF_QUIET` | Enabled state | Start hour, start minute, end hour, and end minute packed into bytes 3..0 |
| `CF_WATER` | Interval in the high 16 bits, enabled state in the low byte | Start hour, start minute, end hour, and end minute packed into bytes 3..0 |
| `CF_WEATHER` | Update type: `1` daily temperatures, `2` high/low temperatures, `0` city/location update | `1` when city/location changed, otherwise `0` |
| `CF_CAMERA` | `0` | Camera ready state |
| `CF_PBAT` | Phone charging state | Phone battery level |
| `CF_APP` | App version code when `b == 0`, SDK version when `b == 1` | App info type: `0` app version, `1` phone SDK/device info |
| `CF_QR` | `0` while receiving a link, `1` when all links are complete | Link index while receiving, link count when complete |
| `CF_NAV_DATA` | Navigation active state | `0` |
| `CF_NAV_ICON` | Icon chunk position | Icon CRC |
| `CF_CONTACT` | `0` when contact metadata starts, `1` when contact transfer completes | SOS index in the high byte, contact count in the low byte |
| `CF_SYNCED` | `0` | `0` |
| `CF_MUSIC` | Music info type: `0` app/status/colors, `1` title, `2` artist | Current music playback state |

### Data Callback

```cpp
void dataCallback(uint8_t *data, int length) {
  for (int i = 0; i < length; i++) {
    Serial.printf("%02X ", data[i]);
  }
  Serial.println();
}
```

Called after chunked incoming packets have been assembled. Packets generally start with `0xAB` or `0xEA`.

### Raw Data Callback

Called for every raw BLE write before packet assembly.

### Health Request Callback

```cpp
void healthRequestCallback(HealthRequest request, bool state) {
  switch (request) {
    case HR_STEPS_RECORDS:
      // send historical step records
      break;
    case HR_SLEEP_RECORDS:
      // send historical sleep records
      break;
    case HR_HEART_RATE_MEASURE:
    case HR_BLOOD_OXYGEN_MEASURE:
    case HR_BLOOD_PRESSURE_MEASURE:
    case HR_MEASURE_ALL:
      // state true means start, false means stop
      break;
  }
}
```

## Common Patterns

### Show Connection State

```cpp
digitalWrite(LED_PIN, watch.isConnected());
```

### Display Time

```cpp
String time = String(watch.getHourC()) + watch.getTime(":%M ") + watch.getAmPmC();
Serial.println(time);
```

### Send Music Control

```cpp
watch.musicControl(MUSIC_TOGGLE);
watch.musicControl(MUSIC_NEXT);
watch.setVolume(50);
```

### Capture Photo

```cpp
if (watch.capturePhoto()) {
  Serial.println("Capture command sent");
} else {
  Serial.println("Camera not ready");
}
```

### Read Notifications

```cpp
int count = watch.getNotificationCount();
for (int i = 0; i < count; i++) {
  Notification &n = watch.getNotificationAt(i);
  Serial.println(n.title);
  Serial.println(n.message);
}
```

### Read Weather

```cpp
int count = watch.getWeatherCount();
for (int i = 0; i < count; i++) {
  Weather &w = watch.getWeatherAt(i);
  Serial.printf("Day %d: %d C\n", w.day, w.temp);
}
```

### Read Navigation

```cpp
Navigation &nav = watch.getNavigation();
if (nav.active) {
  Serial.println(nav.title);
  Serial.println(nav.directions);
  Serial.println(nav.distance);
  Serial.println(nav.duration);
}
```

### Draw Navigation Icon

```cpp
Navigation &nav = watch.getNavigation();

for (int y = 0; y < 48; y++) {
  for (int x = 0; x < 48; x++) {
    int byteIndex = (y * 48 + x) / 8;
    int bitPos = 7 - (x % 8);
    bool pixelOn = (nav.icon[byteIndex] >> bitPos) & 0x01;
    // tft.drawPixel(x, y, pixelOn ? TFT_WHITE : TFT_BLACK);
  }
}
```

## Notes

- Register callbacks before `begin()`.
- Keep calling `watch.loop()`; several outgoing updates and timeouts depend on it.
- Call `setName()` and `setScreen()` before `begin()`.
- `getAddress()` is populated after `begin()`.
- Several indexed getters wrap using modulo, so out-of-range indices do not fail.
- The library stores notifications, weather, alarms, contacts, QR links, and navigation state in RAM.
- On disconnect, advertising restarts and camera state is reset to inactive.
