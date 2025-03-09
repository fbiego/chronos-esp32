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
	_connected = false;
	_cameraReady = false;
	_batteryChanged = true;
	_qrLinks[0] = "https://chronos.ke/";

	_notifications[0].icon = 0xC0;
	_notifications[0].time = "Now";
	_notifications[0].app = "Chronos";
	_notifications[0].message = "Download from Google Play to sync time and receive notifications";

	_infoTimer.duration = 3 * 1000;	   // 3 seconds for info timer
	_findTimer.duration = 30 * 1000;   // 30 seconds for find phone
	_ringerTimer.duration = 30 * 1000; // 30 seconds for ringer alert
}

/*!
	@brief  Constructor for ChronosESP32
	@param  name
			Bluetooth name
	@param  screen
			screen config
*/
ChronosESP32::ChronosESP32(String name, ChronosScreen screen)
{
	_watchName = name;
	_screenConf = screen;
	ChronosESP32();
}

/*!
	@brief  set bluetooth name (call before begin function)
	@param  name
			Bluetooth name
*/
void ChronosESP32::setName(String name)
{
	_watchName = name;
}

/*!
	@brief  set screen config (call before begin function)
	@param  screen
			screen config
*/
void ChronosESP32::setScreen(ChronosScreen screen)
{
	_screenConf = screen;
}

/*!
	@brief  initializes bluetooth LE server
*/
void ChronosESP32::begin()
{
	BLEDevice::init(_watchName.c_str());
	BLEServer *pServer = BLEDevice::createServer();
	BLEDevice::setMTU(517);
	pServer->setCallbacks(this, false);

	BLEService *pService = pServer->createService(SERVICE_UUID);
	pCharacteristicTX = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);
	pCharacteristicRX = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
	pCharacteristicRX->setCallbacks(this);
	pCharacteristicTX->setCallbacks(this);
	pService->start();

	BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(SERVICE_UUID);
	pAdvertising->enableScanResponse(true);
	pAdvertising->setPreferredParams(0x06, 0x12); // functions that help with iPhone connections issue
	pAdvertising->setName(_watchName.c_str());
	pAdvertising->start();

	_address = BLEDevice::getAddress().toString().c_str();

	_inited = true;
}

/*!
	@brief  stops bluetooth LE server
*/
void ChronosESP32::stop(bool clearAll)
{
	BLEDevice::deinit(clearAll);
	_inited = false;
}

/*!
	@brief  check whether bluetooth LE server is initialized and running
*/
bool ChronosESP32::isRunning()
{
	return _inited;
}

/*!
	@brief  handles routine functions
*/
void ChronosESP32::loop()
{
	if (!_inited)
	{
		// begin not called. do nothing
		return;
	}

	if (_connected)
	{
		if (_infoTimer.active)
		{
			if (_infoTimer.time + _infoTimer.duration < millis())
			{
				// timer end
				_infoTimer.active = false;

				sendInfo();
				sendBattery();
				setNotifyBattery(_notifyPhone);
			}
		}
		if (_findTimer.active)
		{
			if (_findTimer.time + _findTimer.duration < millis())
			{
				// timer end
				_findTimer.active = false;
				findPhone(false); // auto cancel the command
			}
		}
		if (_batteryChanged)
		{
			_batteryChanged = false;
			sendBattery();
		}
		if (_sendESP)
		{
			_sendESP = false;
			sendESP();
		}
	}

	if (_ringerTimer.active)
	{
		if (_ringerTimer.time + _ringerTimer.duration < millis())
		{
			// ring timer end
			_ringerTimer.active = false;
			if (ringerAlertCallback != nullptr)
			{
				ringerAlertCallback("", false);
			}
		}
	}
}

/*!
	@brief  set whether to split transferred bytes
	@param  chunked
			enable or disable state
*/
void ChronosESP32::setChunkedTransfer(bool chunked)
{
	_chunked = chunked;
}

/*!
	@brief  check whether the device is connected
*/
bool ChronosESP32::isConnected()
{
	return _connected;
}

/*!
	@brief  check whether the device is subcribed to ble notifications
*/
bool ChronosESP32::isSubscribed()
{
	return _subscribed;
}

/*!
	@brief  set the clock to 24 hour mode
	@param  mode
			enable or disable state
*/
void ChronosESP32::set24Hour(bool mode)
{
	_hour24 = mode;
}

