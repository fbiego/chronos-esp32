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

#include <ChronosESP32.h>

#define BUTTON_PIN 0
#define LED_PIN 2

ChronosESP32 watch;
// ChronosESP32 watch("Chronos Watch"); // set the bluetooth name

volatile bool buttonPressed = false;

void IRAM_ATTR buttonISR()
{
    buttonPressed = true;
}

void connectionCallback(bool state)
{
    Serial.print("Connection state: ");
    Serial.println(state ? "Connected" : "Disconnected");
}

void setup()
{
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    watch.setConnectionCallback(connectionCallback);
    watch.begin();

    Serial.println(watch.getAddress()); // mac address, call after begin()

    watch.setBattery(80); // set the battery level, will be synced to the app

    attachInterrupt(BUTTON_PIN, buttonISR, FALLING);
}

void loop()
{
    watch.loop(); // handles internal routine functions

    digitalWrite(LED_PIN, watch.isConnected()); // use a led to show the connection status

    // String time = watch.getHourC() + watch.getTime(":%M ") + watch.getAmPmC();
    // Serial.println(time);
    // delay(5000);

    if (buttonPressed)
    {
        buttonPressed = false;
        // music control,
        watch.musicControl(MUSIC_TOGGLE); // MUSIC_PLAY, MUSIC_PAUSE, MUSIC_PREVIOUS, MUSIC_NEXT, MUSIC_TOGGLE, VOLUME_UP, VOLUME_DOWN, VOLUME_MUTE
        Serial.println("Sent music command");

        // watch.setVolume(50); // set the music volume [0-100] (experimental)

        // find phone
        // watch.findPhone(true); // true -> ring the phone, false -> stop the ringing, the ringing will also be stopped automatically after 30 seconds;
    }
}