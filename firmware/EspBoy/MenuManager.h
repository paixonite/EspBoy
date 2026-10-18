#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <TFT_eSPI.h>
#include "SoundManager.h"
#include "BatteryManager.h"
#include "pins.h"

// Added new states for Settings Menu
enum MenuState { 
    MENU_STATE_SPLASH, 
    MENU_STATE_SELECT, 
    MENU_STATE_SETTINGS, 
    MENU_STATE_CONFIRM_RESET 
};

class MenuManager {
public:
    MenuManager(TFT_eSPI* tft, SoundManager* sound, BatteryManager* battery);
    void begin();
    void returnToMenu();
    int update(); // Returns -1 if still in menu, or the game ID

private:
    TFT_eSPI* tft;
    SoundManager* sound;
    BatteryManager* battery;
    
    MenuState currentState;
    unsigned long splashStartTime;
    unsigned long lastBatteryUpdate;
    unsigned long lastNavInputTime;

    // --- Game Carousel Logic ---
    int selectedGameIndex;
    static const int GAME_COUNT = 5;
    const char* gameNames[GAME_COUNT] = {"Snake", "Flappy Bird", "Crossy Road", "Frogger", "Tetris"};

    // --- Settings Logic ---
    int selectedSettingIndex;
    static const int SETTING_COUNT = 3;
    int currentBrightness; // Hook for future PWM brightness (10 to 100)

    // --- Button States ---
    bool lastLeftState, lastRightState, lastUpState, lastDownState;
    bool lastConfirmState, lastCancelState, lastSelectState;

    // --- Drawing Functions ---
    void drawSplashScreen();
    void drawMenu();
    void updateBatteryDisplay();
    void drawSettings();
    void drawConfirmReset();

    // --- Input Handlers ---
    int handleSelectInput();
    void handleSettingsInput();
    void handleConfirmResetInput();
};

#endif // MENUMANAGER_H