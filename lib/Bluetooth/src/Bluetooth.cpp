#include "Arduino.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLE2902.h>
#include <Helper.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <cassert>
#include <sstream>
#include "Bluetooth.h"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_AXIS "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_COMAND "aaa2fb22-9e7a-11ed-a8fc-0242ac120002"


BLECharacteristic *pCharacteristic;
BLECharacteristic *pBatteryLevelCharacteristic;
BLEServer *pServer;
bool deviceConnected = false;
float x, y, z = 0.0f;
float xtrim, ytrim, ztrim = 0.0f;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        pServer->getAdvertising()->start();
    }
};

class MyCallbackHandler : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();
        Serial.println(rxValue.c_str());
    }

    void onWrite(BLECharacteristic *pCharacteristic)
    {
        if (pCharacteristic->getUUID().toString() == CHARACTERISTIC_UUID_AXIS)
        {
            std::string rxValue = pCharacteristic->getValue();
            int len = rxValue.length();

            if (rxValue.length() > 0)
            {

                String xval = getValue(rxValue.c_str(), ' ', 0);
                String yval = getValue(rxValue.c_str(), ' ', 1);
                String zval = getValue(rxValue.c_str(), ' ', 2);

                Serial.println(xval.toFloat());
                x = xval.toFloat() - xtrim;
                y = yval.toFloat() - ytrim;
                z = zval.toFloat() - ztrim;
            }
        }
        if (pCharacteristic->getUUID().toString() == CHARACTERISTIC_UUID_COMAND)
        {
            std::string rxValue = pCharacteristic->getValue();
            int len = rxValue.length();
            Serial.println(rxValue.c_str());
            if (rxValue.length() > 0)
            {
                xtrim = x;
                ytrim = y;
                ztrim = z;
            }
        }
    }
    String getValue(String data, char separator, int index)
    {
        int found = 0;
        int strIndex[] = {0, -1};
        int maxIndex = data.length() - 1;

        for (int i = 0; i <= maxIndex && found <= index; i++)
        {
            if (data.charAt(i) == separator || i == maxIndex)
            {
                found++;
                strIndex[0] = strIndex[1] + 1;
                strIndex[1] = (i == maxIndex) ? i + 1 : i;
            }
        }
        return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
    }
};

class MySecurity : public BLESecurityCallbacks
{

    bool onConfirmPIN(uint32_t pin)
    {
        return false;
    }

    uint32_t onPassKeyRequest()
    {
        Serial.println("PassKeyRequest");
        return 0000;
    }

    void onPassKeyNotify(uint32_t pass_key)
    {
        Serial.println("On passkey Notify number: " + pass_key);
    }

    bool onSecurityRequest()
    {
        Serial.println("On Security Request");
        return true;
    }

    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
    {
        Serial.println("Starting BLE work!");
        if (!cmpl.success)
        {
            Serial.println("BFL Failure : " + cmpl.fail_reason);
        }
    }
};

void Bluetooth::setup()
{
    BLEDevice::init("iFLY");
    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    BLEDevice::setSecurityCallbacks(new MySecurity());

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_AXIS,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);

    BLEDescriptor Descriptor(CHARACTERISTIC_UUID_AXIS);
    Descriptor.setValue("AXIS");
    pCharacteristic->addDescriptor(&Descriptor);
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new MyCallbackHandler());
    pService->addCharacteristic(pCharacteristic);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_COMAND,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);
    BLEDescriptor DescriptorCommand(CHARACTERISTIC_UUID_COMAND);
    DescriptorCommand.setValue("COMMAND");
    pCharacteristic->addDescriptor(&DescriptorCommand);
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new MyCallbackHandler());
    pService->addCharacteristic(pCharacteristic);

    pBatteryLevelCharacteristic = pService->createCharacteristic(
        BLEUUID((uint16_t)0x2A19),
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);

    BLEDescriptor BatteryLevelDescriptor(BLEUUID((uint16_t)0x2901));
    BatteryLevelDescriptor.setValue("Percentage 0 - 100");
    pBatteryLevelCharacteristic->addDescriptor(&BatteryLevelDescriptor);
    pBatteryLevelCharacteristic->addDescriptor(new BLE2902());
    pService->addCharacteristic(pBatteryLevelCharacteristic);

    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    pServer->getAdvertising()->addServiceUUID(SERVICE_UUID);
    pServer->getAdvertising()->start();

    BLESecurity *pSecurity = new BLESecurity();
    // IO capability устанавливается в No Input No Output:
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
    // ESP_IO_CAP_OUT         0   /*!< только отображение */
    // ESP_IO_CAP_IO          1   /*!< отображение Yes/No */
    // ESP_IO_CAP_IN          2   /*!< только клавиатура */
    // ESP_IO_CAP_NONE        3   /*!< NoInput, NoOutput: ни ввода, ни вывода */
    // ESP_IO_CAP_KBDISP      4   /*!< клавиатура и экран */
    // Привязка (bonding) с устройством пира после аутентификации:
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND;
    uint8_t key_size = 16; // размер ключа должен быть 7 .. 16 байт
    // Инициатор распространяет ключи LTK и IRK установкой масок EncKey и IdKey.
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    // Устройство responder распространяет ключи LTK и IRK установкой масок EncKey и IdKey.
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
}

void Bluetooth::loop()
{
}
void Bluetooth::setDataFly(DataFly &data)
{
    data.x = x;
    data.y = y;
    data.z = z;
    data.xtrim = xtrim;
    data.ytrim = ytrim;
    data.ztrim = ztrim;
}

void Bluetooth::setBatteryLevel(int level)
{
    if (deviceConnected)
    {

        pBatteryLevelCharacteristic->setValue(level);
        pBatteryLevelCharacteristic->notify();
        // pCharacteristic->setValue(&value, 1);
        // pCharacteristic->notify();
    }
}

bool Bluetooth::getDeviceConnected()
{
    return deviceConnected;
}