/*!
	@brief  return the 24 hour mode
*/
bool ChronosESP32::is24Hour()
{
	return _hour24;
}

/*!
	@brief  return the mac address
*/
String ChronosESP32::getAddress()
{
	return _address;
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
	if (_batteryLevel != level || _isCharging != charging)
	{
		_batteryChanged = true;
		_batteryLevel = level;
		_isCharging = charging;
	}
}

/*!
	@brief  return camera status
*/
bool ChronosESP32::isCameraReady()
{
	return _cameraReady;
}

/*!
	@brief  return the number of notifications in the buffer
*/
int ChronosESP32::getNotificationCount()
{
	if (_notificationIndex + 1 >= NOTIF_SIZE)
	{
		return NOTIF_SIZE; // the buffer is full
	}
	else
	{
		return _notificationIndex + 1; // the buffer is not full,
	}
}

/*!
	@brief  return the notification at the buffer index
	@param  index
			position of the notification to be returned, at 0 is the latest received
*/
Notification ChronosESP32::getNotificationAt(int index)
{
	int latestIndex = (_notificationIndex - index + NOTIF_SIZE) % NOTIF_SIZE;
	return _notifications[latestIndex];
}

/*!
	@brief  clear the notificaitons
*/
void ChronosESP32::clearNotifications()
{
	// here we just set the index to -1, existing data at the buffer will be overwritten
	// getNotificationCount() will return 0 but getNotificationAt() will return previous existing data
	_notificationIndex = -1;
}

/*!
	@brief  set the contact at the index
	@param  index
			position to set the contact
	@param  contact
			the contact to be set
*/
void ChronosESP32::setContact(int index, Contact contact)
{
	_contacts[index % CONTACTS_SIZE] = contact;
}

/*!
	@brief  return the contact at the index
	@param  index
			position of the contact to be returned
*/
Contact ChronosESP32::getContact(int index)
{
	return _contacts[index % CONTACTS_SIZE];
}

/*!
	@brief  return the contact size
*/
int ChronosESP32::getContactCount()
{
	return _contactSize;
}

/*!
	@brief  return the sos contact
*/
Contact ChronosESP32::getSoSContact()
{
	return _contacts[_sosContact % CONTACTS_SIZE];
}

/*!
	@brief  set the sos contact index
*/
void ChronosESP32::setSOSContactIndex(int index)
{
	_sosContact = index;
}

/*!
	@brief  return sos contact index
*/
int ChronosESP32::getSOSContactIndex()
{
	return _sosContact;
}

/*!
	@brief  return the weather count
*/
int ChronosESP32::getWeatherCount()
{
	return _weatherSize;
}

/*!
	@brief  return the weather city name
*/
String ChronosESP32::getWeatherCity()
{
	return _weatherCity;
}

/*!
	@brief  return the weather update time, HH:MM format
*/
String ChronosESP32::getWeatherTime()
{
	return _weatherTime;
}

/*!
	@brief  return the weather at the buffer index
	@param  index
			position of the weather to be returned
*/
Weather ChronosESP32::getWeatherAt(int index)
{
	return _weather[index % WEATHER_SIZE];
}

/*!
	@brief  return the weatherforecast for the hour
	@param  hour
			position of the weather to be returned
*/
HourlyForecast ChronosESP32::getForecastHour(int hour)
{
	return _hourlyForecast[hour % FORECAST_SIZE];
}

/*!
	@brief  get the alarm at the index
	@param	index
			position of the alarm to be returned
*/
Alarm ChronosESP32::getAlarm(int index)
{
	return _alarms[index % ALARM_SIZE];
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
	_alarms[index % ALARM_SIZE] = alarm;
}

