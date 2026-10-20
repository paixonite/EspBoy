#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "pins.h"
#include "../localization/Localization.h"

class ConfigManager {
public:
    static const int SLEEP_OPTION_COUNT = 5;

    ConfigManager();
    void begin();

    // --- Volume (0% to 100%, 0% = OFF) ---
    int getVolume() const;
    void setVolume(int vol);
    void toggleMute();
    bool isMuted() const;

    // --- Brightness (10% to 100%) ---
    int getBrightness() const;
    void setBrightness(int val);
    void applyBrightness() const;
    void applyDimmedBrightness() const;
    void setBacklightRaw(uint8_t pwm) const;
    static int calculateBrightnessPwm(int percent);

    // --- Auto-Sleep Timeout ---
    int getSleepTimeoutIndex() const;
    void setSleepTimeoutIndex(int idx);
    unsigned long getSleepTimeoutMs() const;

    // --- Language ---
    Language getLanguage() const;
    void setLanguage(Language lang);

    // --- Highscore Management ---
    void resetAllHighscores(const char* const gameNames[], int gameCount);

private:
    int _volume;
    int _lastNonZeroVolume;
    int _brightness;
    int _sleepTimeoutIndex;
    Language _language;

    static const unsigned long SLEEP_TIMEOUTS_MS[SLEEP_OPTION_COUNT];
};

#endif // CONFIGMANAGER_H
