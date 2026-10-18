#include "Game.h"
#include "audio_assets.h"

// Initialize global memory for last used name
char Game::lastInitials[3] = {'A', 'A', 'A'};

Game::Game(TFT_eSPI* tft_display, SoundManager* sound_manager, const String& game_name)
    : tft(tft_display), sound(sound_manager), gameName(game_name) {
    _exit_request = false;
    score = 0;
    currentState = STATE_TITLE;
    
    // Configure universal system placeholders.
    // Any child game can override this in its own constructor.
    defaultScores[0] = 30; defaultNames[0] = "CPU";
    defaultScores[1] = 20; defaultNames[1] = "ESP";
    defaultScores[2] = 10; defaultNames[2] = "BOY";
}

void Game::setup() {
    // Load highscores (or placeholders if flash is empty) BEFORE starting the game
    loadHighscores();
    resetGame();
    changeState(STATE_TITLE);
}

void Game::loop() {
    checkGlobalExit();
    if (_exit_request) return;

    switch (currentState) {
        case STATE_TITLE:
            handleInputTitle();
            break;
            
        case STATE_HIGHSCORE_VIEW:
            handleHighscoreViewInput();
            break;

        case STATE_PLAYING:
            handleInputPlaying();
            updatePlaying();
            if (currentState == STATE_GAME_OVER) break; 
            
            drawPlaying();
            delay(16); 
            break;

        case STATE_GAME_OVER:
            handleGameOverState();
            break;
            
        case STATE_HIGHSCORE_ENTRY:
            handleHighscoreEntryInput();
            break;
    }
}

bool Game::shouldExit() const {
    return _exit_request;
}

void Game::changeState(GameState newState) {
    currentState = newState;
    stateTimer = millis();

    if (newState == STATE_TITLE) drawTitleScreen();
    else if (newState == STATE_GAME_OVER) drawGameOverScreen();
    else if (newState == STATE_HIGHSCORE_VIEW) drawHighscoreViewScreen();
    else if (newState == STATE_HIGHSCORE_ENTRY) drawHighscoreEntryScreen();
}

void Game::checkGlobalExit() {
    static bool lastSelectState = LOW;
    bool currentSelectState = digitalRead(BTN_SELECT);
    
    if (lastSelectState == LOW && currentSelectState == HIGH) {
        if (currentState == STATE_PLAYING || currentState == STATE_GAME_OVER) {
            _exit_request = true;
        }
    }
    lastSelectState = currentSelectState;
}

// =================================================================
// TITLE SCREEN AND GAME OVER
// =================================================================

void Game::drawTitleScreen() {
    tft->fillScreen(TFT_BLACK);
    drawTopBar(gameName, TFT_YELLOW);
    
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    tft->drawString("[A] Play", tft->width() / 2, tft->height() / 2);

    drawBottomBar("[SELECT] Highscores    [B] Back", TFT_WHITE);
}

void Game::handleInputTitle() {
    static bool lastA = LOW, lastSelect = LOW, lastB = LOW;
    bool curA = digitalRead(BTN_A);
    bool curSelect = digitalRead(BTN_SELECT);
    bool curB = digitalRead(BTN_B);

    if (millis() - stateTimer < 250) {
        lastA = curA; lastSelect = curSelect; lastB = curB;
        return;
    }

    if (lastA == LOW && curA == HIGH) {
        changeState(STATE_PLAYING);
        tft->fillScreen(TFT_BLACK); 
    } 
    else if (lastSelect == LOW && curSelect == HIGH) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        changeState(STATE_HIGHSCORE_VIEW);
    }
    else if (lastB == LOW && curB == HIGH) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        _exit_request = true;
    }
    
    lastA = curA; lastSelect = curSelect; lastB = curB;
}

void Game::drawGameOverScreen() {
    tft->fillRect(tft->width() / 2 - 90, tft->height() / 2 - 40, 175, 80, TFT_BLACK);
    tft->drawRect(tft->width() / 2 - 90, tft->height() / 2 - 40, 175, 80, TFT_WHITE);
    
    tft->setTextColor(TFT_RED);
    tft->setTextSize(3);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("GAME OVER", tft->width() / 2, tft->height() / 2 - 10);
    
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    tft->drawString("Score: " + String(score), tft->width() / 2, tft->height() / 2 + 20);
}

void Game::handleGameOverState() {
    if (millis() - stateTimer > 2500) {
        if (newHighscoreRank != -1) {
            currentInitials[0] = lastInitials[0];
            currentInitials[1] = lastInitials[1];
            currentInitials[2] = lastInitials[2];
            currentInitialIndex = 0;
            sound->play(MELODY_NEW_HIGHSCORE, MELODY_NEW_HIGHSCORE_LENGTH);

            changeState(STATE_HIGHSCORE_ENTRY);
        } else {
            setup(); 
        }
    }
}

// =================================================================
// HIGHSCORE LOGIC
// =================================================================

void Game::endGame() {
    newHighscoreRank = -1;
    for (int i = 0; i < 3; i++) {
        if (score > topScores[i]) {
            newHighscoreRank = i;
            break;
        }
    }
    changeState(STATE_GAME_OVER);
}

void Game::loadHighscores() {
    String ns = gameName;
    ns.replace(" ", ""); 
    
    preferences.begin(ns.c_str(), true); 
    for (int i = 0; i < 3; i++) {
        // Try loading from memory; if not found, use defined defaults
        topScores[i] = preferences.getInt(("score" + String(i)).c_str(), defaultScores[i]);
        topNames[i] = preferences.getString(("name" + String(i)).c_str(), defaultNames[i]);
    }
    preferences.end();
}

