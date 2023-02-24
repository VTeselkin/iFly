#include <Arduino.h>
#include <Battery.h>

int lastLevel = 0;
bool isCharged = false;

void Battery::setup()
{
    analogReadResolution(12); // разрешение АЦП 12 бит
    analogSetAttenuation(ADC_11db);
    lastLevel = 100;
    isCharged = true;
}

void Battery::loop(Display display)
{

    int adcValue = analogRead(PIN_BAT_VOLT);
    // преобразовать напряжение в проценты заряда батареи
    // 4095 - максимальное значение АЦП, 3.3 - опорное напряжение, 2.0 - коэффициент деления
    float batteryVoltage = adcValue / 4095.0 * 3.3 * 2.0;
    // преобразовать напряжение в проценты заряда батареи
    int level = ((batteryVoltage - VOLTAGE_MIN) / (VOLTAGE_MAX - VOLTAGE_MIN)) * 100;

    if (level > 100)
    {
        level = 100;
    }
    if (level < 0)
    {
        level = 0;
    }
    // вывести уровень заряда батареи
    Serial.printf("Battery level: %d%%\n", level);

    if (batteryVoltage > 4.5f)
    {
        display.setBatteryLevel("Chg");
        isCharged = true;
    }
    else
    {

        if (level <= lastLevel)
        {
            lastLevel = level;
            display.setBatteryLevel(String(level));
        }
        else if (isCharged)
        {
            lastLevel = 100;
            isCharged = false;
            display.setBatteryLevel(String(level));
        }
    }
    display.setSimpleText(String(batteryVoltage), 20, 100);
    display.setSimpleText(String(level), 20, 120);
}