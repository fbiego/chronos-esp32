/*
   MIT License

  Copyright (c) 2025 Felix Biego

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

ChronosESP32 watch("Chronos Health"); // set the bluetooth name

bool send_health = false; // flag to send health data
bool send_sleep = false;  // flag to send sleep data

HealthRequest measureType;
bool measure_run = false;

void connectionCallback(bool state)
{
  Serial.print("Connection state: ");
  Serial.println(state ? "Connected" : "Disconnected");
}

void healthRequestCallback(HealthRequest request, bool state)
{

  switch (request)
  {
  case HR_STEPS_RECORDS:
  {
    Serial.println("Steps records request");
    send_health = true;
  }
  break;
  case HR_SLEEP_RECORDS:
    Serial.println("Sleep records request");
    send_sleep = true;
    break;
  case HR_MEASURE_ALL:
  case HR_HEART_RATE_MEASURE:
  case HR_BLOOD_OXYGEN_MEASURE:
  case HR_BLOOD_PRESSURE_MEASURE:
    measureType = request;
    measure_run = state;
    Serial.print("Health realtime request ");
    Serial.print(measureType);
    Serial.print("\t");
    Serial.println(state ? "Start" : "Stop");
    break;
  }
}

void setup()
{
  Serial.begin(115200);

  // set the callbacks before calling begin funtion
  watch.setConnectionCallback(connectionCallback);
  watch.setHealthRequestCallback(healthRequestCallback);

  watch.begin(); // initializes the BLE
  // make sure the ESP32 is not paired with your phone in the bluetooth settings
  // go to Chronos app > Watches tab > Watches button > Pair New Devices > Search > Select your board
  // you only need to do it once. To disconnect, click on the rotating icon (Top Right)

  Serial.println(watch.getAddress()); // mac address, call after begin()

  watch.setBattery(80); // set the battery level, will be synced to the app

  // watch.clearNotifications(); // clear the default notification (Chronos app install text)

  watch.set24Hour(true); // the 24 hour mode will be overwritten when the command is received from the app
}

void loop()
{
  watch.loop(); // handles internal routine functions

  String time = watch.getHourC() + watch.getTime(":%M ") + watch.getAmPmC();
  Serial.println(time);
  delay(5000);

  // update steps and calories when values changes
  // watch.sendRealtimeSteps(steps, calories); // send realtime steps & calories

  if (send_health)
  {
    // send health data
    uint32_t steps = random(1000, 3000);
    uint32_t calories = random(500, 800);
    watch.sendRealtimeSteps(steps, calories); // send realtime steps & calories
    Serial.printf("Steps: %d, Calories: %d\n", steps, calories);

    // steps and calories are grouped by hour. They are also cumulative throughout the day.
    // ie if steps at 10am is 100 and 11am is 200, (the 11am record will be previous + current 100 + 200 = 300)
    watch.sendStepsRecord(204, 23, 12, watch.getDay(), watch.getMonth() + 1, watch.getYear(), 72, 98, 120, 82);   // send steps records
    watch.sendStepsRecord(646, 45, 14, watch.getDay(), watch.getMonth() + 1, watch.getYear(), 73, 99, 126, 78);   // send steps records
    watch.sendStepsRecord(2345, 69, 14, watch.getDay(), watch.getMonth() + 1, watch.getYear(), 76, 96, 110, 70);  // send steps records
    watch.sendStepsRecord(5654, 124, 15, watch.getDay(), watch.getMonth() + 1, watch.getYear(), 75, 97, 114, 76); // send steps records

    // heart rate records
    watch.sendHeartRateRecord(78, 30, 11, watch.getDay(), watch.getMonth() + 1, watch.getYear());
    watch.sendHeartRateRecord(82, 5, 12, watch.getDay(), watch.getMonth() + 1, watch.getYear());
    watch.sendHeartRateRecord(86, 20, 12, watch.getDay(), watch.getMonth() + 1, watch.getYear());

    // blood pressure records
    watch.sendBloodPressureRecord(112, 76, 30, 11, watch.getDay(), watch.getMonth() + 1, watch.getYear());
    watch.sendBloodPressureRecord(120, 84, 5, 12, watch.getDay(), watch.getMonth() + 1, watch.getYear());
    watch.sendBloodPressureRecord(118, 79, 20, 12, watch.getDay(), watch.getMonth() + 1, watch.getYear());

    // blood oxygen records
    watch.sendBloodOxygenRecord(96, 30, 11, watch.getDay(), watch.getMonth() + 1, watch.getYear());
    watch.sendBloodOxygenRecord(98, 5, 12, watch.getDay(), watch.getMonth() + 1, watch.getYear());
    watch.sendBloodOxygenRecord(97, 20, 12, watch.getDay(), watch.getMonth() + 1, watch.getYear());

    // temperature records
    watch.sendTemperatureRecord(35.6, 30, 11, watch.getDay(), watch.getMonth() + 1, watch.getYear());
    watch.sendTemperatureRecord(37.2, 5, 12, watch.getDay(), watch.getMonth() + 1, watch.getYear());
    watch.sendTemperatureRecord(37.6, 20, 12, watch.getDay(), watch.getMonth() + 1, watch.getYear());

    send_health = false;
  }

  if (send_sleep)
  {
    // sleep records sample
    watch.sendSleepRecord(90, SLEEP_LIGHT, 0, 21, watch.getDay() - 1, watch.getMonth() + 1, watch.getYear());
    watch.sendSleepRecord(330, SLEEP_DEEP, 30, 22, watch.getDay() - 1, watch.getMonth() + 1, watch.getYear());
    watch.sendSleepRecord(60, SLEEP_LIGHT, 0, 4, watch.getDay(), watch.getMonth() + 1, watch.getYear());
    watch.sendSleepRecord(120, SLEEP_DEEP, 0, 5, watch.getDay(), watch.getMonth() + 1, watch.getYear());

    send_sleep = false;
  }

  if (measure_run)
  {
    // simulate running measurements, ideally the values would be retrieved from a sensor
    uint8_t hr = random(72, 84);
    uint8_t sp = random(95, 100);
    uint8_t bpH = random(110, 130);
    uint8_t bpL = random(75, 95);

    switch (measureType)
    {
    case HR_MEASURE_ALL:
      watch.sendRealtimeHealthData(hr, sp, bpH, bpL);
      break;
    case HR_HEART_RATE_MEASURE:
      watch.sendRealtimeHeartRate(hr);
      break;
    case HR_BLOOD_OXYGEN_MEASURE:
      watch.sendRealtimeBloodOxygen(sp);
      break;
    case HR_BLOOD_PRESSURE_MEASURE:
      watch.sendRealtimeBloodPressure(bpH, bpL);
      break;
    }
  }
}