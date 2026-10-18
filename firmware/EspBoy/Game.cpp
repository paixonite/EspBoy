#include "Game.h"
#include "audio_assets.h"

// Initialize global memory for last used name
char Game::lastInitials[3] = {'A', 'A', 'A'};

// Pause popup layout (centered on the 320x170 landscape canvas)
static const char* const PAUSE_OPTIONS[] = {"Resume", "Quit"};
static const int PAUSE_OPTION_COUNT   = sizeof(PAUSE_OPTIONS) / sizeof(PAUSE_OPTIONS[0]);
static const int PAUSE_OPTION_RESUME  = 0;
static const int PAUSE_OPTION_QUIT    = 1;

static const int PAUSE_BOX_W          = 180;
static const int PAUSE_BOX_H          = 104;
static const int PAUSE_TITLE_OFFSET_Y = 20; // Title center, relative to the box top
static const int PAUSE_FIRST_OPT_Y    = 57; // First option center, relative to the box top
static const int PAUSE_OPT_SPACING    = 26;
static const int PAUSE_OPT_W          = 110;
static const int PAUSE_OPT_H          = 22;

Game::Game(TFT_eSPI* tft_display, SoundManager* sound_manager, const String& game_name)
    : tft(tft_display), sound(sound_manager), gameName(game_name) {
    _exit_request = false;
    score = 0;
    currentState = STATE_TITLE;

    pauseStartTime = 0;
    pausedTotalMs = 0;
    pauseSelection = PAUSE_OPTION_RESUME;
    resumePending = false;
    
    // Configure universal system placeholders.
    // Any child game can override this in its own constructor.
    defaultScores[0] = 30; defaultNames[0] = "ESP";
    defaultScores[1] = 20; defaultNames[1] = "BOY";
    defaultScores[2] = 10; defaultNames[2] = "CPU";
}

void Game::setup() {
    // Load highscores (or placeholders if flash is empty) BEFORE starting the game
    loadHighscores();
    resetGame();
    changeState(STATE_TITLE);
}

void Game::loop() {
    checkPauseButton();

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

        case STATE_PAUSED:
            handlePausedInput();
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
    else if (newState == STATE_PAUSED) drawPauseScreen();
    else if (newState == STATE_GAME_OVER) drawGameOverScreen();
    else if (newState == STATE_HIGHSCORE_VIEW) drawHighscoreViewScreen();
    else if (newState == STATE_HIGHSCORE_ENTRY) drawHighscoreEntryScreen();
}

unsigned long Game::gameMillis() const {
    // While paused, the clock is frozen at the instant the pause began
    unsigned long now = (currentState == STATE_PAUSED) ? pauseStartTime : millis();
    return now - pausedTotalMs;
}

void Game::checkPauseButton() {
    static bool lastStartState = LOW;
    bool currentStartState = digitalRead(BTN_START);
    
    if (lastStartState == LOW && currentStartState == HIGH && currentState == STATE_PLAYING) {
        pauseStartTime = millis();
        pauseSelection = PAUSE_OPTION_RESUME;
        resumePending = false;
        sound->play(MELODY_PAUSE, MELODY_PAUSE_LENGTH);
        changeState(STATE_PAUSED);
    }
    lastStartState = currentStartState;
}

// =================================================================
// PAUSE POPUP
// =================================================================

void Game::drawPauseScreen() {
    int boxX = (tft->width() - PAUSE_BOX_W) / 2;
    int boxY = (tft->height() - PAUSE_BOX_H) / 2;

    tft->fillRect(boxX, boxY, PAUSE_BOX_W, PAUSE_BOX_H, TFT_BLACK);
    tft->drawRect(boxX, boxY, PAUSE_BOX_W, PAUSE_BOX_H, TFT_WHITE);

    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_YELLOW);
    tft->setTextSize(3);
    tft->drawString("PAUSED", tft->width() / 2, boxY + PAUSE_TITLE_OFFSET_Y);

    drawPauseOptions();
}

void Game::drawPauseOptions() {
    int boxY = (tft->height() - PAUSE_BOX_H) / 2;
    int optX = (tft->width() - PAUSE_OPT_W) / 2;

    tft->setTextDatum(MC_DATUM);
    tft->setTextSize(2);

    for (int i = 0; i < PAUSE_OPTION_COUNT; i++) {
        int centerY = boxY + PAUSE_FIRST_OPT_Y + (i * PAUSE_OPT_SPACING);
        int optY = centerY - (PAUSE_OPT_H / 2);
        bool selected = (i == pauseSelection);

        // Same highlight language as the Settings rows: double-thick yellow outline
        tft->fillRect(optX, optY, PAUSE_OPT_W, PAUSE_OPT_H, TFT_BLACK);
        if (selected) {
            tft->drawRoundRect(optX, optY, PAUSE_OPT_W, PAUSE_OPT_H, 4, TFT_YELLOW);
            tft->drawRoundRect(optX + 1, optY + 1, PAUSE_OPT_W - 2, PAUSE_OPT_H - 2, 3, TFT_YELLOW);
        }

        tft->setTextColor(selected ? TFT_YELLOW : TFT_WHITE);
        tft->drawString(PAUSE_OPTIONS[i], tft->width() / 2, centerY);
    }
}

void Game::handlePausedInput() {
    static bool lastUp = LOW, lastDown = LOW, lastA = LOW, lastStart = LOW;
    bool curUp    = digitalRead(BTN_UP);
    bool curDown  = digitalRead(BTN_DOWN);
    bool curA     = digitalRead(BTN_A);
    bool curStart = digitalRead(BTN_START);

    if (millis() - stateTimer < 250) {
        lastUp = curUp; lastDown = curDown; lastA = curA; lastStart = curStart;
        return;
    }

    // A confirmed resume waits for the button release, so the press can't leak
    // into the game's own edge detection (e.g. an unwanted Flappy Bird flap).
    if (resumePending) {
        if (curA == LOW && curStart == LOW) {
            resumeFromPause();
        }
        lastUp = curUp; lastDown = curDown; lastA = curA; lastStart = curStart;
        return;
    }

    if (lastUp == LOW && curUp == HIGH && pauseSelection > 0) {
        pauseSelection--;
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawPauseOptions();
    }
    else if (lastDown == LOW && curDown == HIGH && pauseSelection < PAUSE_OPTION_COUNT - 1) {
        pauseSelection++;
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawPauseOptions();
    }
    else if (lastStart == LOW && curStart == HIGH) {
        // [START] toggles the pause off, regardless of the highlighted option
        resumePending = true;
    }
    else if (lastA == LOW && curA == HIGH) {
        if (pauseSelection == PAUSE_OPTION_RESUME) {
            resumePending = true;
        } else if (pauseSelection == PAUSE_OPTION_QUIT) {
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            _exit_request = true;
        }
    }

    lastUp = curUp; lastDown = curDown; lastA = curA; lastStart = curStart;
}

void Game::resumeFromPause() {
    pausedTotalMs += millis() - pauseStartTime;
    resumePending = false;
    sound->play(MELODY_RESUME, MELODY_RESUME_LENGTH);

    // Refreshing stateTimer also re-arms each game's short input lockout
    changeState(STATE_PLAYING);

    // Repaint immediately, so even a game over on the very next tick
    // never leaves popup leftovers behind the GAME OVER box
    onResume();
    drawPlaying();
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