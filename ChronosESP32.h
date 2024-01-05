/*
   MIT License

  Copyright (c) 2023 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ______________  _____
  ___  __/___  /_ ___(_)_____ _______ _______
  __  /_  __  __ \__  / _  _ \__  __ `/_  __ \
  _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
  /_/     /_.___/ /_/   \___/ _\__, /  \____/
							  /____/

*/

#ifndef CHRONOSESP32_H
#define CHRONOSESP32_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ESP32Time.h>

#define NOTIF_SIZE 10
#define WEATHER_SIZE 7
#define ALARM_SIZE 8
#define DATA_SIZE 512

#define MUSIC_PLAY 0x9D00
#define MUSIC_PAUSE 0x9D01
#define MUSIC_PREVIOUS 0x9D02
#define MUSIC_NEXT 0x9D03
#define MUSIC_TOGGLE 0x9900

#define VOLUME_UP	0x99A1
#define VOLUME_DOWN	0x99A2
#define VOLUME_MUTE	0x99A3

struct Notification
{
	int icon;
	String app;
	String time;
	String message;
};

struct Weather
{
	int icon;
	int day;
	int temp;
	int high;
	int low;
};

struct ChronosTimer
{
	unsigned long time;
	long duration = 5000;
	bool active;
};

struct ChronosData
{
	int length;
	uint8_t data[DATA_SIZE];
};

struct Alarm
{
	uint8_t hour;
	uint8_t minute;
	uint8_t repeat;
	bool enabled;
};

struct Setting
{
	uint8_t hour;
	uint8_t minute;
	uint8_t repeat;
	bool enabled;
};

enum Config
{
	CF_TIME = 0, // time -
	CF_RTW,		 // raise to wake  -
	CF_HR24,	 // 24 hour mode -
	CF_LANG,	 // watch language -
	CF_RST,		 // watch reset -
	CF_CLR,		 // watch clear data
	CF_HOURLY,	 // hour measurement -
	CF_FIND,	 // find watch -
	CF_USER,	 // user details (age)(height)(weight)(step length)(target)(units[])
	CF_ALARM,	 // alarm (index)(hour) (minute) (enabled) (repeat) -
	CF_FONT,	 // font settings (color[3])(b1+b2) -
	CF_SED,		 // sedentary (hour)(minute)(hour)(minute)(interval)(enabled) -
	CF_SLEEP,	 // sleep time (hour)(minute)(hour)(minute)(enabled) -
	CF_QUIET,	 // quiet hours (hour)(minute)(hour)(minute)(enabled) -
	CF_WATER,	 // water reminder (hour)(minute)(hour)(minute)(interval)(enabled)-
	CF_WEATHER,	 // weather config (a Weekly) (b City Name) -
	CF_CAMERA	 // camera config, (ready state)
};

/*
The screen configurations below is only used for identification on the Chronos app. 
Under the watch tab, when you click on watch info you can see the detected screen configuration.
The primary purpose of this configuration is to aid in loading watchfaces on supported watches with the correct resolution. 
ChronosESP32 library is implementing this for future development
*/
enum ChronosScreen
{
    // Resolution(240x240), Size in inches(1.3), Type(0 - Round [C], 1 - Square [S], 2 - Rectangular [R])
    CS_0x0_000_CFF = 0, // default no config
    CS_240x240_130_STF = 1,  // 240x240, 1.3 inches, Square, True, False
    CS_240x240_130_STT = 2,  // 240x240, 1.3 inches, Square, True, True
    CS_80x160_096_RTF = 3,   // 80x160, 0.96 inches, Rectangular, True, False
    CS_80x160_096_RTT = 4,   // 80x160, 0.96 inches, Rectangular, True, True
    CS_135x240_114_RTF = 5,  // 135x240, 1.14 inches, Rectangular, True, False
    CS_135x240_114_RTT = 6,  // 135x240, 1.14 inches, Rectangular, True, True
    CS_240x240_128_CTF = 7,  // 240x240, 1.28 inches, Round, True, False
    CS_240x240_128_CTT = 8,  // 240x240, 1.28 inches, Round, True, True
    CS_240x288_157_RTF = 9,  // 240x288, 1.57 inches, Rectangular, True, False
    CS_240x288_157_RTT = 10, // 240x288, 1.57 inches, Rectangular, True, True
    CS_240x283_172_RTF = 11, // 240x283, 1.72 inches, Rectangular, True, False
    CS_240x283_172_RTT = 12, // 240x283, 1.72 inches, Rectangular, True, True
    CS_360x360_130_CTF = 13, // 360x360, 1.3 inches, Round, True, False
    CS_360x360_130_CTT = 14, // 360x360, 1.3 inches, Round, True, True
    CS_320x380_177_RTF = 15, // 320x380, 1.77 inches, Rectangular, True, False
    CS_320x380_177_RTT = 16, // 320x380, 1.77 inches, Rectangular, True, True
    CS_320x385_175_RTF = 17, // 320x385, 1.75 inches, Rectangular, True, False
    CS_320x385_175_RTT = 18, // 320x385, 1.75 inches, Rectangular, True, True
    CS_320x360_160_RTF = 19, // 320x360, 1.6 inches, Rectangular, True, False
    CS_320x360_160_RTT = 20, // 320x360, 1.6 inches, Rectangular, True, True
    CS_240x296_191_RTF = 21, // 240x296, 1.91 inches, Rectangular, True, False
    CS_240x296_191_RTT = 22, // 240x296, 1.91 inches, Rectangular, True, True
    CS_412x412_145_CTF = 23, // 412x412, 1.45 inches, Round, True, False
    CS_412x412_145_CTT = 24, // 412x412, 1.45 inches, Round, True, True
    CS_410x494_200_RTF = 25, // 410x494, 2.0 inches, Rectangular, True, False
    CS_410x494_200_RTT = 32, // 410x494, 2.0 inches, Rectangular, True, True
    CS_466x466_143_CTF = 33, // 466x466, 1.43 inches, Round, True, False
    CS_466x466_143_CTT = 34  // 466x466, 1.43 inches, Round, True, True
};


