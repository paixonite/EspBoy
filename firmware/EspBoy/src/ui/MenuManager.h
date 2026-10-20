#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <TFT_eSPI.h>
#include "../core/SoundManager.h"
#include "../core/BatteryManager.h"
#include "../core/ConfigManager.h"
#include "../core/InputManager.h"
#include "../localization/Localization.h"
#include "TextRenderer.h"
#include "../core/pins.h"

// Added new states for Settings Menu
enum MenuState { 
    MENU_STATE_SPLASH, 
    MENU_STATE_SELECT, 
    MENU_STATE_SETTINGS, 
    MENU_STATE_CONFIRM_RESET,
    MENU_STATE_CREDITS
};

class MenuManager {
public:
    MenuManager(TFT_eSPI* tft, SoundManager* sound, BatteryManager* battery, ConfigManager* config, InputManager* input);
    void begin();
    void returnToMenu();
    int update(); // Returns -1 if still in menu, or the game ID

private:
    TFT_eSPI* tft;
    SoundManager* sound;
    BatteryManager* battery;
    ConfigManager* config;
    InputManager* input;
    
    MenuState currentState;
    unsigned long splashStartTime;
    unsigned long lastBatteryUpdate;

    // --- Game Carousel Logic ---
    int selectedGameIndex;
    static const int GAME_COUNT = 6;
    // Games are listed in alphabetical order
    const char* gameNames[GAME_COUNT] = {"Crossy Road", "Flappy Bird", "Frogger", "Genius", "Snake", "Tetris"};

    // --- Settings Logic ---
    int selectedSettingIndex;
    int settingsScrollOffset;
    // Row order in the Settings list (Language first)
    enum SettingItem {
        SETTING_LANGUAGE,
        SETTING_VOLUME,
        SETTING_BRIGHTNESS,
        SETTING_AUTO_SLEEP,
        SETTING_HIGHSCORES,
        SETTING_CREDITS,
        SETTING_ITEM_COUNT
    };
    static const int SETTING_COUNT = SETTING_ITEM_COUNT;

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
    void cycleLanguage(int direction);
    void handleConfirmResetInput();
    void handleCreditsInput();
};

#endif // MENUMANAGER_H