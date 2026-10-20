#include "EspBoyCore.h"
#include "../games/CrossyRoadGame.h"
#include "../games/FlappyBirdGame.h"
#include "../games/FroggerGame.h"
#include "../games/GeniusGame.h"
#include "../games/SnakeGame.h"
#include "../games/TetrisGame.h"

EspBoyCore::EspBoyCore() 
    : tft(), 
      configManager(),
      inputManager(),
      soundManager(BUZZER_PIN, &configManager), 
      menuManager(&tft, &soundManager, &batteryManager, &configManager, &inputManager) {
    currentGame = nullptr;
    currentMode = MODE_MENU;
    lastActivityTime = 0;
    isSleeping = false;
    isDimmed = false;
}

void EspBoyCore::begin() {
    loadHardware();
    inputManager.begin();
    configManager.begin();
    soundManager.begin();
    batteryManager.begin();
    menuManager.begin();
    lastActivityTime = millis();
}

void EspBoyCore::update() {
    inputManager.update();
    handlePowerManagement();
    if (isSleeping) {
        return;
    }

    soundManager.loop();

    if (currentMode == MODE_MENU) {
        int command = menuManager.update();
        
        // If the menu returns a valid ID, start the game
        if (command >= 0) {
            loadGame(command);
        }
    } 
    else if (currentMode == MODE_GAME) {
        if (currentGame != nullptr) {
            currentGame->loop();
            
            // Check if the game requested exit (Quit in the pause popup, or B on the title screen)
            if (currentGame->shouldExit()) {
                delete currentGame;
                currentGame = nullptr;
                currentMode = MODE_MENU;
                
                // Return directly to the carousel, skipping the splash screen
                menuManager.returnToMenu();
            }
        }
    }
}

void EspBoyCore::handlePowerManagement() {
    if (inputManager.anyPhysicalActivity()) {
        lastActivityTime = millis();

        if (isSleeping) {
            // Wake ST7789 display controller
            tft.writecommand(0x11); // ST7789_SLPOUT
            delay(120);
            configManager.applyBrightness();
            isSleeping = false;
            isDimmed = false;

            // Consume the wake button press so it does not trigger menu/game actions
            inputManager.consumeAll();
            lastActivityTime = millis();
            return;
        }

        if (isDimmed) {
            configManager.applyBrightness();
            isDimmed = false;
        }
        return;
    }

    if (isSleeping) {
        delay(40);
        return;
    }

    unsigned long timeoutMs = configManager.getSleepTimeoutMs();
    if (timeoutMs == 0) {
        if (isDimmed) {
            configManager.applyBrightness();
            isDimmed = false;
        }
        return;
    }

    unsigned long idleMs = millis() - lastActivityTime;
    const unsigned long DIM_WARNING_MS = 5000;

    if (idleMs >= timeoutMs) {
        soundManager.stop();
        configManager.setBacklightRaw(0);
        tft.writecommand(0x10); // ST7789_SLPIN
        isSleeping = true;
        isDimmed = false;
    } else if (timeoutMs > DIM_WARNING_MS && idleMs >= (timeoutMs - DIM_WARNING_MS)) {
        if (!isDimmed) {
            configManager.applyDimmedBrightness();
            isDimmed = true;
        }
    }
}

void EspBoyCore::loadHardware() {
    Serial.begin(115200);
    tft.init();
    tft.setRotation(1);
    
    pinMode(POWER_ENABLE_PIN, OUTPUT);
    digitalWrite(POWER_ENABLE_PIN, HIGH);

    pinMode(TFT_BL_PIN, OUTPUT);
}

void EspBoyCore::loadGame(int gameIndex) {
    if (currentGame != nullptr) {
        delete currentGame;
    }

    switch (gameIndex) {
        case 0:
            currentGame = new CrossyRoadGame(&tft, &soundManager, &inputManager);
            break;
        case 1:
            currentGame = new FlappyBirdGame(&tft, &soundManager, &inputManager);
            break;
        case 2:
            currentGame = new FroggerGame(&tft, &soundManager, &inputManager);
            break;
        case 3:
            currentGame = new GeniusGame(&tft, &soundManager, &inputManager);
            break;
        case 4:
            currentGame = new SnakeGame(&tft, &soundManager, &inputManager);
            break;
        case 5:
            currentGame = new TetrisGame(&tft, &soundManager, &inputManager);
            break;
        default:
            return;
    }

    inputManager.consumeAll();
    currentGame->setup();
    currentMode = MODE_GAME;
}