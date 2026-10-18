#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <TFT_eSPI.h>
#include "SoundManager.h"
#include "BatteryManager.h"
#include "ConfigManager.h"
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
    MenuManager(TFT_eSPI* tft, SoundManager* sound, BatteryManager* battery, ConfigManager* config);
    void begin();
    void returnToMenu();
    int update(); // Returns -1 if still in menu, or the game ID

private:
    TFT_eSPI* tft;
    SoundManager* sound;
    BatteryManager* battery;
    ConfigManager* config;
    
    MenuState currentState;
    unsigned long splashStartTime;
    unsigned long lastBatteryUpdate;
    unsigned long lastNavInputTime;

    // --- Game Carousel Logic ---
    int selectedGameIndex;
    static const int GAME_COUNT = 6;
    // Games are listed in alphabetical order
    const char* gameNames[GAME_COUNT] = {"Crossy Road", "Flappy Bird", "Frogger", "Genius", "Snake", "Tetris"};

    // --- Settings Logic ---
    int selectedSettingIndex;
    int settingsScrollOffset;
    static const int SETTING_COUNT = 4;

    // --- Button States ---
    bool lastLeftState, lastRightState, lastUpState, lastDownState;
    bool lastConfirmState, lastCancelState, lastSelectState;

    // --- Drawing Functions ---
    void drawSplashScreen();
    void drawMenu();
    void updateBatteryDisplay();
    void drawSettings();
    void drawConfirmReset();
    void drawCarouselIndicators();
    void drawTopBar(const String& title, uint16_t color = TFT_YELLOW);
    void drawBottomBar(const String& text, uint16_t color = TFT_WHITE);
    static const int BAR_HEIGHT = 25;

    // --- Input Handlers ---
    int handleSelectInput();
    void handleSettingsInput();
    void handleConfirmResetInput();
};

#endif // MENUMANAGER_H