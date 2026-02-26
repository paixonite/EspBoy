#include "Game.h"

Game::Game(TFT_eSPI* tft_display, SoundManager* sound_manager, const String& game_name)
    : tft(tft_display), sound(sound_manager), gameName(game_name) {
    _exit_request = false;
    score = 0;
    currentState = STATE_TITLE;
}

void Game::setup() {
    resetGame();
    changeState(STATE_TITLE);
}

void Game::loop() {
    // Checagem global de saída (Start + Select) aplicável a qualquer estado
    checkGlobalExit();
    if (_exit_request) return;

    // Máquina de estados centralizada
    switch (currentState) {
        case STATE_TITLE:
            handleInputTitle();
            break;

        case STATE_PLAYING:
            handleInputPlaying();
            updatePlaying();
            
            // Se o update causou um Game Over, não desenha o frame atual
            if (currentState == STATE_GAME_OVER) break; 
            
            drawPlaying();
            delay(16); // Pequeno delay global para ~60 FPS
            break;

        case STATE_GAME_OVER:
            handleGameOverState();
            break;
    }
}

bool Game::shouldExit() const {
    return _exit_request;
}

void Game::changeState(GameState newState) {
    currentState = newState;
    stateTimer = millis();

    // Gatilhos de desenho de tela ao entrar no estado
    if (newState == STATE_TITLE) {
        drawTitleScreen();
    } else if (newState == STATE_GAME_OVER) {
        drawGameOverScreen();
    }
}

void Game::checkGlobalExit() {
    if (digitalRead(BTN_START) == HIGH && digitalRead(BTN_SELECT) == HIGH) {
        _exit_request = true;
    }
}

// --- Comportamentos Padrão ---

void Game::drawTitleScreen() {
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_YELLOW);
    tft->setTextSize(3);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(gameName, tft->width() / 2, tft->height() / 2 - 20);
    
    tft->setTextSize(2);
    tft->setTextColor(TFT_WHITE);
    tft->drawString("Press 'A' or 'Start'", tft->width() / 2, tft->height() / 2 + 20);
}

void Game::drawGameOverScreen() {
    // Tela padronizada (inspirada no seu Flappy Bird)
    tft->fillRect(tft->width() / 2 - 90, tft->height() / 2 - 40, 175, 80, TFT_BLACK);
    tft->drawRect(tft->width() / 2 - 90, tft->height() / 2 - 40, 175, 80, TFT_WHITE);
    
    tft->setTextColor(TFT_RED);
    tft->setTextSize(3);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("GAME OVER", tft->width() / 2, tft->height() / 2 - 10);
    
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    String s = "Score: " + String(score);
    tft->drawString(s, tft->width() / 2, tft->height() / 2 + 20);
}

void Game::handleInputTitle() {
    static bool lastButtonA = LOW;
    bool currentButtonA = digitalRead(BTN_A);
    bool currentStart = digitalRead(BTN_START);

    // Começa se apertar A ou Start
    if ((lastButtonA == LOW && currentButtonA == HIGH) || currentStart == HIGH) {
        changeState(STATE_PLAYING);
        tft->fillScreen(TFT_BLACK); // Limpa a tela para o jogo começar
    }
    lastButtonA = currentButtonA;
}

void Game::handleGameOverState() {
    // Espera 2.5 segundos e reinicia
    if (millis() - stateTimer > 2500) {
        setup(); 
    }
}