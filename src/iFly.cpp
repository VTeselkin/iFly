#include "Arduino.h"
#include <Display.h>
#include <Bluetooth.h>
#include <Helper.h>
#include <iFlyWiFi.h>
#include "time.h"
#include <ESP32Servo.h>
#include <Battery.h>
#include <Wire.h>


Display display;
Bluetooth bluetooth;
iFlyWiFi wifi;
Battery batteryLevel;

uint8_t value = 0;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

DataFly dataFly;

void setBatteryLevel();
int calc_battery_percentage(int adc);

uint8_t level = 0;

Servo FLESC; // name the servo object, here ESC
Servo FRESC;
Servo RLESC;
Servo RRESC;

const int flEscPin = 12;
const int frEscPin = 13;
const int rlEscPin = 15;
const int rrEscPin = 32;

void setup(void)
{
  Serial.begin(9600);
  delay(1000);

  FLESC.attach(flEscPin); // Generate PWM in pin 9 of Arduino
  FRESC.attach(frEscPin);
  RLESC.attach(rlEscPin);
  RRESC.attach(rrEscPin);

  display.setup();
  wifi.setup();
  bluetooth.setup();
  batteryLevel.setup();
  delay(10);
  Serial.println();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop()
{
  bluetooth.loop();
  bluetooth.setDataFly(dataFly);
  display.setBluetooth(bluetooth.getDeviceConnected());
  display.setXYZRemote(dataFly);
  batteryLevel.loop(display);
  display.loop();
}


