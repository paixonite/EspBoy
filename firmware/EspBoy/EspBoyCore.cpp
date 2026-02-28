#include "EspBoyCore.h"
#include "SnakeGame.h"
#include "FlappyBirdGame.h"

EspBoyCore::EspBoyCore() 
    : tft(), 
      soundManager(BUZZER_PIN), 
      menuManager(&tft, &soundManager, &batteryManager) {
    currentGame = nullptr;
    currentMode = MODE_MENU;
}

void EspBoyCore::begin() {
    loadHardware();
    batteryManager.begin();
    menuManager.begin();
}

void EspBoyCore::update() {
    soundManager.loop();

    if (currentMode == MODE_MENU) {
        int command = menuManager.update();
        
        // Se o menu retornar um ID válido, inicia o jogo
        if (command >= 0) {
            loadGame(command);
        }
    } 
    else if (currentMode == MODE_GAME) {
        if (currentGame != nullptr) {
            currentGame->loop();
            
            // Verifica se o jogo solicitou saída (Start + Select)
            if (currentGame->shouldExit()) {
                delete currentGame;
                currentGame = nullptr;
                currentMode = MODE_MENU;
                
                // Retorna diretamente para o carrossel, pulando a splash
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
    analogWrite(TFT_BL_PIN, 255); // Liga a tela com 100% de brilho (0-255)
}

void EspBoyCore::loadGame(int gameIndex) {
    if (currentGame != nullptr) {
        delete currentGame;
    }

    switch (gameIndex) {
        case 0:
            currentGame = new SnakeGame(&tft, &soundManager);
            break;
        case 1:
            currentGame = new FlappyBirdGame(&tft, &soundManager);
            break;
        default:
            return;
    }

    currentGame->setup();
    currentMode = MODE_GAME;
}