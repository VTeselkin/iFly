#pragma once
#include <Display.h>

#define PIN_BAT_VOLT 4
#define VOLTAGE_MAX 4.1
#define VOLTAGE_MIN 2.8

class Battery
{

public:

    void setup();
    void loop(Display display);
   

private:
    
};