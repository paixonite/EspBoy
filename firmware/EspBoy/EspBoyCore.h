#ifndef ESPBOYCORE_H
#define ESPBOYCORE_H

#include <TFT_eSPI.h>
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
    SoundManager soundManager;
    BatteryManager batteryManager;
    MenuManager menuManager;
    
    CoreMode currentMode;
    Game* currentGame;

    void loadHardware();
    void loadGame(int gameIndex);
};

#endif // ESPBOYCORE_H