/*!
	@brief  send a command to the app
	@param  command
			command data
	@param  length
			command length
	@param  force_chunked
			override internal chunked
*/
void ChronosESP32::sendCommand(uint8_t *command, size_t length, bool force_chunked)
{
	if (!_inited)
	{
		// begin not called. do nothing
		return;
	}

	if ((length <= 20 || !_chunked) && !force_chunked)
	{
		// Send the entire command if it fits in one packet
		pCharacteristicTX->setValue(command, length);
		pCharacteristicTX->notify();
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
	else
	{
		// Send the first 20 bytes as is (no header)
		pCharacteristicTX->setValue(command, 20);
		pCharacteristicTX->notify();
		vTaskDelay(200 / portTICK_PERIOD_MS);

		// Send the remaining bytes with a header
		const size_t maxPayloadSize = 19; // Payload size excluding header
		uint8_t chunk[20];				  // Buffer for chunks with header
		size_t offset = 20;				  // Start after the first 20 bytes
		uint8_t sequenceNumber = 0;		  // Sequence number for headers

		while (offset < length)
		{
			// Add the header (sequence number)
			chunk[0] = sequenceNumber++;

			// Calculate how many bytes to send in this chunk
			size_t bytesToSend = min(maxPayloadSize, length - offset);

			// Copy data to chunk, leaving space for the header
			memcpy(chunk + 1, command + offset, bytesToSend);

			// Send the chunk
			pCharacteristicTX->setValue(chunk, bytesToSend + 1);
			pCharacteristicTX->notify();
			vTaskDelay(200 / portTICK_PERIOD_MS);

			// Update offset
			offset += bytesToSend;
		}
	}
}

/*!
	@brief  send a music control command to the app
	@param  command
			music action
*/
void ChronosESP32::musicControl(Control command)
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
	if (_cameraReady)
	{
		uint8_t captureCmd[] = {0xAB, 0x00, 0x04, 0xFF, 0x79, 0x80, 0x01};
		sendCommand(captureCmd, 7);
	}
	return _cameraReady;
}

