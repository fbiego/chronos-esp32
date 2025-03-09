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

#define CHRONOSESP_VERSION_MAJOR 1
#define CHRONOSESP_VERSION_MINOR 7
#define CHRONOSESP_VERSION_PATCH 0

#define CHRONOSESP_VERSION F(CHRONOSESP_VERSION_MAJOR "." CHRONOSESP_VERSION_MINOR "." CHRONOSESP_VERSION_PATCH)

#define NOTIF_SIZE 10
#define WEATHER_SIZE 7
#define ALARM_SIZE 8
#define DATA_SIZE 512
#define FORECAST_SIZE 24
#define QR_SIZE 9
#define ICON_SIZE 48
#define ICON_DATA_SIZE (ICON_SIZE * ICON_SIZE) / 8
#define CONTACTS_SIZE 255

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

enum Control
{
	MUSIC_PLAY = 0x9D00,
	MUSIC_PAUSE = 0x9D01,
	MUSIC_PREVIOUS = 0x9D02,
	MUSIC_NEXT = 0x9D03,
	MUSIC_TOGGLE = 0x9900,

	VOLUME_UP = 0x99A1,
	VOLUME_DOWN = 0x99A2,
	VOLUME_MUTE = 0x99A3,
};

struct Notification
{
	int icon;
	String app;
	String time;
	String title;
	String message;
};

struct Weather
{
	int icon;
	int day;
	int temp;
	int high;
	int low;
	int pressure;
	int uv;
};

struct HourlyForecast
{
	int day;  // day of forecast
	int hour; // hour of the forecast
	int icon;
	int temp;	  //
	int uv;		  // uv index
	int humidity; // %
	int wind;	  // wind speed km/h
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

struct RemoteTouch
{
	bool state;
	uint32_t x;
	uint32_t y;
};

struct Navigation
{
	bool active = false;		  // whether running or not
	bool isNavigation = false;	  // navigation or general info
	bool hasIcon = false;		  // icon present in the navigation data
	String distance;			  // distance to destination
	String duration;			  // time to destination
	String eta;					  // estimated time of arrival (time,date)
	String title;				  // distance to next point or title
	String directions;			  // place info ie current street name/ instructions
	uint8_t icon[ICON_DATA_SIZE]; // navigation icon 48x48 (1bpp)
	uint32_t iconCRC;			  // to identify whether the icon has changed
};

struct Contact
{
	String name;
	String number;
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
	CF_CAMERA,	 // camera config, (ready state)
	CF_PBAT,	 // phone battery ([a] isPhoneCharing, [b] phoneBatteryLevel)
	CF_APP,		 // app version info
	CF_QR,		 // qr codes received
	CF_NAV_DATA, // navigation data received
	CF_NAV_ICON, // navigation icon received
	CF_CONTACT,	 // contacts data received
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
	CS_0x0_000_CFF = 0,		 // default no config
	CS_240x240_130_STF = 1,	 // 240x240, 1.3 inches, Square, True, False
	CS_240x240_130_STT = 2,	 // 240x240, 1.3 inches, Square, True, True
	CS_80x160_096_RTF = 3,	 // 80x160, 0.96 inches, Rectangular, True, False
	CS_80x160_096_RTT = 4,	 // 80x160, 0.96 inches, Rectangular, True, True
	CS_135x240_114_RTF = 5,	 // 135x240, 1.14 inches, Rectangular, True, False
	CS_135x240_114_RTT = 6,	 // 135x240, 1.14 inches, Rectangular, True, True
	CS_240x240_128_CTF = 7,	 // 240x240, 1.28 inches, Round, True, False
	CS_240x240_128_CTT = 8,	 // 240x240, 1.28 inches, Round, True, True
	CS_240x288_157_RTF = 9,	 // 240x288, 1.57 inches, Rectangular, True, False
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
	CS_466x466_143_CTT = 34	 // 466x466, 1.43 inches, Round, True, True
};

class ChronosESP32 : public BLEServerCallbacks, public BLECharacteristicCallbacks, public ESP32Time
{

public:
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

	// TODO (settings)
	// isQuietActive
	// isSleepActive

	// alarms
	Alarm getAlarm(int index);
	void setAlarm(int index, Alarm alarm);
	// TODO (alarms)
	// alarm active callback
	// isAlarmActive
	// getActiveAlarms

	// control
	void sendCommand(uint8_t *command, size_t length, bool force_chunked = false);
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

private:
	String _watchName = "Chronos ESP32";
	String _address;
	bool _inited;
	bool _subscribed;
	uint8_t _batteryLevel;
	bool _isCharging;
	bool _connected;
	bool _batteryChanged;
	bool _hour24;
	bool _cameraReady;

	uint8_t _phoneBatteryLevel = 0;
	bool _phoneCharging;
	bool _notifyPhone = true;
	bool _sendESP;
	bool _chunked;

	Notification _notifications[NOTIF_SIZE];
	int _notificationIndex;

	Weather _weather[WEATHER_SIZE];
	String _weatherCity;
	String _weatherTime;
	int _weatherSize;

	HourlyForecast _hourlyForecast[FORECAST_SIZE];

	RemoteTouch _touch;

	int _appCode;
	String _appVersion;

	Alarm _alarms[ALARM_SIZE];

	String _qrLinks[QR_SIZE];

	Contact _contacts[CONTACTS_SIZE];
	int _sosContact;
	int _contactSize;

	ChronosTimer _infoTimer;
	ChronosTimer _findTimer;
	ChronosTimer _ringerTimer;

	ChronosData _incomingData;
	ChronosData _outgoingData;

	ChronosScreen _screenConf = CS_240x240_128_CTF;

	Navigation _navigation;

	void (*connectionChangeCallback)(bool) = nullptr;
	void (*notificationReceivedCallback)(Notification) = nullptr;
	void (*ringerAlertCallback)(String, bool) = nullptr;
	void (*configurationReceivedCallback)(Config, uint32_t, uint32_t) = nullptr;
	void (*dataReceivedCallback)(uint8_t *, int) = nullptr;
	void (*rawDataReceivedCallback)(uint8_t *, int) = nullptr;

	void sendInfo();
	void sendBattery();
	void sendESP();

	void splitTitle(const String &input, String &title, String &message, int icon);

	String appName(int id);
	String flashMode(FlashMode_t mode);

	// from BLEServerCallbacks
	virtual void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
	virtual void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;

	// from BLECharacteristicCallbacks
	virtual void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
	virtual void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override;

	void dataReceived();

	static BLECharacteristic *pCharacteristicTX;
	static BLECharacteristic *pCharacteristicRX;
};

#endif
