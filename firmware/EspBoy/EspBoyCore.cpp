#include "EspBoyCore.h"
#include "CrossyRoadGame.h"
#include "FlappyBirdGame.h"
#include "FroggerGame.h"
#include "SnakeGame.h"
#include "TetrisGame.h"

EspBoyCore::EspBoyCore() 
    : tft(), 
      soundManager(BUZZER_PIN), 
      menuManager(&tft, &soundManager, &batteryManager) {
    currentGame = nullptr;
    currentMode = MODE_MENU;
}

void EspBoyCore::begin() {
    loadHardware();
    soundManager.begin();
    batteryManager.begin();
    menuManager.begin();
}

void EspBoyCore::update() {
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
            
            // Check if the game requested exit (Start + Select)
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
    analogWrite(TFT_BL_PIN, 255); // Turn on the screen with 100% brightness (0-255)
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
            currentGame = new SnakeGame(&tft, &soundManager);
            break;
        case 4:
            currentGame = new TetrisGame(&tft, &soundManager);
            break;
        default:
            return;
    }

    currentGame->setup();
    currentMode = MODE_GAME;
}