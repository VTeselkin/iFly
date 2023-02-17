#include "Arduino.h"
#include <Display.h>
#include <Bluetooth.h>
#include <Helper.h>
#include <WiFi.h>
#include "time.h"
#include "esp_wpa2.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"

Display display;
Bluetooth bluetooth;


uint8_t value = 0;

const char *ssid = "R4R WIFI";
const char *password = "6XI1krpG";

#define R2 100
#define R3 100
#define VOLTAGE_OUT(Vin) (((Vin)*R3) / (R2 + R3))
#define VOLTAGE_MAX 4200
#define VOLTAGE_MIN 3300
#define ADC_REFERENCE 1100
#define VOLTAGE_TO_ADC(in) ((ADC_REFERENCE * (in)) / 4096)
#define BATTERY_MAX_ADC VOLTAGE_TO_ADC(VOLTAGE_OUT(VOLTAGE_MAX))
#define BATTERY_MIN_ADC VOLTAGE_TO_ADC(VOLTAGE_OUT(VOLTAGE_MIN))

#define EAP_IDENTITY "2080"
#define EAP_PASSWORD "6XI1krpG"
#define EXAMPLE_WIFI_SSID "R4R WIFI"



const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

DataFly dataFly;

void setBatteryLevel();
int calc_battery_percentage(int adc);

uint8_t level = 0;

void setup(void)
{
 

  Serial.begin(9600);
  delay(1000);
  
  display.setup();
  bluetooth.setup();
  delay(10);
  Serial.println();
  Serial.println(ssid);

  // This part of the code is taken from the oficial wpa2_enterprise example from esp-idf
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();

  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
  WiFi.begin(ssid);

 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop()
{
  bluetooth.loop();
  bluetooth.setDataFly(dataFly);
  display.setBluetooth(bluetooth.getDeviceConnected());
  display.setXYZRemote(dataFly);
  setBatteryLevel();
  display.loop();
}

void setBatteryLevel()
{

  float volt = (analogRead(4) * 2 * 3.3) / 4096;
  float value = (int)(volt * 100 + .5);
  level = calc_battery_percentage(analogRead(4));
  display.setBatteryLevel(level);
}

int calc_battery_percentage(int adc)
{
  int battery_percentage = 100 * (adc - BATTERY_MIN_ADC) / (BATTERY_MAX_ADC - BATTERY_MIN_ADC);

  if (battery_percentage < 0)
    battery_percentage = 0;
  if (battery_percentage > 100)
    battery_percentage = 100;

  return battery_percentage;
}