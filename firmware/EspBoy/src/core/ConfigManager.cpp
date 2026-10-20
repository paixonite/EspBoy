#include "ConfigManager.h"

const unsigned long ConfigManager::SLEEP_TIMEOUTS_MS[ConfigManager::SLEEP_OPTION_COUNT] = {
    0,       // 0: OFF
    30000,   // 1: 30 seconds
    60000,   // 2: 1 minute
    180000,  // 3: 3 minutes
    300000   // 4: 5 minutes
};

ConfigManager::ConfigManager()
    : _volume(80),
      _lastNonZeroVolume(80),
      _brightness(100),
      _sleepTimeoutIndex(2),
      _language(LANG_EN) {
}

void ConfigManager::begin() {
    Preferences prefs;
    prefs.begin("system_cfg", true);
    _volume = prefs.getInt("volume", 80);
    _lastNonZeroVolume = prefs.getInt("last_vol", 80);
    _brightness = prefs.getInt("brightness", 100);
    _sleepTimeoutIndex = prefs.getInt("sleep_idx", 2);
    int storedLanguage = prefs.getInt("lang", LANG_EN);
    prefs.end();

    if (_volume < 0) _volume = 0;
    if (_volume > 100) _volume = 100;
    if (_lastNonZeroVolume < 10) _lastNonZeroVolume = 80;
    if (_lastNonZeroVolume > 100) _lastNonZeroVolume = 100;
    if (_brightness < 10) _brightness = 10;
    if (_brightness > 100) _brightness = 100;
    if (_sleepTimeoutIndex < 0 || _sleepTimeoutIndex >= SLEEP_OPTION_COUNT) {
        _sleepTimeoutIndex = 2;
    }

    _language = (storedLanguage >= 0 && storedLanguage < LANG_COUNT) ? (Language)storedLanguage : LANG_EN;
    I18n::setLanguage(_language);

    applyBrightness();
}

int ConfigManager::getVolume() const {
    return _volume;
}

void ConfigManager::setVolume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 100) vol = 100;
    _volume = vol;

    Preferences prefs;
    prefs.begin("system_cfg", false);
    prefs.putInt("volume", _volume);
    if (_volume > 0) {
        _lastNonZeroVolume = _volume;
        prefs.putInt("last_vol", _lastNonZeroVolume);
    }
    prefs.end();
}

void ConfigManager::toggleMute() {
    if (_volume > 0) {
        setVolume(0);
    } else {
        setVolume(_lastNonZeroVolume > 0 ? _lastNonZeroVolume : 80);
    }
}

bool ConfigManager::isMuted() const {
    return _volume == 0;
}

int ConfigManager::getBrightness() const {
    return _brightness;
}

void ConfigManager::setBrightness(int val) {
    if (val < 10) val = 10;
    if (val > 100) val = 100;
    _brightness = val;

    applyBrightness();

    Preferences prefs;
    prefs.begin("system_cfg", false);
    prefs.putInt("brightness", _brightness);
    prefs.end();
}

int ConfigManager::calculateBrightnessPwm(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    // Squared curve compensates for the eye's non-linear brightness perception.
    // Range: ~42 (10%) up to 254 (100%), so the screen never goes fully dark while awake.
    float normalized = percent / 100.0f;
    return 40 + (int)(214.0f * normalized * normalized);
}

void ConfigManager::applyBrightness() const {
    analogWrite(TFT_BL_PIN, calculateBrightnessPwm(_brightness));
}

void ConfigManager::applyDimmedBrightness() const {
    analogWrite(TFT_BL_PIN, calculateBrightnessPwm(10)); // Exactly 10% brightness
}

void ConfigManager::setBacklightRaw(uint8_t pwm) const {
    analogWrite(TFT_BL_PIN, pwm);
}

int ConfigManager::getSleepTimeoutIndex() const {
    return _sleepTimeoutIndex;
}

void ConfigManager::setSleepTimeoutIndex(int idx) {
    if (idx < 0) idx = SLEEP_OPTION_COUNT - 1;
    if (idx >= SLEEP_OPTION_COUNT) idx = 0;
    _sleepTimeoutIndex = idx;

    Preferences prefs;
    prefs.begin("system_cfg", false);
    prefs.putInt("sleep_idx", _sleepTimeoutIndex);
    prefs.end();
}

unsigned long ConfigManager::getSleepTimeoutMs() const {
    return SLEEP_TIMEOUTS_MS[_sleepTimeoutIndex];
}

Language ConfigManager::getLanguage() const {
    return _language;
}

void ConfigManager::setLanguage(Language lang) {
    if (lang >= LANG_COUNT) lang = LANG_EN;
    _language = lang;
    I18n::setLanguage(lang);

    Preferences prefs;
    prefs.begin("system_cfg", false);
    prefs.putInt("lang", _language);
    prefs.end();
}

void ConfigManager::resetAllHighscores(const char* const gameNames[], int gameCount) {
    Preferences prefs;
    for (int i = 0; i < gameCount; i++) {
        String ns = String(gameNames[i]);
        ns.replace(" ", "");

        prefs.begin(ns.c_str(), false);
        prefs.clear();
        prefs.end();
    }
}