void Game::saveHighscores() {
    String ns = gameName;
    ns.replace(" ", ""); 
    
    preferences.begin(ns.c_str(), false); 
    for (int i = 0; i < 3; i++) {
        preferences.putInt(("score" + String(i)).c_str(), topScores[i]);
        preferences.putString(("name" + String(i)).c_str(), topNames[i]);
    }
    preferences.end();
}

// =================================================================
// VIEW SCREEN (TOP SCORES)
// =================================================================

void Game::drawHighscoreViewScreen() {
    tft->fillScreen(TFT_BLACK);
    drawTopBar("HIGHSCORES", TFT_YELLOW);

    tft->setTextSize(2);
    int startY = 55;
    int spacingY = 30;
    
    for (int i = 0; i < 3; i++) {
        int y = startY + (i * spacingY);
        tft->setTextColor(TFT_WHITE);
        tft->setTextDatum(ML_DATUM);
        tft->drawString(String(i + 1) + ". " + topNames[i], 40, y);
        
        tft->setTextDatum(MR_DATUM);
        tft->drawString(String(topScores[i]), tft->width() - 40, y);
    }

    drawBottomBar("[B] Back", TFT_WHITE);
}

void Game::handleHighscoreViewInput() {
    static bool lastB = LOW;
    bool curB = digitalRead(BTN_B);

    if (millis() - stateTimer < 250) {
        lastB = curB; return;
    }

    if (lastB == LOW && curB == HIGH) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        changeState(STATE_TITLE);
    }
    lastB = curB;
}

// =================================================================
// ENTRY SCREEN (INPUT INITIALS)
// =================================================================

void Game::drawHighscoreEntryScreen() {
    tft->fillScreen(TFT_BLACK);
    drawTopBar("NEW TOP SCORE!", TFT_GREEN);
    
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    tft->drawString("Score: " + String(score), tft->width() / 2, 52);

    drawBottomBar("[A] Confirm", TFT_WHITE);

    drawInitials();
}

void Game::drawInitials() {
    tft->setTextSize(4);
    int charSpacing = 35;
    int startX = (tft->width() / 2) - charSpacing;
    int yPos = 98;

    tft->setTextDatum(MC_DATUM);

    for (int i = 0; i < 3; i++) {
        int centerX = startX + (i * charSpacing);
        
        tft->fillRect(centerX - 15, yPos - 20, 30, 45, TFT_BLACK);

        tft->setTextColor(i == currentInitialIndex ? TFT_YELLOW : TFT_WHITE);
        tft->drawString(String(currentInitials[i]), centerX, yPos);
        
        if (i == currentInitialIndex) {
            tft->drawFastHLine(centerX - 14, yPos + 20, 24, TFT_YELLOW);
        }
    }
}

void Game::handleHighscoreEntryInput() {
    static unsigned long lastNavTime = 0;
    const int NAV_DELAY = 150; 
    
    bool curUp = digitalRead(BTN_UP);
    bool curDown = digitalRead(BTN_DOWN);
    bool curLeft = digitalRead(BTN_LEFT);
    bool curRight = digitalRead(BTN_RIGHT);
    
    static bool lastA = LOW;
    bool curA = digitalRead(BTN_A);

    if (millis() - stateTimer < 250) {
        lastA = curA; return;
    }

    bool redraw = false;

    if (millis() - lastNavTime > NAV_DELAY) {
        if (curUp == HIGH) {
            currentInitials[currentInitialIndex]++;
            if (currentInitials[currentInitialIndex] > 'Z') currentInitials[currentInitialIndex] = 'A';
            redraw = true;
            lastNavTime = millis();
        } 
        else if (curDown == HIGH) {
            currentInitials[currentInitialIndex]--;
            if (currentInitials[currentInitialIndex] < 'A') currentInitials[currentInitialIndex] = 'Z';
            redraw = true;
            lastNavTime = millis();
        } 
        else if (curLeft == HIGH) {
            currentInitialIndex--;
            if (currentInitialIndex < 0) currentInitialIndex = 2;
            redraw = true;
            lastNavTime = millis();
        } 
        else if (curRight == HIGH) {
            currentInitialIndex++;
            if (currentInitialIndex > 2) currentInitialIndex = 0;
            redraw = true;
            lastNavTime = millis();
        }
    }

    if (lastA == LOW && curA == HIGH) {
        for (int i = 0; i < 3; i++) lastInitials[i] = currentInitials[i];

        for (int i = 2; i > newHighscoreRank; i--) {
            topScores[i] = topScores[i - 1];
            topNames[i] = topNames[i - 1];
        }
        topScores[newHighscoreRank] = score;
        topNames[newHighscoreRank] = String(currentInitials[0]) + String(currentInitials[1]) + String(currentInitials[2]);
        
        saveHighscores();
        sound->play(MELODY_SCORE, MELODY_SCORE_LENGTH); 
        setup(); 
        
        lastA = curA;
        return;
    }

    if (redraw) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawInitials();
    }

    lastA = curA;
}


// =================================================================
// UI HELPERS (STANDARDIZED BARS)
// =================================================================

void Game::drawTopBar(const String& title, uint16_t color) {
    tft->drawFastHLine(0, BAR_HEIGHT - 1, tft->width(), TFT_WHITE);
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(color);
    tft->setTextSize(2);
    tft->drawString(title, tft->width() / 2, BAR_HEIGHT / 2);
}

void Game::drawBottomBar(const String& text, uint16_t color) {
    tft->drawFastHLine(0, tft->height() - BAR_HEIGHT, tft->width(), TFT_WHITE);
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(color);
    tft->setTextSize(1);
    tft->drawString(text, tft->width() / 2, tft->height() - (BAR_HEIGHT / 2));
}