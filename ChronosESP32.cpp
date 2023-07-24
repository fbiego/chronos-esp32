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

#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

static BLECharacteristic *pCharacteristicTX;
static BLECharacteristic *pCharacteristicRX;

/*!
	@brief  Constructor for ChronosESP32
*/
ChronosESP32::ChronosESP32()
{
	connected = false;
	cameraReady = false;
	receiving = false;
}

/*!
	@brief  Constructor for ChronosESP32
	@param  name
			Bluetooth name
*/
ChronosESP32::ChronosESP32(String name)
{
	watchName = name;
	ChronosESP32();
}

/*!
	@brief  begin
*/
void ChronosESP32::begin()
{
	BLEDevice::init(watchName.c_str());
	BLEServer *pServer = BLEDevice::createServer();
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

	findTimer.duration = 10000; // 10 seconds for find phone
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
}

/*!
	@brief  set the logging state
	@param	state
			new state of logging
*/
void ChronosESP32::setLogging(bool state)
{
	logging = state;
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
*/
void ChronosESP32::setBattery(uint8_t level)
{
	if (batteryLevel != level)
	{
		batteryChanged = true;
		batteryLevel = level;
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
	pCharacteristicTX->setValue(musicCmd, 7);
	pCharacteristicTX->notify();
	vTaskDelay(200);
}

/*!
	@brief  send capture photo command to the app
*/
bool ChronosESP32::capturePhoto()
{
	if (cameraReady)
	{
		uint8_t captureCmd[] = {0xAB, 0x00, 0x04, 0xFF, 0x79, 0x80, 0x01};
		pCharacteristicTX->setValue(captureCmd, 7);
		pCharacteristicTX->notify();
		vTaskDelay(200);
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
	pCharacteristicTX->setValue(findCmd, 7);
	pCharacteristicTX->notify();
	vTaskDelay(200);
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
		return this->getAmPm(caps);
	}
	return "";
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
	@brief  set the configuration callback
*/
void ChronosESP32::setConfigurationCallback(void (*callback)(Config, uint32_t, uint32_t))
{
	configurationReceivedCallback = callback;
}

/*!
	@brief  set the data callback
*/
void ChronosESP32::setDataCallback(void (*callback)(uint8_t *, int))
{
	dataReceivedCallback = callback;
}

/*!
	@brief  send the info proprerties to the app
*/
void ChronosESP32::sendInfo()
{
	uint8_t infoCmd[] = {0xab, 0x00, 0x11, 0xff, 0x92, 0xc0, 0x01, 0x00, 0x00, 0xfb, 0x1e, 0x40, 0xc0, 0x0e, 0x32, 0x28, 0x00, 0xe2, 0x07, 0x80};
	pCharacteristicTX->setValue(infoCmd, 20);
	pCharacteristicTX->notify();
	vTaskDelay(200);
}

/*!
	@brief  send the battery level
*/
void ChronosESP32::sendBattery()
{
	uint8_t batCmd[] = {0xAB, 0x00, 0x05, 0xFF, 0x91, 0x80, 0x00, batteryLevel};
	pCharacteristicTX->setValue(batCmd, 8);
	pCharacteristicTX->notify();
	vTaskDelay(200);
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
		return "ChronosESP32";
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
		if (logging)
		{
			for (int i = 0; i < len; i++)
			{
				Serial.printf("%02X ", pData[i]);
			}
			Serial.println();
		}
		if (dataReceivedCallback != nullptr)
		{
			dataReceivedCallback((uint8_t *)pData.data(), len);
		}
		if (pData[0] == 0xAB)
		{
			switch (pData[4])
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
					uint8_t hour = pData[7];
					uint8_t minute = pData[8];
					uint8_t hour2 = pData[9];
					uint8_t minute2 = pData[10];
					bool enabled = pData[6];
					uint32_t interval = ((uint32_t)pData[11] << 16) | (uint16_t)pData[6];
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
				int icon = pData[6];

				if (icon == 0x02)
				{
					// skip cancel call command
					break;
				}

				notificationIndex++;
				notifications[notificationIndex % NOTIF_SIZE].icon = icon;
				notifications[notificationIndex % NOTIF_SIZE].app = appName(icon);
				notifications[notificationIndex % NOTIF_SIZE].time = this->getTime("%H:%M");

				String message = "";
				for (int i = 8; i < len; i++)
				{
					message += (char)pData[i];
				}

				notifications[notificationIndex % NOTIF_SIZE].message = message;

				msgLen = pData[2] + 2;

				if (msgLen <= 19)
				{
					// message is complete
					receiving = false;
					if (notificationReceivedCallback != nullptr)
					{
						notificationReceivedCallback(notifications[notificationIndex % NOTIF_SIZE]);
					}
				}
				else
				{
					// message not complete
					receiving = true;
				}
			}
			break;
			case 0x73:
			{
				uint8_t hour = pData[8];
				uint8_t minute = pData[9];
				uint8_t repeat = pData[10];
				bool enabled = pData[7];
				uint32_t index = (uint32_t)pData[6];
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
					uint8_t age = pData[7];
					uint8_t height = pData[8];
					uint8_t weight = pData[9];
					uint8_t step = pData[6];
					uint32_t u1 = ((uint32_t)age << 24) | ((uint32_t)height << 16) | ((uint32_t)weight << 8) | ((uint32_t)step);
					uint8_t unit = pData[10];
					uint8_t target = pData[11];
					uint8_t temp = pData[12];
					uint32_t u2 = ((uint32_t)unit << 24) | ((uint32_t)target << 16) | ((uint32_t)temp << 8) | ((uint32_t)step);

					configurationReceivedCallback(CF_USER, u1, u2);
				}
				break;
			case 0x75:
				if (configurationReceivedCallback != nullptr)
				{
					uint8_t hour = pData[7];
					uint8_t minute = pData[8];
					uint8_t hour2 = pData[9];
					uint8_t minute2 = pData[10];
					bool enabled = pData[6];
					uint32_t interval = ((uint32_t)pData[11] << 16) | (uint16_t)pData[6];
					uint32_t sed = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)hour2 << 8) | ((uint32_t)minute2);
					configurationReceivedCallback(CF_SED, interval, sed);
				}
				break;
			case 0x76:
				if (configurationReceivedCallback != nullptr)
				{
					uint8_t hour = pData[7];
					uint8_t minute = pData[8];
					uint8_t hour2 = pData[9];
					uint8_t minute2 = pData[10];
					bool enabled = (uint32_t)pData[6];
					uint32_t qt = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)hour2 << 8) | ((uint32_t)minute2);
					configurationReceivedCallback(CF_QUIET, enabled, qt);
				}
				break;
			case 0x77:
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_RTW, 0, (uint32_t)pData[6]);
				}
				break;
			case 0x78:
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_HOURLY, 0, (uint32_t)pData[6]);
				}
				break;
			case 0x79:
				cameraReady = ((uint8_t)pData[6] == 1);
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_CAMERA, 0, (uint32_t)pData[6]);
				}
				break;
			case 0x7B:
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_LANG, 0, (uint32_t)pData[6]);
				}
				break;
			case 0x7C:
				hour24 = ((uint8_t)pData[6] == 0);
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_HR24, 0, (uint32_t)(pData[6] == 0));
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
					int icon = pData[(k * 2) + 6] >> 4;
					int sign = (pData[(k * 2) + 6] & 1) ? -1 : 1;
					int temp = ((int)pData[(k * 2) + 7]) * sign;
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
			case 0x7F:
				if (configurationReceivedCallback != nullptr)
				{
					uint8_t hour = pData[7];
					uint8_t minute = pData[8];
					uint8_t hour2 = pData[9];
					uint8_t minute2 = pData[10];
					bool enabled = pData[6];
					uint32_t slp = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)hour2 << 8) | ((uint32_t)minute2);
					configurationReceivedCallback(CF_SLEEP, enabled, slp);
				}
				break;
			case 0x93:
				this->setTime(pData[13], pData[12], pData[11], pData[10], pData[9], pData[7] * 256 + pData[8]);

				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_TIME, 0, 0);
				}
				break;
			case 0x9C:
				if (configurationReceivedCallback != nullptr)
				{
					uint32_t color = ((uint32_t)pData[5] << 16) | ((uint32_t)pData[6] << 8) | (uint32_t)pData[7];
					uint32_t select = ((uint32_t)(pData[8]) << 16) | (uint32_t)pData[9];
					configurationReceivedCallback(CF_FONT, color, select);
				}
				break;
			}
		}
		else if (pData[0] == 0xEA)
		{
			if (pData[4] == 0x7E && pData[5] == 0x01)
			{
				String city = "";
				for (int c = 7; c < len; c++)
				{
					city += (char)pData[c];
				}
				weatherCity = city;
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_WEATHER, 0, 1);
				}
			}
		}
		else if (pData[0] <= 0x0F)
		{
			if (receiving)
			{
				String message = "";
				for (int i = 1; i < len; i++)
				{
					message += (char)pData[i];
				}
				notifications[notificationIndex % NOTIF_SIZE].message += message;
				if (((msgLen > (pData[0] + 1) * 19) && (msgLen <= (pData[0] + 2) * 19)) || (pData[0] == 0x0F))
				{
					// message is complete || message is longer than expected, truncate
					receiving = false;

					if (notificationReceivedCallback != nullptr)
					{
						notificationReceivedCallback(notifications[notificationIndex % NOTIF_SIZE]);
					}
				}
			}
		}
	}
}