/*!
	@brief  send a command to find the phone
	@param  state
			enable or disable state
*/
void ChronosESP32::findPhone(bool state)
{
	_findTimer.active = state;
	if (state)
	{
		_findTimer.time = millis();
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
	return this->getHour(_hour24);
}

/*!
	@brief  get the zero padded hour based on hour 24 mode
*/
String ChronosESP32::getHourZ()
{
	return this->getTime(_hour24 ? "%H" : "%I");
}

/*!
	@brief  get the am pm label
	@param	caps
			capital letters mode
*/
String ChronosESP32::getAmPmC(bool caps)
{
	return _hour24 ? "" : this->getAmPm(!caps); // on esp32time it's getAmPm(bool lowercase);
}

/*!
	@brief  get remote touch data
*/
RemoteTouch ChronosESP32::getTouch()
{
	return _touch;
}

/*!
	@brief  get the qr link at the index
	@param	index
			position of the qr link to be returned
*/
String ChronosESP32::getQrAt(int index)
{
	return _qrLinks[index % QR_SIZE];
}

void ChronosESP32::setQr(int index, String qr)
{
	_qrLinks[index % QR_SIZE] = qr;
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
	@brief  send the info properties to the app
*/
void ChronosESP32::sendInfo()
{
	uint8_t infoCmd[] = {0xab, 0x00, 0x11, 0xff, 0x92, 0xc0, CHRONOSESP_VERSION_MAJOR, (CHRONOSESP_VERSION_MINOR * 10 + CHRONOSESP_VERSION_PATCH), 0x00, 0xfb, 0x1e, 0x40, 0xc0, 0x0e, 0x32, 0x28, 0x00, 0xe2, _screenConf, 0x80};
	sendCommand(infoCmd, 20);
}

/*!
	@brief  send the esp properties to the app
*/
void ChronosESP32::sendESP()
{
	String espInfo;
	espInfo += "ChronosESP32 v" + String(CHRONOSESP_VERSION_MAJOR) + "." + String(CHRONOSESP_VERSION_MINOR) + "." + String(CHRONOSESP_VERSION_PATCH);
	espInfo += "\n" + String(ESP.getChipModel());
	espInfo += " @" + String(ESP.getCpuFreqMHz()) + "Mhz";
	espInfo += " Cores:" + String(ESP.getChipCores());
	espInfo += " rev" + String(ESP.getChipRevision());

	espInfo += "\nRAM: " + String((ESP.getHeapSize() / 1024.0), 0) + "kB";
	espInfo += " + PSRAM: " + String((ESP.getPsramSize() / (1024.0 * 1024.0)), 0) + "MB";

	espInfo += "\nFlash: " + String((ESP.getFlashChipSize() / (1024.0 * 1024.0)), 0) + "MB";
	espInfo += " @" + String((ESP.getFlashChipSpeed() / 1000000.0), 0) + "Mhz";
	espInfo += " " + flashMode(ESP.getFlashChipMode());

	espInfo += "\nSDK: " + String(ESP.getSdkVersion());
	espInfo += "\nSketch: " + String((ESP.getSketchSize() / (1024.0)), 0) + "kB";

	if (espInfo.length() > 505)
	{
		espInfo = espInfo.substring(0, 505);
	}

	uint16_t len = espInfo.length();
	_outgoingData.data[0] = 0xAB;
	_outgoingData.data[1] = highByte(len + 3);
	_outgoingData.data[2] = lowByte(len + 3);
	_outgoingData.data[3] = 0xFE;
	_outgoingData.data[4] = 0x92;
	_outgoingData.data[5] = 0x80;
	espInfo.toCharArray((char *)_outgoingData.data + 6, 506);
	sendCommand((uint8_t *)_outgoingData.data, 6 + len, true);
}

/*!
	@brief  get flash mode string
	@param	mode
			flash mode type
*/
String ChronosESP32::flashMode(FlashMode_t mode)
{
	switch (mode)
	{
	case FM_QIO:
		return "QIO";
	case FM_QOUT:
		return "QOUT";
	case FM_DIO:
		return "DIO";
	case FM_DOUT:
		return "DOUT";
	case FM_FAST_READ:
		return "FAST_READ";
	case FM_SLOW_READ:
		return "SLOW_READ";
	default:
		return "UNKNOWN";
	}
}

/*!
	@brief  send the battery level
*/
void ChronosESP32::sendBattery()
{
	uint8_t c = _isCharging ? 0x01 : 0x00;
	uint8_t batCmd[] = {0xAB, 0x00, 0x05, 0xFF, 0x91, 0x80, c, _batteryLevel};
	sendCommand(batCmd, 8);
}

/*!
	@brief  request the battery level of the phone
*/
void ChronosESP32::setNotifyBattery(bool state)
{
	_notifyPhone = state;
	uint8_t s = state ? 0x01 : 0x00;
	uint8_t batRq[] = {0xAB, 0x00, 0x04, 0xFE, 0x91, 0x80, s}; // custom command AB..FE
	sendCommand(batRq, 7);
}

/*!
	@brief  charging status of the phone
*/
bool ChronosESP32::isPhoneCharging()
{
	return _phoneCharging;
}

/*!
	@brief  battery level of the phone
*/
uint8_t ChronosESP32::getPhoneBattery()
{
	return _phoneBatteryLevel;
}

/*!
	@brief  app version code
*/
int ChronosESP32::getAppCode()
{
	return _appCode;
}
/*!
	@brief  app version name
*/

String ChronosESP32::getAppVersion()
{
	return _appVersion;
}

/*!
	@brief  get navigation data
*/
Navigation ChronosESP32::getNavigation()
{
	return _navigation;
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
	@param	connInfo
			connection information
*/
void ChronosESP32::onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo)
{
	_connected = true;
	if (connectionChangeCallback != nullptr)
	{
		connectionChangeCallback(true);
	}
}

/*!
	@brief  onDisconnect from BLEServerCallbacks
	@param  pServer
			BLE server object
	@param	connInfo
			connection information
	@param	reason
			disconnect reason
*/
void ChronosESP32::onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason)
{
	_connected = false;
	_cameraReady = false;
	BLEDevice::startAdvertising();
	_touch.state = false; // release touch

	if (_navigation.active)
	{
		_navigation.active = false;
		if (configurationReceivedCallback != nullptr)
		{
			configurationReceivedCallback(CF_NAV_DATA, _navigation.active ? 1 : 0, 0);
		}
	}

	if (connectionChangeCallback != nullptr)
	{
		connectionChangeCallback(false);
	}
}

/*!
	@brief  onSubscribe to BLECharacteristicCallbacks
	@param  pCharacteristic
			the BLECharacteristic object
	@param	connInfo
			connection information
	@param	subValue
			subcribe value
*/
void ChronosESP32::onSubscribe(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo, uint16_t subValue)
{
	if (pCharacteristic == pCharacteristicTX)
	{
		_subscribed = subValue == 1;

		if (_subscribed)
		{
			_infoTimer.time = millis();
			_infoTimer.active = true;
		}
	}
}

/*!
	@brief  onWrite from BLECharacteristicCallbacks
	@param  pCharacteristic
			the BLECharacteristic object
	@param	connInfo
			connection information
*/
void ChronosESP32::onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo)
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
			_incomingData.length = pData[1] * 256 + pData[2] + 3;
			// copy data to incomingBuffer
			for (int i = 0; i < len; i++)
			{
				_incomingData.data[i] = pData[i];
			}

			if (_incomingData.length <= len)
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
				_incomingData.data[j + i] = pData[i + 1];
			}

			if (_incomingData.length <= len + j - 1)
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

		if (pData[0] == 0xB0)
		{
			// bin watchface chunk info
		}

		if (pData[0] == 0xAF)
		{
			// bin watchface chunk data
		}
	}
}

