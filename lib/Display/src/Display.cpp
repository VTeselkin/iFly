#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include <Display.h>
#include <Helper.h>
#include <WiFi.h>

#define PIN_POWER_ON 15
#define UP 14
#define DOWN 0

TFT_eSPI lcd = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&lcd);
int brightness = 25;
int32_t counter = 0;

void Display::setup()
{
  Serial.println("Start Init Display");
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);

  lcd.init();
  lcd.fillScreen(TFT_BLACK);
  lcd.setRotation(1);
  sprite.createSprite(320, 170);
  sprite.setTextDatum(3);
  sprite.setSwapBytes(true);
  setBidgest();
}

void Display::loop()
{
  setBidgest();
  setWifiLevel();
  if (WiFi.status() != WL_CONNECTED)
  {
    sprite.setTextSize(1);
    sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
    sprite.drawString("Wait for connection: " + String(counter), 0, 10);
    counter++;
    sprite.pushSprite(0, 0);
    delay(1000);
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
    sprite.drawString("Connection: " + WiFi.SSID(), 0, 10);
  }
  sprite.pushSprite(0, 0);
}

void Display::setBatteryLevel(int level)
{
  sprite.setTextColor(TFT_GREEN, TFT_BLACK);
  sprite.pushImage(290, 1, 29, 16, battery);
  sprite.drawString(String(level), 294, 9);
}

void Display::setBluetooth(bool connected)
{
  if (connected)
  {
    sprite.pushImage(234, 0, 16, 16, ble);
  }
  else
  {
    sprite.fillRect(260, 0, 16, 16, TFT_BLACK);
  }
}

void Display::setBidgest()
{
  if (digitalRead(14) == 0 && brightness < 255)
  {
    brightness += 5;
  }

  if (digitalRead(0) == 0 && brightness > 0)
  {
    brightness -= 5;
  }
  auto seg = brightness / 24;
  sprite.fillRect(304, 30, 12, 136, TFT_BLACK);
  for (int i = 0; i < seg; i++)
  {
    sprite.fillRect(314, 150 - (i * 13), 2, 11, 0x35F9);
  }
  sprite.drawRoundRect(311, 30, 8, 136, 2, TFT_SILVER);
  ledcSetup(0, 10000, 8);
  ledcAttachPin(38, 0);
  ledcWrite(0, brightness);
}

void Display::setWifiLevel()
{
  const uint16_t *data = wifi_no;
  if (WiFi.status() != WL_CONNECTED)
  {
    sprite.pushImage(252, 0, 16, 16, wifi_sign_off);
    sprite.pushImage(272, 2, 16, 16, data);
    sprite.setTextColor(TFT_RED, TFT_BLACK);
    return;
  }
  auto level = WiFi.RSSI();
  if (level < -70)
  {
    data = wifi_lvl1;
  }
  if (level > -70)
  {
    data = wifi_lvl2;
  }
  if (level > -67)
  {
    data = wifi_lvl3;
  }
  if (level > -30)
  {
    data = wifi_lvl4;
  }
  sprite.pushImage(252, 0, 16, 16, wifi_sign_on);
  sprite.pushImage(272, 2, 16, 16, data);
}

void Display::setXYZRemote(DataFly &dataFly)
{
  sprite.setTextSize(2);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.drawString("X = " + String(dataFly.x), 20, 40);
  sprite.drawString("Y = " + String(dataFly.y), 20, 60);
  sprite.drawString("Z = " + String(dataFly.z), 20, 80);
  sprite.setTextSize(1);
}