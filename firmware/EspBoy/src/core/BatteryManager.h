#ifndef BATTERYMANAGER_H
#define BATTERYMANAGER_H

#include <Arduino.h>
#include "pins.h"

class BatteryManager {
public:
    BatteryManager();
    void begin();
    int getPercentage();
    bool isCharging();
    String getStatusString();

private:
    int calculateBatteryPercentage(int adcValue);
    unsigned long lastCheckTime;
    int lastAdcValue;
    bool chargingState;
};

#endif // BATTERYMANAGER_H