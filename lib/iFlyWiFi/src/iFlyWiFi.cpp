#include <iFlyWiFi.h>

#define EAP_IDENTITY "2080"
#define EAP_PASSWORD "6XI1krpG"
#define EXAMPLE_WIFI_SSID "R4R WIFI"

const char *ssid = "R4R WIFI";
const char *password = "6XI1krpG";

void iFlyWiFi::setup()
{
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
}

void iFlyWiFi::loop()
{
}