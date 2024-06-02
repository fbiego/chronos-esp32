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

#include <Arduino.h>
#include "ChronosESP32.h"

BLECharacteristic *ChronosESP32::pCharacteristicTX;
BLECharacteristic *ChronosESP32::pCharacteristicRX;

/*!
	@brief  Constructor for ChronosESP32
*/
ChronosESP32::ChronosESP32()
{
	connected = false;
	cameraReady = false;
	batteryChanged = true;
	qrLinks[0] = "https://chronos.ke/";
}

/*!
	@brief  Constructor for ChronosESP32
	@param  name
			Bluetooth name
*/
ChronosESP32::ChronosESP32(String name, ChronosScreen screen)
{
	watchName = name;
	screenConf = screen;
	ChronosESP32();
}

/*!
	@brief  begin
*/
void ChronosESP32::begin()
{
	BLEDevice::init(watchName.c_str());
	BLEServer *pServer = BLEDevice::createServer();
	BLEDevice::setMTU(517);
	pServer->setCallbacks(this);

	BLEService *pService = pServer->createService(SERVICE_UUID);
	pCharacteristicTX = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);
	pCharacteristicRX = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
	pCharacteristicRX->setCallbacks(this);
	pCharacteristicTX->setCallbacks(this);
	pService->start();

	BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(SERVICE_UUID);
	pAdvertising->setScanResponse(true);
	pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
	pAdvertising->setMinPreferred(0x12);
	BLEDevice::startAdvertising();

	address = BLEDevice::getAddress().toString().c_str();

	notifications[0].icon = 0xC0;
	notifications[0].time = "Now";
	notifications[0].app = "Chronos";
	notifications[0].message = "Download from Google Play to sync time and receive notifications";

	findTimer.duration = 30 * 1000;	  // 30 seconds for find phone
	ringerTimer.duration = 30 * 1000; // 30 seconds for ringer alert
}

/*!
	@brief  loop
*/
void ChronosESP32::loop()
{
	if (connected)
	{
		if (infoTimer.active)
		{
			if (infoTimer.time + infoTimer.duration < millis())
			{
				// timer end
				infoTimer.active = false;

				sendInfo();
				sendBattery();
				setNotifyBattery(notifyPhone);
			}
		}
		if (findTimer.active)
		{
			if (findTimer.time + findTimer.duration < millis())
			{
				// timer end
				findTimer.active = false;
				findPhone(false); // auto cancel the command
			}
		}
		if (batteryChanged)
		{
			batteryChanged = false;
			sendBattery();
		}
	}

	if (ringerTimer.active)
	{
		if (ringerTimer.time + ringerTimer.duration < millis())
		{
			// ring timer end
			ringerTimer.active = false;
			if (ringerAlertCallback != nullptr)
			{
				ringerAlertCallback("", false);
			}
		}
	}
}

/*!
	@brief  check whether the device is connected
*/
bool ChronosESP32::isConnected()
{
	return connected;
}

/*!
	@brief  set the clock to 24 hour
	@param  mode
			enable or disable state
*/
void ChronosESP32::set24Hour(bool mode)
{
	hour24 = mode;
}

/*!
	@brief  return the 24 hour mode
*/
bool ChronosESP32::is24Hour()
{
	return hour24;
}

/*!
	@brief  return the mac address
*/
String ChronosESP32::getAddress()
{
	return address;
}

/*!
	@brief  set the battery level
	@param	level
			battery level
	@param	charging
			charging state
*/
void ChronosESP32::setBattery(uint8_t level, bool charging)
{
	if (batteryLevel != level || isCharging != charging)
	{
		batteryChanged = true;
		batteryLevel = level;
		isCharging = charging;
	}
}

/*!
	@brief  return camera status
*/
bool ChronosESP32::isCameraReady()
{
	return cameraReady;
}

/*!
	@brief  return the number of notifications in the buffer
*/
int ChronosESP32::getNotificationCount()
{
	if (notificationIndex + 1 >= NOTIF_SIZE)
	{
		return NOTIF_SIZE; // the buffer is full
	}
	else
	{
		return notificationIndex + 1; // the buffer is not full,
	}
}

/*!
	@brief  return the notification at the buffer index
	@param  index
			position of the notification to be returned, at 0 is the latest received
*/
Notification ChronosESP32::getNotificationAt(int index)
{
	int latestIndex = (notificationIndex - index + NOTIF_SIZE) % NOTIF_SIZE;
	return notifications[latestIndex];
}

