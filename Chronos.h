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

#ifndef CHRONOS_H
#define CHRONOS_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ESP32Time.h>

#define NOTIF_SIZE 10
#define WEATHER_SIZE 7
#define ALARM_SIZE 8

#define MUSIC_PLAY 0x9D00
#define MUSIC_PAUSE 0x9D01
#define MUSIC_PREVIOUS 0x9D02
#define MUSIC_NEXT 0x9D03
#define MUSIC_TOGGLE 0x9900

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
};

struct ChronosTimer
{
	unsigned long time;
	long duration = 5000;
	bool active;
};

struct Alarm
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
	CF_WEATHER	 // weather config (a Weekly) (b City Name) -
};

class Chronos : public BLEServerCallbacks, public BLECharacteristicCallbacks, public ESP32Time
{

public:
	Chronos();
	Chronos(String name);		 // set the BLE name
	void begin();				 // initializes BLE
	void loop();				 // handles routine functions
	void setLogging(bool state); // to view raw data receive over BLE

	// watch
	bool isConnected();
	void set24Hour(bool mode);
	bool is24Hour();
	String getAddress();
	void setBattery(uint8_t level);

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
	void findPhone(bool state);

	// helper functions for ESP32Time
	int getHourC();					   // return hour based on hour 24 variable
	String getHourZ();				   // return zero padded hour string based on hour 24 variable
	String getAmPmC(bool caps = true); // return (no caps)am/pm or (caps)AM/PM for 12 hour mode or none for 24 hour mode

	// callbacks
	void setConnectionCallback(void (*callback)(bool));
	void setNotificationCallback(void (*callback)(Notification));
	void setConfigurationCallback(void (*callback)(Config, uint32_t, uint32_t));

private:
	String watchName = "Chronos ESP32";
	String address;
	uint8_t batteryLevel;
	bool connected;
	bool batteryChanged;
	bool logging;
	bool hour24;

	Notification notifications[NOTIF_SIZE];
	int notificationIndex;
	int msgLen = 0;

	Weather weather[WEATHER_SIZE];
	String weatherCity;
	String weatherTime;
	int weatherSize;

	Alarm alarms[ALARM_SIZE];

	ChronosTimer infoTimer;
	ChronosTimer findTimer;

	void (*connectionChangeCallback)(bool) = nullptr;
	void (*notificationReceivedCallback)(Notification) = nullptr;
	void (*configurationReceivedCallback)(Config, uint32_t, uint32_t) = nullptr;

	void sendInfo();
	void sendBattery();

	String appName(int id);

	// from BLEServerCallbacks
	virtual void onConnect(BLEServer *pServer);
	virtual void onDisconnect(BLEServer *pServer);

	// from BLECharacteristicCallbacks
	virtual void onWrite(BLECharacteristic *pCharacteristic);
};

#endif
