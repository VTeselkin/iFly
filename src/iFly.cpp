#include "Arduino.h"
#include <Display.h>
#include <Bluetooth.h>
#include <Helper.h>
#include <iFlyWiFi.h>
#include <time.h>
#include <DShotRMT.h>
#include <Battery.h>
#include <Wire.h>
#include <Sensor.h>

Display display;
Bluetooth bluetooth;
iFlyWiFi wifi;
Battery batteryLevel;
Sensor mpu9250;

uint8_t value = 0;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

DataFly dataFly;

uint16_t throttle_value = 0x30;

const int flEscPin = 12;
const int frEscPin = 13;
const int rlEscPin = 15;
const int rrEscPin = 32;

DShotRMT FLESC(flEscPin, RMT_CHANNEL_0);
DShotRMT FRESC(frEscPin, RMT_CHANNEL_1);
DShotRMT RLESC(rlEscPin, RMT_CHANNEL_2);
DShotRMT RRESC(rrEscPin, RMT_CHANNEL_3);

void setup(void)
{
  Serial.begin(9600);
  delay(1000);

  FLESC.begin(DSHOT600);
  FRESC.begin(DSHOT600);
  RLESC.begin(DSHOT600);
  RRESC.begin(DSHOT600);

  display.setup();
  wifi.setup();
  bluetooth.setup();
  batteryLevel.setup();
  delay(10);
  Serial.println();
  mpu9250.setup();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  FLESC.send_dshot_value(throttle_value);
  FRESC.send_dshot_value(throttle_value);
  RLESC.send_dshot_value(throttle_value);
  RRESC.send_dshot_value(throttle_value);
}

void loop()
{
  mpu9250.loop(dataFly);
  // стабилизация дрона
  float FLESC_speed_1 = 1500 + dataFly.Pitch * 10 + dataFly.Roll * 10 - dataFly.Yaw * 5;
  float FRESC_speed_2 = 1500 + dataFly.Pitch * 10 - dataFly.Roll * 10 + dataFly.Yaw * 5;
  float RLESC_speed_3 = 1500 - dataFly.Pitch * 10 - dataFly.Roll * 10 - dataFly.Yaw * 5;
  float RRESC_speed_4 = 1500 - dataFly.Pitch * 10 + dataFly.Roll * 10 + dataFly.Yaw * 5;

  // ограничение скорости моторов от 1000 до 2000
  FLESC_speed_1 = constrain(FLESC_speed_1, 1000, 2000);
  FRESC_speed_2 = constrain(FRESC_speed_2, 1000, 2000);
  RLESC_speed_3 = constrain(RLESC_speed_3, 1000, 2000);
  RRESC_speed_4 = constrain(RRESC_speed_4, 1000, 2000);

// управление скоростью моторов по протоколу DSHOT600
  FLESC.send_dshot_value(FLESC_speed_1);
  FRESC.send_dshot_value(FRESC_speed_2);
  RLESC.send_dshot_value(RLESC_speed_3);
  RRESC.send_dshot_value(RRESC_speed_4);

  bluetooth.loop();
  bluetooth.setDataFly(dataFly);
  display.setBluetooth(bluetooth.getDeviceConnected());
  display.setXYZRemote(dataFly);
  batteryLevel.loop(display);
  display.loop();
}