/*!
	@brief  clear the notificaitons
*/
void ChronosESP32::clearNotifications()
{
	// here we just set the index to -1, existing data at the buffer will be overwritten
	// getNotificationCount() will return 0 but getNotificationAt() will return previous existing data
	notificationIndex = -1;
}

/*!
	@brief  return the weather count
*/
int ChronosESP32::getWeatherCount()
{
	return weatherSize;
}

/*!
	@brief  return the weather city name
*/
String ChronosESP32::getWeatherCity()
{
	return weatherCity;
}

/*!
	@brief  return the weather update time
*/
String ChronosESP32::getWeatherTime()
{
	return weatherTime;
}

/*!
	@brief  return the weather at the buffer index
	@param  index
			position of the weather to be returned
*/
Weather ChronosESP32::getWeatherAt(int index)
{
	return weather[index % WEATHER_SIZE];
}

/*!
	@brief  return the weatherforecast for the hour
	@param  hour
			position of the weather to be returned
*/
HourlyForecast ChronosESP32::getForecastHour(int hour)
{
	return hourlyForecast[hour % FORECAST_SIZE];
}

/*!
	@brief  get the alarm at the index
	@param	index
			position of the alarm to be returned
*/
Alarm ChronosESP32::getAlarm(int index)
{
	return alarms[index % ALARM_SIZE];
}

/*!
	@brief  set the alarm at the index
	@param  index
			position of the alarm to be set
	@param  alarm
			the alarm object
*/
void ChronosESP32::setAlarm(int index, Alarm alarm)
{
	alarms[index % ALARM_SIZE] = alarm;
}

/*!
	@brief  send a command to the app
	@param  command
			command data
	@param  length
			command length
*/
void ChronosESP32::sendCommand(uint8_t *command, size_t length)
{
	pCharacteristicTX->setValue(command, length);
	pCharacteristicTX->notify();
	vTaskDelay(200);
}

/*!
	@brief  send a music control command to the app
	@param  command
			music action
*/
void ChronosESP32::musicControl(uint16_t command)
{
	uint8_t musicCmd[] = {0xAB, 0x00, 0x04, 0xFF, (uint8_t)(command >> 8), 0x80, (uint8_t)(command)};
	sendCommand(musicCmd, 7);
}

/*!
	@brief  send a command to set the volume level
	@param  level
			volume level (0 - 100)
*/
void ChronosESP32::setVolume(uint8_t level)
{
	uint8_t volumeCmd[] = {0xAB, 0x00, 0x05, 0xFF, 0x99, 0x80, 0xA0, level};
	sendCommand(volumeCmd, 8);
}

/*!
	@brief  send capture photo command to the app
*/
bool ChronosESP32::capturePhoto()
{
	if (cameraReady)
	{
		uint8_t captureCmd[] = {0xAB, 0x00, 0x04, 0xFF, 0x79, 0x80, 0x01};
		sendCommand(captureCmd, 7);
	}
	return cameraReady;
}

/*!
	@brief  send a command to find the phone
	@param  state
			enable or disable state
*/
void ChronosESP32::findPhone(bool state)
{
	findTimer.active = state;
	if (state)
	{
		findTimer.time = millis();
	}
	uint8_t c = state ? 0x01 : 0x00;
	uint8_t findCmd[] = {0xAB, 0x00, 0x04, 0xFF, 0x7D, 0x80, c};
	sendCommand(findCmd, 7);
}

/*!
	@brief  get the hour based on hour 24 mode
*/
int ChronosESP32::getHourC()
{
	return this->getHour(hour24);
}

/*!
	@brief  get the zero padded hour based on hour 24 mode
*/
String ChronosESP32::getHourZ()
{
	if (hour24)
	{
		return this->getTime("%H");
	}
	else
	{
		return this->getTime("%I");
	}
}

/*!
	@brief  get the am pm label
	@param	caps
			capital letters mode
*/
String ChronosESP32::getAmPmC(bool caps)
{
	if (hour24)
	{
		return "";
	}
	else
	{
		return this->getAmPm(!caps); // esp32time is getAmPm(bool lowercase);
	}
	return "";
}


/*!
	@brief  get remote touch data
*/
RemoteTouch ChronosESP32::getTouch()
{
	return touch;
}

/*!
	@brief  get the qr link at the index
	@param	index
			position of the qr link to be returned
*/
String ChronosESP32::getQrAt(int index)
{
	return qrLinks[index % QR_SIZE];
}

/*!
	@brief  set the connection callback
*/
void ChronosESP32::setConnectionCallback(void (*callback)(bool))
{
	connectionChangeCallback = callback;
}

