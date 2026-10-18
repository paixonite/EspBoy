#ifndef ESPBOYCORE_H
#define ESPBOYCORE_H

#include <TFT_eSPI.h>
#include "ConfigManager.h"
#include "SoundManager.h"
#include "BatteryManager.h"
#include "MenuManager.h"
#include "Game.h"
#include "pins.h"

enum CoreMode { MODE_MENU, MODE_GAME };

class EspBoyCore {
public:
    EspBoyCore();
    void begin();
    void update();

private:
    TFT_eSPI tft;
    ConfigManager configManager;
    SoundManager soundManager;
    BatteryManager batteryManager;
    MenuManager menuManager;
    
    CoreMode currentMode;
    Game* currentGame;

    // --- Power & Auto-Sleep State ---
    unsigned long lastActivityTime;
    bool isSleeping;
    bool isDimmed;

    void loadHardware();
    void loadGame(int gameIndex);
    bool anyButtonPressed() const;
    void handlePowerManagement();
};

#endif // ESPBOYCORE_H