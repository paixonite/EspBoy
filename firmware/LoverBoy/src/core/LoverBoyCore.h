#ifndef LOVERBOYCORE_H
#define LOVERBOYCORE_H

#include <TFT_eSPI.h>
#include "ConfigManager.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "BatteryManager.h"
#include "../ui/MenuManager.h"
#include "../games/Game.h"
#include "pins.h"

enum CoreMode { MODE_MENU, MODE_GAME };

class LoverBoyCore {
public:
    LoverBoyCore();
    void begin();
    void update();

private:
    TFT_eSPI tft;
    ConfigManager configManager;
    InputManager inputManager;
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
    void handlePowerManagement();
};

#endif // LOVERBOYCORE_H