/*!
	@brief  set the notification callback
*/
void ChronosESP32::setNotificationCallback(void (*callback)(Notification))
{
	notificationReceivedCallback = callback;
}

/*!
	@brief  set the ringer callback
*/
void ChronosESP32::setRingerCallback(void (*callback)(String, bool))
{
	ringerAlertCallback = callback;
}

/*!
	@brief  set the configuration callback
*/
void ChronosESP32::setConfigurationCallback(void (*callback)(Config, uint32_t, uint32_t))
{
	configurationReceivedCallback = callback;
}

/*!
	@brief  set the data callback, assembled data packets matching the specified format (should start with 0xAB or 0xEA)
*/
void ChronosESP32::setDataCallback(void (*callback)(uint8_t *, int))
{
	dataReceivedCallback = callback;
}

/*!
	@brief  set the raw data callback, all incoming data via ble
*/
void ChronosESP32::setRawDataCallback(void (*callback)(uint8_t *, int))
{
	rawDataReceivedCallback = callback;
}

/*!
	@brief  send the info proprerties to the app
*/
void ChronosESP32::sendInfo()
{
	uint8_t infoCmd[] = {0xab, 0x00, 0x11, 0xff, 0x92, 0xc0, LIB_VER_MAJOR, (LIB_VER_MINOR * 10 + LIB_VER_PATCH), 0x00, 0xfb, 0x1e, 0x40, 0xc0, 0x0e, 0x32, 0x28, 0x00, 0xe2, screenConf, 0x80};
	sendCommand(infoCmd, 20);
}

/*!
	@brief  send the battery level
*/
void ChronosESP32::sendBattery()
{
	uint8_t c = isCharging ? 0x01 : 0x00;
	uint8_t batCmd[] = {0xAB, 0x00, 0x05, 0xFF, 0x91, 0x80, c, batteryLevel};
	sendCommand(batCmd, 8);
}

/*!
	@brief  request the battery level of the phone
*/
void ChronosESP32::setNotifyBattery(bool state)
{
	notifyPhone = state;
	uint8_t s = state ? 0x01 : 0x00;
	uint8_t batRq[] = {0xAB, 0x00, 0x04, 0xFE, 0x91, 0x80, s}; // custom command AB..FE
	sendCommand(batRq, 7);
}

/*!
	@brief  charging status of the phone
*/
bool ChronosESP32::isPhoneCharging()
{
	return phoneCharging;
}

/*!
	@brief  battery level of the phone
*/
uint8_t ChronosESP32::getPhoneBattery()
{
	return phoneBatteryLevel;
}

/*!
	@brief  app version code
*/
int ChronosESP32::getAppCode()
{
	return appCode;
}
/*!
	@brief  app version name
*/

String ChronosESP32::getAppVersion()
{
	return appVersion;
}

/*!
	@brief  get the app name from the notification id
	@param  id
			identifier of the app icon
*/
String ChronosESP32::appName(int id)
{
	switch (id)
	{
	case 0x03:
		return "Message";
	case 0x04:
		return "Mail";
	case 0x07:
		return "Tencent";
	case 0x08:
		return "Skype";
	case 0x09:
		return "Wechat";
	case 0x0A:
		return "WhatsApp";
	case 0x0B:
		return "Gmail";
	case 0x0E:
		return "Line";
	case 0x0F:
		return "Twitter";
	case 0x10:
		return "Facebook";
	case 0x11:
		return "Messenger";
	case 0x12:
		return "Instagram";
	case 0x13:
		return "Weibo";
	case 0x14:
		return "KakaoTalk";
	case 0x16:
		return "Viber";
	case 0x17:
		return "Vkontakte";
	case 0x18:
		return "Telegram";
	case 0x1B:
		return "DingTalk";
	case 0x20:
		return "WhatsApp Business";
	case 0x22:
		return "WearFit Pro";
	case 0xC0:
		return "Chronos";
	default:
		return "Message";
	}
}

/*!
	@brief  onConnect from BLEServerCallbacks
	@param  pServer
			BLE server object
*/
void ChronosESP32::onConnect(BLEServer *pServer)
{
	connected = true;
	infoTimer.active = true;
	infoTimer.time = millis();
	if (connectionChangeCallback != nullptr)
	{
		connectionChangeCallback(true);
	}
}