class ChronosESP32 : public BLEServerCallbacks, public BLECharacteristicCallbacks, public ESP32Time
{

public:
	ChronosESP32();
	ChronosESP32(String name, ChronosScreen screen = CS_240x240_128_CTF);		 // set the BLE name
	void begin();				 // initializes BLE
	void loop();				 // handles routine functions

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

	// settings
	// isQuietActive
	// isSleepActive

	// alarms
	Alarm getAlarm(int index);
	void setAlarm(int index, Alarm alarm);
	// alarm active callback
	// isAlarmActive
	// getActiveAlarms

	// control
	void sendCommand(uint8_t *command, size_t length);
	void musicControl(uint16_t command);
	void setVolume(uint8_t level);
	bool capturePhoto();
	void findPhone(bool state);

	// helper functions for ESP32Time
	int getHourC();					   // return hour based on hour 24 variable
	String getHourZ();				   // return zero padded hour string based on hour 24 variable
	String getAmPmC(bool caps = true); // return (no caps)am/pm or (caps)AM/PM for 12 hour mode or none for 24 hour mode

	// callbacks
	void setConnectionCallback(void (*callback)(bool));
	void setNotificationCallback(void (*callback)(Notification));
	void setRingerCallback(void (*callback)(String, bool));
	void setConfigurationCallback(void (*callback)(Config, uint32_t, uint32_t));
	void setDataCallback(void (*callback)(uint8_t *, int));
	void setRawDataCallback(void (*callback)(uint8_t *, int));

private:
	String watchName = "Chronos ESP32";
	String address;
	uint8_t batteryLevel;
	bool isCharging;
	bool connected;
	bool batteryChanged;
	bool hour24;
	bool cameraReady;

	Notification notifications[NOTIF_SIZE];
	int notificationIndex;

	Weather weather[WEATHER_SIZE];
	String weatherCity;
	String weatherTime;
	int weatherSize;

	Alarm alarms[ALARM_SIZE];

	ChronosTimer infoTimer;
	ChronosTimer findTimer;
	ChronosTimer ringerTimer;

	ChronosData incomingData;

	ChronosScreen screenConf = CS_240x240_128_CTF;

	void (*connectionChangeCallback)(bool) = nullptr;
	void (*notificationReceivedCallback)(Notification) = nullptr;
	void (*ringerAlertCallback)(String, bool) = nullptr;
	void (*configurationReceivedCallback)(Config, uint32_t, uint32_t) = nullptr;
	void (*dataReceivedCallback)(uint8_t *, int) = nullptr;
	void (*rawDataReceivedCallback)(uint8_t *, int) = nullptr;

	void sendInfo();
	void sendBattery();

	String appName(int id);

	// from BLEServerCallbacks
	virtual void onConnect(BLEServer *pServer);
	virtual void onDisconnect(BLEServer *pServer);

	// from BLECharacteristicCallbacks
	virtual void onWrite(BLECharacteristic *pCharacteristic);

	void dataReceived();
	
};

#endif