void  ChronosESP32::splitTitle(const String &input, String &title, String &message, int icon) {
    int index = input.indexOf(':');  // Find the first occurrence of ':'
    int newlineIndex = input.indexOf('\n'); // Find the first occurrence of '\n'

    if (index != -1 && index < 30 && (newlineIndex == -1 || newlineIndex > index)) {
        // Split only if ':' is before index 30 and there's no '\n' before it
        title = input.substring(0, index);
        message = input.substring(index + 1);
    } else {
        title = appName(icon);  // No valid ':' before index 30, or '\n' appears before ':'
        message = input; // Keep the full string in message
    }
}

/*!
	@brief  dataReceived function, called after data packets have been assembled
*/
void ChronosESP32::dataReceived()
{
	int len = _incomingData.length;

	if (dataReceivedCallback != nullptr)
	{
		dataReceivedCallback(_incomingData.data, _incomingData.length);
	}
	if (_incomingData.data[0] == 0xAB)
	{
		switch (_incomingData.data[4])
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
				uint8_t hour = _incomingData.data[7];
				uint8_t minute = _incomingData.data[8];
				uint8_t hour2 = _incomingData.data[9];
				uint8_t minute2 = _incomingData.data[10];
				bool enabled = _incomingData.data[6];
				uint32_t interval = ((uint32_t)_incomingData.data[11] << 16) | (uint16_t)_incomingData.data[6];
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
			int icon = _incomingData.data[6];
			int state = _incomingData.data[7];

			String message = "";
			for (int i = 8; i < len; i++)
			{
				message += (char)_incomingData.data[i];
			}

			if (icon == 0x01)
			{
				_ringerTimer.time = millis();
				_ringerTimer.active = true;
				// ringer command
				if (ringerAlertCallback != nullptr)
				{
					ringerAlertCallback(message, true);
				}
				break;
			}
			if (icon == 0x02)
			{
				_ringerTimer.active = false;
				// cancel ringer command
				if (ringerAlertCallback != nullptr)
				{
					ringerAlertCallback(message, false);
				}
				break;
			}
			if (state == 0x02)
			{
				_notificationIndex++;
				_notifications[_notificationIndex % NOTIF_SIZE].icon = icon;
				_notifications[_notificationIndex % NOTIF_SIZE].app = appName(icon);
				_notifications[_notificationIndex % NOTIF_SIZE].time = this->getTime("%H:%M");
				splitTitle(message, _notifications[_notificationIndex % NOTIF_SIZE].title, _notifications[_notificationIndex % NOTIF_SIZE].message, icon);

				if (notificationReceivedCallback != nullptr)
				{
					notificationReceivedCallback(_notifications[_notificationIndex % NOTIF_SIZE]);
				}
			}
		}
		break;
		case 0x73:
		{
			uint8_t hour = _incomingData.data[8];
			uint8_t minute = _incomingData.data[9];
			uint8_t repeat = _incomingData.data[10];
			bool enabled = _incomingData.data[7];
			uint32_t index = (uint32_t)_incomingData.data[6];
			_alarms[index % ALARM_SIZE].hour = hour;
			_alarms[index % ALARM_SIZE].minute = minute;
			_alarms[index % ALARM_SIZE].repeat = repeat;
			_alarms[index % ALARM_SIZE].enabled = enabled;
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
				uint8_t age = _incomingData.data[7];
				uint8_t height = _incomingData.data[8];
				uint8_t weight = _incomingData.data[9];
				uint8_t step = _incomingData.data[6];
				uint32_t u1 = ((uint32_t)age << 24) | ((uint32_t)height << 16) | ((uint32_t)weight << 8) | ((uint32_t)step);
				uint8_t unit = _incomingData.data[10];
				uint8_t target = _incomingData.data[11];
				uint8_t temp = _incomingData.data[12];
				uint32_t u2 = ((uint32_t)unit << 24) | ((uint32_t)target << 16) | ((uint32_t)temp << 8) | ((uint32_t)step);

				configurationReceivedCallback(CF_USER, u1, u2);
			}
			break;
		case 0x75:
			if (configurationReceivedCallback != nullptr)
			{
				uint8_t hour = _incomingData.data[7];
				uint8_t minute = _incomingData.data[8];
				uint8_t hour2 = _incomingData.data[9];
				uint8_t minute2 = _incomingData.data[10];
				bool enabled = _incomingData.data[6];
				uint32_t interval = ((uint32_t)_incomingData.data[11] << 16) | (uint16_t)_incomingData.data[6];
				uint32_t sed = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)hour2 << 8) | ((uint32_t)minute2);
				configurationReceivedCallback(CF_SED, interval, sed);
			}
			break;
		case 0x76:
			if (configurationReceivedCallback != nullptr)
			{
				uint8_t hour = _incomingData.data[7];
				uint8_t minute = _incomingData.data[8];
				uint8_t hour2 = _incomingData.data[9];
				uint8_t minute2 = _incomingData.data[10];
				bool enabled = (uint32_t)_incomingData.data[6];
				uint32_t qt = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)hour2 << 8) | ((uint32_t)minute2);
				configurationReceivedCallback(CF_QUIET, enabled, qt);
			}
			break;
		case 0x77:
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_RTW, 0, (uint32_t)_incomingData.data[6]);
			}
			break;
		case 0x78:
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_HOURLY, 0, (uint32_t)_incomingData.data[6]);
			}
			break;
		case 0x79:
			_cameraReady = ((uint8_t)_incomingData.data[6] == 1);
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_CAMERA, 0, (uint32_t)_incomingData.data[6]);
			}
			break;
		case 0x7B:
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_LANG, 0, (uint32_t)_incomingData.data[6]);
			}
			break;
		case 0x7C:
			_hour24 = ((uint8_t)_incomingData.data[6] == 0);
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_HR24, 0, (uint32_t)(_incomingData.data[6] == 0));
			}
			break;
		case 0x7E:
		{
			_weatherTime = this->getTime("%H:%M");
			_weatherSize = 0;
			for (int k = 0; k < (len - 6) / 2; k++)
			{
				if (k >= WEATHER_SIZE)
				{
					break;
				}
				int icon = _incomingData.data[(k * 2) + 6] >> 4;
				int sign = (_incomingData.data[(k * 2) + 6] & 1) ? -1 : 1;
				int temp = ((int)_incomingData.data[(k * 2) + 7]) * sign;
				int dy = this->getDayofWeek() + k;
				_weather[k].day = dy % 7;
				_weather[k].icon = icon;
				_weather[k].temp = temp;
				_weatherSize++;
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
				int signH = (_incomingData.data[(k * 2) + 6] >> 7 & 1) ? -1 : 1;
				int tempH = ((int)_incomingData.data[(k * 2) + 6] & 0x7F) * signH;

				int signL = (_incomingData.data[(k * 2) + 7] >> 7 & 1) ? -1 : 1;
				int tempL = ((int)_incomingData.data[(k * 2) + 7] & 0x7F) * signL;

				_weather[k].high = tempH;
				_weather[k].low = tempL;
			}
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_WEATHER, 2, 0);
			}
		}
		break;
		case 0x8A:
		{
			_weather[0].uv = _incomingData.data[6];
			_weather[0].pressure = (_incomingData.data[7] * 256) + _incomingData.data[8];
		}
		break;
		case 0x7F:
			if (configurationReceivedCallback != nullptr)
			{
				uint8_t hour = _incomingData.data[7];
				uint8_t minute = _incomingData.data[8];
				uint8_t hour2 = _incomingData.data[9];
				uint8_t minute2 = _incomingData.data[10];
				bool enabled = _incomingData.data[6];
				uint32_t slp = ((uint32_t)hour << 24) | ((uint32_t)minute << 16) | ((uint32_t)hour2 << 8) | ((uint32_t)minute2);
				configurationReceivedCallback(CF_SLEEP, enabled, slp);
			}
			break;
		case 0x91:

			if (_incomingData.data[3] == 0xFE)
			{
				_phoneCharging = _incomingData.data[6] == 1;
				_phoneBatteryLevel = _incomingData.data[7];
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_PBAT, _incomingData.data[6], _phoneBatteryLevel);
				}
			}

			break;
		case 0x93:
			this->setTime(_incomingData.data[13], _incomingData.data[12], _incomingData.data[11], _incomingData.data[10], _incomingData.data[9], _incomingData.data[7] * 256 + _incomingData.data[8]);

			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_TIME, 0, 0);
			}
			break;
		case 0x9C:
			if (configurationReceivedCallback != nullptr)
			{
				uint32_t color = ((uint32_t)_incomingData.data[5] << 16) | ((uint32_t)_incomingData.data[6] << 8) | (uint32_t)_incomingData.data[7];
				uint32_t select = ((uint32_t)(_incomingData.data[8]) << 16) | (uint32_t)_incomingData.data[9];
				configurationReceivedCallback(CF_FONT, color, select);
			}
			break;
		case 0xA2:
		{
			int pos = _incomingData.data[5];
			_contacts[pos].name = "";
			for (int i = 6; i < len; i++)
			{
				_contacts[pos].name += (char)_incomingData.data[i];
			}
		}
		break;
		case 0xA3:
		{
			int pos = _incomingData.data[5];
			int nSize = _incomingData.data[6];
			_contacts[pos].number = "";
			for (int i = 7; i < len; i++)
			{
				char digit[3];
				sprintf(digit, "%02X", _incomingData.data[i]);
				// reverse characters
				digit[2] = digit[0]; // save digit at 0 to 2
				digit[0] = digit[1]; // swap 1 to 0
				digit[1] = digit[2]; // swap saved 2 to 1
				digit[2] = 0;		 // null termination character
				_contacts[pos].number += digit;
			}
			_contacts[pos].number.replace("A", "+");
			_contacts[pos].number = _contacts[pos].number.substring(0, nSize);

			if (configurationReceivedCallback != nullptr && pos == (_contactSize - 1))
			{
				configurationReceivedCallback(CF_CONTACT, 1, uint32_t(_sosContact << 8) | uint32_t(_contactSize));
			}
		}
		break;
		case 0xA5:
			_sosContact = _incomingData.data[6];
			_contactSize = _incomingData.data[7];
			if (configurationReceivedCallback != nullptr)
			{
				configurationReceivedCallback(CF_CONTACT, 0, uint32_t(_sosContact << 8) | uint32_t(_contactSize));
			}
			break;
		case 0xA8:
			if (_incomingData.data[3] == 0xFE)
			{
				// end of qr data
				int size = _incomingData.data[5]; // number of links received
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_QR, 1, size);
				}
			}
			if (_incomingData.data[3] == 0xFF)
			{
				// receiving qr data
				int index = _incomingData.data[5]; // index of the curent link
				_qrLinks[index] = "";			   // clear existing
				for (int i = 6; i < len; i++)
				{
					_qrLinks[index] += (char)_incomingData.data[i];
				}
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_QR, 0, index);
				}
			}
			break;
		case 0xBF:
			if (_incomingData.data[3] == 0xFE)
			{
				_touch.state = _incomingData.data[5] == 1;
				_touch.x = uint32_t(_incomingData.data[6] << 8) | uint32_t(_incomingData.data[7]);
				_touch.y = uint32_t(_incomingData.data[8] << 8) | uint32_t(_incomingData.data[9]);
			}
			break;
		case 0xCA:
			if (_incomingData.data[3] == 0xFE)
			{
				_appCode = (_incomingData.data[6] * 256) + _incomingData.data[7];
				_appVersion = "";
				for (int i = 8; i < len; i++)
				{
					_appVersion += (char)_incomingData.data[i];
				}
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_APP, _appCode, 0);
				}
				_sendESP = true;
			}
			break;
		case 0xCC:
			if (_incomingData.data[3] == 0xFE)
			{
				setChunkedTransfer(_incomingData.data[5] != 0x00);
			}
			break;
		case 0xEE:
			if (_incomingData.data[3] == 0xFE)
			{
				// navigation icon data received
				uint8_t pos = _incomingData.data[6];
				uint32_t crc = uint32_t(_incomingData.data[7] << 24) | uint32_t(_incomingData.data[8] << 16) | uint32_t(_incomingData.data[9] << 8) | uint32_t(_incomingData.data[10]);
				for (int i = 0; i < 96; i++)
				{
					_navigation.icon[i + (96 * pos)] = _incomingData.data[11 + i];
				}

				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_NAV_ICON, pos, crc);
				}
			}
			break;
		case 0xEF:
			if (_incomingData.data[3] == 0xFE)
			{
				// navigation data received
				if (_incomingData.data[5] == 0x00)
				{
					_navigation.active = false;
					_navigation.eta = "Navigation";
					_navigation.title = "Chronos";
					_navigation.duration = "Inactive";
					_navigation.distance = "";
					_navigation.directions = "Start navigation on Google maps";
					_navigation.hasIcon = false;
					_navigation.isNavigation = false;
					_navigation.iconCRC = 0xFFFFFFFF;
				}
				else if (_incomingData.data[5] == 0xFF)
				{
					_navigation.active = true;
					_navigation.title = "Chronos";
					_navigation.duration = "Disabled";
					_navigation.distance = "";
					_navigation.eta = "Navigation";
					_navigation.directions = "Check Chronos app settings";
					_navigation.hasIcon = false;
					_navigation.isNavigation = false;
					_navigation.iconCRC = 0xFFFFFFFF;
				}
				else if (_incomingData.data[5] == 0x80)
				{
					_navigation.active = true;
					_navigation.hasIcon = _incomingData.data[6] == 1;
					_navigation.isNavigation = _incomingData.data[7] == 1;
					_navigation.iconCRC = uint32_t(_incomingData.data[8] << 24) | uint32_t(_incomingData.data[9] << 16) | uint32_t(_incomingData.data[10] << 8) | uint32_t(_incomingData.data[11]);

					int i = 12;
					_navigation.title = "";
					while (_incomingData.data[i] != 0 && i < len)
					{
						_navigation.title += char(_incomingData.data[i]);
						i++;
					}
					i++;

					_navigation.duration = "";
					while (_incomingData.data[i] != 0 && i < len)
					{
						_navigation.duration += char(_incomingData.data[i]);
						i++;
					}
					i++;

					_navigation.distance = "";
					while (_incomingData.data[i] != 0 && i < len)
					{
						_navigation.distance += char(_incomingData.data[i]);
						i++;
					}
					i++;

					_navigation.eta = "";
					while (_incomingData.data[i] != 0 && i < len)
					{
						_navigation.eta += char(_incomingData.data[i]);
						i++;
					}
					i++;

					_navigation.directions = "";
					while (_incomingData.data[i] != 0 && i < len)
					{
						_navigation.directions += char(_incomingData.data[i]);
						i++;
					}
					i++;
				}
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_NAV_DATA, _navigation.active ? 1 : 0, 0);
				}
			}
			break;
		}
	}
	else if (_incomingData.data[0] == 0xEA)
	{
		if (_incomingData.data[4] == 0x7E)
		{
			switch (_incomingData.data[5])
			{
			case 0x01:
			{
				String city = "";
				for (int c = 7; c < len; c++)
				{
					city += (char)_incomingData.data[c];
				}
				_weatherCity = city;
				if (configurationReceivedCallback != nullptr)
				{
					configurationReceivedCallback(CF_WEATHER, 0, 1);
				}
			}
			break;
			case 0x02:
			{
				int size = _incomingData.data[6];
				int hour = _incomingData.data[7];
				for (int z = 0; z < size; z++)
				{
					if (hour + z >= FORECAST_SIZE)
					{
						break;
					}
					int icon = _incomingData.data[8 + (6 * z)] >> 4;
					int sign = (_incomingData.data[8 + (6 * z)] & 1) ? -1 : 1;
					int temp = ((int)_incomingData.data[9 + (6 * z)]) * sign;

					_hourlyForecast[hour + z].day = this->getDayofYear();
					_hourlyForecast[hour + z].hour = hour + z;
					_hourlyForecast[hour + z].wind = (_incomingData.data[10 + (6 * z)] * 256) + _incomingData.data[11 + (6 * z)];
					_hourlyForecast[hour + z].humidity = _incomingData.data[12 + (6 * z)];
					_hourlyForecast[hour + z].uv = _incomingData.data[13 + (6 * z)];
					_hourlyForecast[hour + z].icon = icon;
					_hourlyForecast[hour + z].temp = temp;
				}
			}
			break;
			}
		}
	}
}
