#pragma once
#include "Arduino.h"
#include <Helper.h>
class Bluetooth
{
public:
    void setup();
    void loop();
    void setBatteryLevel(int level);
    bool getDeviceConnected();
    void setDataFly(DataFly& data);

private:

   
};
