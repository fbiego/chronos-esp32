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

#include <Chronos.h>

Chronos watch;
// Chronos watch("Chronos Watch"); // set the bluetooth name

void connectionCallback(bool state)
{
  Serial.print("Connection state: ");
  Serial.println(state ? "Connected" : "Disconnected");
  // bool connected = watch.isConnected();
}

void notificationCallback(Notification notification)
{
  Serial.print("Notification received at ");
  Serial.println(notification.time);
  Serial.print("From: ");
  Serial.print(notification.app);
  Serial.print("\t Icon->");
  Serial.println(notification.icon);
  Serial.println(notification.message);
  // see loop on how to access notifications
}

void configCallback(Config config, uint32_t a, uint32_t b)
{
  switch (config)
  {
  case CF_TIME:
    // time is saved internally
    // command with no parameters
    Serial.println("The time has been set");
    Serial.println(watch.getTimeDate());
    break;
  case CF_RTW:
    // state not saved internally
    Serial.print("Raise to wake: ");
    Serial.println(b ? "ON" : "OFF");
    break;
  case CF_RST:
    // command with no parameters
    Serial.println("Reset request");
    break;
  case CF_FIND:
    // command with no parameters
    Serial.println("Find request");
    break;
  case CF_FONT:
    // state not saved internally
    Serial.print("Font settings: Color ");
    Serial.printf("0x%06X", a);
    Serial.print("\tStyle: ");
    Serial.print((b >> 16) & 0xFFFF);
    Serial.print("\tPosition: ");
    Serial.println(b & 0xFFFF);
    break;
  case CF_ALARM:
    // alarms are saved on the alarms
    // see loop for how to access alarms
    Serial.print("Alarm: Index ");
    Serial.print(a);
    Serial.print("\tTime: ");
    Serial.print(uint8_t(b >> 24));
    Serial.print(":");
    Serial.print(uint8_t(b >> 16));
    Serial.print("\tRepeat: ");
    Serial.print(uint8_t(b >> 8));
    Serial.print("\tEnabled: ");
    Serial.println((uint8_t(b)) ? "ON" : "OFF");
    break;
  case CF_QUIET:
    // state not saved internally
    Serial.print("Quiet hours: ");
    Serial.print(a ? "ON" : "OFF");
    Serial.print("\tTime: ");
    Serial.print(uint8_t(b >> 24));
    Serial.print(":");
    Serial.print(uint8_t(b >> 16));
    Serial.print("\tto\t");
    Serial.print(uint8_t(b >> 8));
    Serial.print(":");
    Serial.println((uint8_t(b)));
    break;
  case CF_SLEEP:
    // state not saved internally
    Serial.print("Sleep time: ");
    Serial.print(a ? "ON" : "OFF");
    Serial.print("\tTime: ");
    Serial.print(uint8_t(b >> 24));
    Serial.print(":");
    Serial.print(uint8_t(b >> 16));
    Serial.print("\tto\t");
    Serial.print(uint8_t(b >> 8));
    Serial.print(":");
    Serial.println((uint8_t(b)));
    break;
  case CF_SED:
    // state not saved internally
    Serial.print("Sedentary time: ");
    Serial.print((a & 0xFF) ? "ON" : "OFF");
    Serial.print("\tInterval: ");
    Serial.print(((a >> 16) & 0xFFFF));
    Serial.print("\tTime: ");
    Serial.print(uint8_t(b >> 24));
    Serial.print(":");
    Serial.print(uint8_t(b >> 16));
    Serial.print("\tto\t");
    Serial.print(uint8_t(b >> 8));
    Serial.print(":");
    Serial.println((uint8_t(b)));
    break;
  case CF_WATER:
    // state not saved internally
    Serial.print("Drink water reminder: ");
    Serial.print((a & 0xFF) ? "ON" : "OFF");
    Serial.print("\tInterval: ");
    Serial.print(((a >> 16) & 0xFFFF));
    Serial.print("\tTime: ");
    Serial.print(uint8_t(b >> 24));
    Serial.print(":");
    Serial.print(uint8_t(b >> 16));
    Serial.print("\tto\t");
    Serial.print(uint8_t(b >> 8));
    Serial.print(":");
    Serial.println((uint8_t(b)));
    break;
  case CF_USER:
    // state not saved internally
    Serial.print("User info: Age: ");
    Serial.print(uint8_t(a >> 24));
    Serial.print("\tHeight: ");
    Serial.print(uint8_t(a >> 16));
    Serial.print("\tWeight: ");
    Serial.print(uint8_t(a >> 8));
    Serial.print("\tStep length: ");
    Serial.print(uint8_t(a));
    Serial.print("\tUnits: ");
    Serial.print(uint8_t(b >> 24) ? "Metric" : "Imperial");
    Serial.print("\tTarget steps: ");
    Serial.print(uint8_t(b >> 16) * 1000);
    Serial.print("\tTemp: ");
    Serial.println(uint8_t(b >> 8) ? "°F" : "°C");
    break;
  case CF_HOURLY:
    // state not saved internally
    Serial.print("Hourly measurement: ");
    Serial.println(b ? "ON" : "OFF");
    break;
  case CF_HR24:
    // state is saved internally
    // bool hr24 =  watch.is24Hour(); // to access outside the callback
    Serial.print("24 hour mode: ");
    Serial.println(b ? "ON" : "OFF");
    break;
  case CF_LANG:
    // state not saved internally
    Serial.print("Language: ");
    Serial.println(b);
    break;
  case CF_WEATHER:
    // weather is saved
    Serial.print("Weather received: ");
    if (a)
    {
      Serial.print("Forecast");
      // see loop for weather details
    }
    if (b)
    {
      Serial.print("City name: ");
      String city = watch.getWeatherCity(); //
      Serial.print(city);
    }
    Serial.println();
    break;
  }
}

