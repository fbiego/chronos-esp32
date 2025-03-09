/*
   MIT License

  Copyright (c) 2024 Felix Biego

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

#include <ChronosESP32.h>

ChronosESP32 watch("Chronos Nav"); // set the bluetooth name

bool change = false;

void connectionCallback(bool state)
{
    Serial.print("Connection state: ");
    Serial.println(state ? "Connected" : "Disconnected");
}

void notificationCallback(Notification notification)
{
    Serial.print("Notification received at ");
    Serial.println(notification.time);
    Serial.print("From: ");
    Serial.print(notification.app);
    Serial.print("\tIcon: ");
    Serial.println(notification.icon);
    Serial.println(notification.title);
    Serial.println(notification.message);
}

void configCallback(Config config, uint32_t a, uint32_t b)
{
    switch (config)
    {
    case CF_NAV_DATA:
        Serial.print("Navigation state: ");
        Serial.println(a ? "Active" : "Inactive");
        change = true;
        if (a){
            Navigation nav = watch.getNavigation();
            Serial.println(nav.directions);
            Serial.println(nav.eta);
            Serial.println(nav.duration);
            Serial.println(nav.distance);
            Serial.println(nav.title);
        }
        break;
    case CF_NAV_ICON:
        Serial.print("Navigation Icon data, position: ");
        Serial.println(a);
        Serial.print("Icon CRC: ");
        Serial.printf("0x%04X\n", b);
        break;
    }
}

void setup()
{
    Serial.begin(115200);

    // set the callbacks before calling begin funtion
    watch.setConnectionCallback(connectionCallback);
    watch.setNotificationCallback(notificationCallback);
    watch.setConfigurationCallback(configCallback);

    watch.begin(); // initializes the BLE
    // make sure the ESP32 is not paired with your phone in the bluetooth settings
    // go to Chronos app > Watches tab > Watches button > Pair New Devices > Search > Select your board
    // you only need to do it once. To disconnect, click on the rotating icon (Top Right)

    Serial.println(watch.getAddress()); // mac address, call after begin()

    watch.setBattery(80); // set the battery level, will be synced to the app
}

void loop()
{
    watch.loop(); // handles internal routine functions

    // if (change){
    //     change = false;
        
    //     Navigation nav = watch.getNavigation();
    //     if (nav.active){
    //         Serial.println(nav.directions);
    //         Serial.println(nav.eta);
    //         Serial.println(nav.duration);
    //         Serial.println(nav.distance);
    //         Serial.println(nav.title);
    //     }
    // }

}