/*!
	@brief  onDisconnect from BLEServerCallbacks
	@param  pServer
			BLE server object
*/
void ChronosESP32::onDisconnect(BLEServer *pServer)
{
	connected = false;
	cameraReady = false;
	BLEDevice::startAdvertising();
	touch.state = false; // release touch
	if (connectionChangeCallback != nullptr)
	{
		connectionChangeCallback(false);
	}
}

/*!
	@brief  onWrite from BLECharacteristicCallbacks
	@param  pCharacteristic
			the BLECharacteristic object
*/
void ChronosESP32::onWrite(BLECharacteristic *pCharacteristic)
{
	std::string pData = pCharacteristic->getValue();
	int len = pData.length();
	if (len > 0)
	{
		if (rawDataReceivedCallback != nullptr)
		{
			rawDataReceivedCallback((uint8_t *)pData.data(), len);
		}

		if ((pData[0] == 0xAB || pData[0] == 0xEA) && (pData[3] == 0xFE || pData[3] == 0xFF))
		{
			// start of data, assign length from packet
			incomingData.length = pData[1] * 256 + pData[2] + 3;
			// copy data to incomingBuffer
			for (int i = 0; i < len; i++)
			{
				incomingData.data[i] = pData[i];
			}

			if (incomingData.length <= len)
			{
				// complete packet assembled
				dataReceived();
			}
			else
			{
				// data is still being assembled
				// Serial.println("Incomplete");
			}
		}
		else
		{
			int j = 20 + (pData[0] * 19); // data packet position
			// copy data to incomingBuffer
			for (int i = 0; i < len; i++)
			{
				incomingData.data[j + i] = pData[i + 1];
			}

			if (incomingData.length <= len + j - 1)
			{
				// complete packet assembled
				dataReceived();
			}
			else
			{
				// data is still being assembled
				// Serial.println("Incomplete");
			}
		}
	}
}