void setup()
{
  Serial.begin(115200);

  watch.setLogging(true); // to view the raw received data over BLE
  watch.setConnectionCallback(connectionCallback);
  watch.setNotificationCallback(notificationCallback);
  watch.setConfigurationCallback(configCallback);

  watch.begin(); // initializes the BLE
  // go to Chronos app > Watches tab > Watches button > Pair New Devices > Search > Select your board
  // you only need to do it once. To disconnect, click on the rotating icon

  Serial.println(watch.getAddress()); // mac address, call after begin()
  
  watch.setBattery(80);  // set the battery level, will be synced to the app


  watch.set24Hour(true); // the 24 hour mode will be overwritten when the command is received from the app
  // this modifies the return of 
  watch.getAmPmC(true); // 12 hour mode false->(am/pm), true->(AM/PM), if 24 hour mode returns empty string ("")
  watch.getHourC(); // (0-12), (0-23)
  watch.getHourZ(); // zero padded hour (00-12), (00-23) 
  watch.is24Hour(); // resturns whether in 24 hour mode
}

void loop()
{
  watch.loop(); // handles internal routine functions 

  String time = watch.getHourC() + watch.getTime(":%M ") + watch.getAmPmC();
  Serial.println(time);
  delay(5000);

  /*
  // access available notifications
  int count = watch.getNotificationCount();
  Serial.print("Count: ");
  Serial.println(count);
  for (int i = 0; i < count; i++)
  {
    // iterate through available notifications, index 0 is the latest received notification
    Notification n = watch.getNotificationAt(i);
    Serial.print("Notification received at ");
    Serial.println(n.time);
    Serial.print("From: ");
    Serial.print(n.app);
    Serial.print("\t Icon->");
    Serial.println(n.icon);
    Serial.println(n.message);
  }
  watch.clearNotifications(); // watch.getNotificationCount() will return zero,
  //  notifications in the buffer are still accessible using watch.getNotificationAt(i)
  */

  /*
  // read the alarms, 8 available
  // the alarms are only stored as received from the app
  for (int j = 0; j < 8; j++){
    Alarm a = watch.getAlarm(j);
    Serial.print("Alarm: ");
    Serial.print(j + 1);
    Serial.print("\tTime: ");
    Serial.print(a.hour);
    Serial.print(":");
    Serial.print(a.minute);
    Serial.print("\tState: ");
    Serial.println(a.enabled ? "Enabled": "Disabled");
  }
  */

  /* // access weather forecast details
  int n = watch.getWeatherCount();
  String updateTime = watch.getWeatherTime();
  Serial.print("Weather Count: ");
  Serial.print(n);
  Serial.print("\tUpdated at: ");
  Serial.println(updateTime);

  for (int i = 0; i < n; i++)
  {
    // iterate through available notifications, index 0 is the latest received notification
    Weather w = watch.getWeatherAt(i);
    Serial.print("Day:"); // day of the week (0 - 6)
    Serial.println(w.day);
    Serial.print("\tIcon: ");
    Serial.println(w.icon);
    Serial.print("\t");
    Serial.print(w.temp);
    Serial.println("C");
  }
  */
}