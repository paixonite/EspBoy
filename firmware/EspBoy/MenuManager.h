#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <TFT_eSPI.h>
#include "SoundManager.h"
#include "BatteryManager.h"
#include "pins.h"

enum MenuState { MENU_STATE_SPLASH, MENU_STATE_SELECT };

class MenuManager {
public:
    MenuManager(TFT_eSPI* tft, SoundManager* sound, BatteryManager* battery);
    void begin();
    void returnToMenu();
    int update(); // Retorna -1 se continua no menu, ou o índice do jogo para carregar

private:
    TFT_eSPI* tft;
    SoundManager* sound;
    BatteryManager* battery;
    
    MenuState currentState;
    unsigned long splashStartTime;
    unsigned long lastBatteryUpdate;
    unsigned long lastNavInputTime;

    int selectedGameIndex;
    static const int GAME_COUNT = 2;
    const char* gameNames[GAME_COUNT] = {"Snake", "Flappy Bird"};

    bool lastLeftState, lastRightState, lastConfirmState, lastMuteActionState;

    void drawSplashScreen();
    void drawMenu();
    void updateBatteryDisplay();
    int handleInput();
};

#endif // MENUMANAGER_H