/*!
	@brief  dataReceived function, called after data packets have been assembled
*/
void ChronosESP32::dataReceived()
{
	int len = incomingData.length;

	if (dataReceivedCallback != nullptr)
	{
		dataReceivedCallback(incomingData.data, incomingData.length);
	}
	if (incomingData.data[0] == 0xAB)
	{
		switch (incomingData.data[4])
		{

		case 0x23:
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_RST, 0, 0);
			}
			break;
		case 0x53:
			if (configurationReceivedCallback != nullptr)
			{
				uint8_t hour = incomingData.data[7];
				uint8_t minute = incomingData.data[8];
				uint8_t hour2 = incomingData.data[9];
				uint8_t minute2 = incomingData.data[10];
				bool enabled = incomingData.data[6];
				uint32_t interval = ((uint32_t)incomingData.data[11] << 16) | (uint16_t)incomingData.data[6];
				uint32_t wtr = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)hour2 << 8) | ((uint32_t)minute2);
				configurationReceivedCallback(CF_WATER, interval, wtr);
			}
			break;

		case 0x71:
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_FIND, 0, 0);
			}
			break;

		case 0x72:
		{
			int icon = incomingData.data[6];
			int state = incomingData.data[7];

			String message = "";
			for (int i = 8; i < len; i++)
			{
				message += (char)incomingData.data[i];
			}

			if (icon == 0x01)
			{
				ringerTimer.time = millis();
				ringerTimer.active = true;
				// ringer command
				if (ringerAlertCallback != nullptr)
				{
					ringerAlertCallback(message, true);
				}
				break;
			}
			if (icon == 0x02)
			{
				ringerTimer.active = false;
				// cancel ringer command
				if (ringerAlertCallback != nullptr)
				{
					ringerAlertCallback(message, false);
				}
				break;
			}
			if (state == 0x02)
			{
				notificationIndex++;
				notifications[notificationIndex % NOTIF_SIZE].icon = icon;
				notifications[notificationIndex % NOTIF_SIZE].app = appName(icon);
				notifications[notificationIndex % NOTIF_SIZE].time = this->getTime("%H:%M");
				notifications[notificationIndex % NOTIF_SIZE].message = message;

				if (notificationReceivedCallback != nullptr)
				{
					notificationReceivedCallback(notifications[notificationIndex % NOTIF_SIZE]);
				}
			}
		}
		break;
		case 0x73:
		{
			uint8_t hour = incomingData.data[8];
			uint8_t minute = incomingData.data[9];
			uint8_t repeat = incomingData.data[10];
			bool enabled = incomingData.data[7];
			uint32_t index = (uint32_t)incomingData.data[6];
			alarms[index % ALARM_SIZE].hour = hour;
			alarms[index % ALARM_SIZE].minute = minute;
			alarms[index % ALARM_SIZE].repeat = repeat;
			alarms[index % ALARM_SIZE].enabled = enabled;
			if (configurationReceivedCallback != nullptr)
			{
				uint32_t alarm = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)repeat << 8) | ((uint32_t)enabled);
				configurationReceivedCallback(CF_ALARM, index, alarm);
			}
		}
		break;
		case 0x74:
			if (configurationReceivedCallback != nullptr)
			{
				// user.step, user.age, user.height, user.weight, si, user.target/1000, temp
				uint8_t age = incomingData.data[7];
				uint8_t height = incomingData.data[8];
				uint8_t weight = incomingData.data[9];
				uint8_t step = incomingData.data[6];
				uint32_t u1 = ((uint32_t)age << 24) | ((uint32_t)height << 16) | ((uint32_t)weight << 8) | ((uint32_t)step);
				uint8_t unit = incomingData.data[10];
				uint8_t target = incomingData.data[11];
				uint8_t temp = incomingData.data[12];
				uint32_t u2 = ((uint32_t)unit << 24) | ((uint32_t)target << 16) | ((uint32_t)temp << 8) | ((uint32_t)step);

				configurationReceivedCallback(CF_USER, u1, u2);
			}
			break;
		case 0x75:
			if (configurationReceivedCallback != nullptr)
			{
				uint8_t hour = incomingData.data[7];
				uint8_t minute = incomingData.data[8];
				uint8_t hour2 = incomingData.data[9];
				uint8_t minute2 = incomingData.data[10];
				bool enabled = incomingData.data[6];
				uint32_t interval = ((uint32_t)incomingData.data[11] << 16) | (uint16_t)incomingData.data[6];
				uint32_t sed = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)hour2 << 8) | ((uint32_t)minute2);
				configurationReceivedCallback(CF_SED, interval, sed);
			}
			break;
		case 0x76:
			if (configurationReceivedCallback != nullptr)
			{
				uint8_t hour = incomingData.data[7];
				uint8_t minute = incomingData.data[8];
				uint8_t hour2 = incomingData.data[9];
				uint8_t minute2 = incomingData.data[10];
				bool enabled = (uint32_t)incomingData.data[6];
				uint32_t qt = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)hour2 << 8) | ((uint32_t)minute2);
				configurationReceivedCallback(CF_QUIET, enabled, qt);
			}
			break;
		case 0x77:
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_RTW, 0, (uint32_t)incomingData.data[6]);
			}
			break;
		case 0x78:
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_HOURLY, 0, (uint32_t)incomingData.data[6]);
			}
			break;
		case 0x79:
			cameraReady = ((uint8_t)incomingData.data[6] == 1);
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_CAMERA, 0, (uint32_t)incomingData.data[6]);
			}
			break;
		case 0x7B:
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_LANG, 0, (uint32_t)incomingData.data[6]);
			}
			break;
		case 0x7C:
			hour24 = ((uint8_t)incomingData.data[6] == 0);
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_HR24, 0, (uint32_t)(incomingData.data[6] == 0));
			}
			break;
		case 0x7E:
		{
			weatherTime = this->getTime("%H:%M");
			weatherSize = 0;
			for (int k = 0; k < (len - 6) / 2; k++)
			{
				if (k >= WEATHER_SIZE)
				{
					break;
				}
				int icon = incomingData.data[(k * 2) + 6] >> 4;
				int sign = (incomingData.data[(k * 2) + 6] & 1) ? -1 : 1;
				int temp = ((int)incomingData.data[(k * 2) + 7]) * sign;
				int dy = this->getDayofWeek() + k;
				weather[k].day = dy % 7;
				weather[k].icon = icon;
				weather[k].temp = temp;
				weatherSize++;
			}
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_WEATHER, 1, 0);
			}
		}
		break;
		case 0x88:
		{
			for (int k = 0; k < (len - 6) / 2; k++)
			{
				if (k >= WEATHER_SIZE)
				{
					break;
				}
				int signH = (incomingData.data[(k * 2) + 6] >> 7 & 1) ? -1 : 1;
				int tempH = ((int)incomingData.data[(k * 2) + 6] & 0x7F) * signH;

				int signL = (incomingData.data[(k * 2) + 7] >> 7 & 1) ? -1 : 1;
				int tempL = ((int)incomingData.data[(k * 2) + 7] & 0x7F) * signL;

				weather[k].high = tempH;
				weather[k].low = tempL;
			}
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_WEATHER, 2, 0);
			}
		}
		break;
		case 0x7F:
			if (configurationReceivedCallback != nullptr)
			{
				uint8_t hour = incomingData.data[7];
				uint8_t minute = incomingData.data[8];
				uint8_t hour2 = incomingData.data[9];
				uint8_t minute2 = incomingData.data[10];
				bool enabled = incomingData.data[6];
				uint32_t slp = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)hour2 << 8) | ((uint32_t)minute2);
				configurationReceivedCallback(CF_SLEEP, enabled, slp);
			}
			break;
		case 0x91:

			if (incomingData.data[3] == 0xFE)
			{
				phoneCharging = incomingData.data[6] == 1;
				phoneBatteryLevel = incomingData.data[7];
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_PBAT, incomingData.data[6], phoneBatteryLevel);
				}
			}

			break;
		case 0x93:
			this->setTime(incomingData.data[13], incomingData.data[12], incomingData.data[11], incomingData.data[10], incomingData.data[9], incomingData.data[7] * 256 + incomingData.data[8]);

			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_TIME, 0, 0);
			}
			break;
		case 0x9C:
			if (configurationReceivedCallback != nullptr)
			{
				uint32_t color = ((uint32_t)incomingData.data[5] << 16) | ((uint32_t)incomingData.data[6] << 8) | (uint32_t)incomingData.data[7];
				uint32_t select = ((uint32_t)(incomingData.data[8]) << 16) | (uint32_t)incomingData.data[9];
				configurationReceivedCallback(CF_FONT, color, select);
			}
			break;
		case 0xA8:
			if (incomingData.data[3] == 0xFE)
			{
				// end of qr data
				int size = incomingData.data[5]; // number of links received
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_QR, 1, size);
				}
			}
			if (incomingData.data[3] == 0xFF)
			{
				// receiving qr data
				int index = incomingData.data[5]; // index of the curent link
				qrLinks[index] = ""; // clear existing
				for (int i = 6; i < len; i++)
				{
					qrLinks[index] += (char)incomingData.data[i];
				}
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_QR, 0, index);
				}
				
			}
			break;
		case 0xBF:
			if (incomingData.data[3] == 0xFE)
			{
				touch.state = incomingData.data[5] == 1;
				touch.x = uint32_t(incomingData.data[6] << 8) | uint32_t(incomingData.data[7]);
				touch.y = uint32_t(incomingData.data[8] << 8) | uint32_t(incomingData.data[9]);
			}
			break;
		case 0xCA:
			if (incomingData.data[3] == 0xFE)
			{
				appCode = (incomingData.data[6] * 256) + incomingData.data[7];
				appVersion = "";
				for (int i = 8; i < len; i++)
				{
					appVersion += (char)incomingData.data[i];
				}
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_APP, appCode, 0);
				}
			}
			break;
		case 0xEE:
			if (incomingData.data[3] == 0xFE)
			{
				//nvIc
			}
			break;
		case 0xEF:
			if (incomingData.data[3] == 0xFE)
			{
				//nvData
			}
			break;
		}
	}
	else if (incomingData.data[0] == 0xEA)
	{
		if (incomingData.data[4] == 0x7E)
		{
			switch (incomingData.data[5])
			{
			case 0x01:
			{
				String city = "";
				for (int c = 7; c < len; c++)
				{
					city += (char)incomingData.data[c];
				}
				weatherCity = city;
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_WEATHER, 0, 1);
				}
			}
			break;
			case 0x02:
			{
				int size = incomingData.data[6];
				int hour = incomingData.data[7];
				for (int z = 0; z < size; z++)
				{
					if (hour + z >= FORECAST_SIZE)
					{
						break;
					}
					int icon = incomingData.data[8 + (6 * z)] >> 4;
					int sign = (incomingData.data[8 + (6 * z)] & 1) ? -1 : 1;
					int temp = ((int)incomingData.data[9 + (6 * z)]) * sign;

					hourlyForecast[hour + z].day = this->getDayofYear();
					hourlyForecast[hour + z].hour = hour + z;
					hourlyForecast[hour + z].wind = (incomingData.data[10 + (6 * z)] * 256) + incomingData.data[11 + (6 * z)];
					hourlyForecast[hour + z].humidity = incomingData.data[12 + (6 * z)];
					hourlyForecast[hour + z].uv = incomingData.data[13 + (6 * z)];
					hourlyForecast[hour + z].icon = icon;
					hourlyForecast[hour + z].temp = temp;
				}
			}
			break;
			}
		}
	}
}

