#include "BatteryManager.h"

BatteryManager::BatteryManager() {
    lastCheckTime = 0;
    lastAdcValue = 0;
    chargingState = false;
}

void BatteryManager::begin() {
    pinMode(BATTERY_SENSE_PIN, INPUT);
}

int BatteryManager::calculateBatteryPercentage(int adcValue) {
    int percentage = 0;
    if (adcValue >= 2600) {
        percentage = 100;
    } else if (adcValue > 2500) {
        percentage = map(adcValue, 2500, 2600, 80, 100);
    } else if (adcValue > 2300) {
        percentage = map(adcValue, 2300, 2500, 30, 80);
    } else if (adcValue > 2000) {
        percentage = map(adcValue, 2000, 2300, 5, 30);
    } else if (adcValue > 1900) {
        percentage = map(adcValue, 1900, 2000, 0, 5);
    } else {
        percentage = 0;
    }
    return percentage;
}

bool BatteryManager::isCharging() {
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime > 3000) {
        int currentAdcValue = analogRead(BATTERY_SENSE_PIN);
        
        if (currentAdcValue > lastAdcValue && lastAdcValue > 0) {
            chargingState = true;
        } else if (currentAdcValue < lastAdcValue) {
            chargingState = false;
        }
        
        if (currentAdcValue > 2580) {
            chargingState = true;
        }

        lastAdcValue = currentAdcValue;
        lastCheckTime = currentTime;
    }
    return chargingState;
}

int BatteryManager::getPercentage() {
    int currentAdcValue = analogRead(BATTERY_SENSE_PIN);
    return calculateBatteryPercentage(currentAdcValue);
}

String BatteryManager::getStatusString() {
    int percentage = getPercentage();
    // Lógica de isCharging() pode ser reativada aqui futuramente
    return String(percentage) + "%";
}