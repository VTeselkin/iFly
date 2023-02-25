#pragma once
#include <Helper.h>

class Sensor
{

public:
    void setup();
    void loop(DataFly data);  

private:
   void getAltitude(DataFly data);
   
};