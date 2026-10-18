#include "EspBoyCore.h"
#include "CrossyRoadGame.h"
#include "FlappyBirdGame.h"
#include "FroggerGame.h"
#include "GeniusGame.h"
#include "SnakeGame.h"
#include "TetrisGame.h"

EspBoyCore::EspBoyCore() 
    : tft(), 
      configManager(),
      soundManager(BUZZER_PIN, &configManager), 
      menuManager(&tft, &soundManager, &batteryManager, &configManager) {
    currentGame = nullptr;
    currentMode = MODE_MENU;
    lastActivityTime = 0;
    isSleeping = false;
    isDimmed = false;
}

void EspBoyCore::begin() {
    loadHardware();
    configManager.begin();
    soundManager.begin();
    batteryManager.begin();
    menuManager.begin();
    lastActivityTime = millis();
}

void EspBoyCore::update() {
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
            
            // Check if the game requested exit (SELECT during play / game over, or B on the title screen)
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

bool EspBoyCore::anyButtonPressed() const {
    return digitalRead(BTN_UP) == HIGH ||
           digitalRead(BTN_DOWN) == HIGH ||
           digitalRead(BTN_LEFT) == HIGH ||
           digitalRead(BTN_RIGHT) == HIGH ||
           digitalRead(BTN_START) == HIGH ||
           digitalRead(BTN_SELECT) == HIGH ||
           digitalRead(BTN_A) == HIGH ||
           digitalRead(BTN_B) == HIGH;
}

void EspBoyCore::handlePowerManagement() {
    if (anyButtonPressed()) {
        lastActivityTime = millis();

        if (isSleeping) {
            // Wake ST7789 display controller
            tft.writecommand(0x11); // ST7789_SLPOUT
            delay(120);
            configManager.applyBrightness();
            isSleeping = false;
            isDimmed = false;

            // Consume the wake button press so it does not trigger menu/game actions
            while (anyButtonPressed()) {
                delay(10);
            }
            delay(80);
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

    pinMode(BTN_UP, INPUT_PULLDOWN);
    pinMode(BTN_DOWN, INPUT_PULLDOWN);
    pinMode(BTN_LEFT, INPUT_PULLDOWN);
    pinMode(BTN_RIGHT, INPUT_PULLDOWN);
    pinMode(BTN_START, INPUT_PULLDOWN);
    pinMode(BTN_SELECT, INPUT_PULLDOWN);
    pinMode(BTN_A, INPUT_PULLDOWN);
    pinMode(BTN_B, INPUT_PULLDOWN);
    
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
            currentGame = new CrossyRoadGame(&tft, &soundManager);
            break;
        case 1:
            currentGame = new FlappyBirdGame(&tft, &soundManager);
            break;
        case 2:
            currentGame = new FroggerGame(&tft, &soundManager);
            break;
        case 3:
            currentGame = new GeniusGame(&tft, &soundManager);
            break;
        case 4:
            currentGame = new SnakeGame(&tft, &soundManager);
            break;
        case 5:
            currentGame = new TetrisGame(&tft, &soundManager);
            break;
        default:
            return;
    }

    currentGame->setup();
    currentMode = MODE_GAME;
}