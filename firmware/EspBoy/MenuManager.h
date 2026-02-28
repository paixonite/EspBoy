#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <TFT_eSPI.h>
#include "SoundManager.h"
#include "BatteryManager.h"
#include "pins.h"

// Adicionados os novos estados para o Menu de Configurações
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
    int update(); // Retorna -1 se continua no menu, ou o ID do jogo

private:
    TFT_eSPI* tft;
    SoundManager* sound;
    BatteryManager* battery;
    
    MenuState currentState;
    unsigned long splashStartTime;
    unsigned long lastBatteryUpdate;
    unsigned long lastNavInputTime;

    // --- Lógica do Carrossel de Jogos ---
    int selectedGameIndex;
    static const int GAME_COUNT = 2;
    const char* gameNames[GAME_COUNT] = {"Snake", "Flappy Bird"};

    // --- Lógica de Configurações ---
    int selectedSettingIndex;
    static const int SETTING_COUNT = 3;
    int currentBrightness; // Gancho para o futuro PWM (10 a 100)

    // --- Estados dos Botões ---
    bool lastLeftState, lastRightState, lastUpState, lastDownState;
    bool lastConfirmState, lastCancelState, lastSelectState;

    // --- Funções de Desenho ---
    void drawSplashScreen();
    void drawMenu();
    void updateBatteryDisplay();
    void drawSettings();
    void drawConfirmReset();

    // --- Handlers de Input ---
    int handleSelectInput();
    void handleSettingsInput();
    void handleConfirmResetInput();
};

#endif // MENUMANAGER_H