#include "Game.h"
#include "../core/audio_assets.h"

// Initialize global memory for last used name
char Game::lastInitials[3] = {'A', 'A', 'A'};

// Pause popup layout (centered on the 320x170 landscape canvas)
static const StrId PAUSE_OPTIONS[] = {STR_RESUME, STR_QUIT};
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

// Title menu vertical carousel layout
static const StrId TITLE_OPTIONS[] = {STR_OPT_PLAY, STR_OPT_TUTORIAL, STR_OPT_HIGHSCORES};
static const int TITLE_OPTION_COUNT      = sizeof(TITLE_OPTIONS) / sizeof(TITLE_OPTIONS[0]);
static const int TITLE_OPTION_PLAY       = 0;
static const int TITLE_OPTION_TUTORIAL   = 1;
static const int TITLE_OPTION_HIGHSCORES = 2;

static const int TITLE_OPT_W          = 160;
static const int TITLE_OPT_H          = 24;
static const int TITLE_FIRST_OPT_Y    = 55;
static const int TITLE_OPT_SPACING    = 30;

Game::Game(TFT_eSPI* tft_display, SoundManager* sound_manager, InputManager* input_manager, const String& game_name)
    : tft(tft_display), sound(sound_manager), input(input_manager), gameName(game_name) {
    _exit_request = false;
    score = 0;
    currentState = STATE_TITLE;

    titleSelection = TITLE_OPTION_PLAY;

    pauseStartTime = 0;
    pausedTotalMs = 0;
    pauseSelection = PAUSE_OPTION_RESUME;
    
    // Configure universal system placeholders.
    // Any child game can override this in its own constructor.
    defaultScores[0] = 30; defaultNames[0] = "CPU";
    defaultScores[1] = 20; defaultNames[1] = "CPU";
    defaultScores[2] = 10; defaultNames[2] = "CPU";
}

void Game::setup() {
    // Load highscores (or placeholders if flash is empty) BEFORE starting the game
    loadHighscores();
    resetGame();
    titleSelection = TITLE_OPTION_PLAY;
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

        case STATE_TUTORIAL:
            handleTutorialInput();
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
    input->consumeAll();

    if (newState == STATE_TITLE) {
        titleSelection = TITLE_OPTION_PLAY;
        drawTitleScreen();
    }
    else if (newState == STATE_PAUSED) drawPauseScreen();
    else if (newState == STATE_GAME_OVER) drawGameOverScreen();
    else if (newState == STATE_HIGHSCORE_VIEW) drawHighscoreViewScreen();
    else if (newState == STATE_HIGHSCORE_ENTRY) drawHighscoreEntryScreen();
    else if (newState == STATE_TUTORIAL) drawTutorialScreen();
}

unsigned long Game::gameMillis() const {
    // While paused, the clock is frozen at the instant the pause began
    unsigned long now = (currentState == STATE_PAUSED) ? pauseStartTime : millis();
    return now - pausedTotalMs;
}

void Game::checkPauseButton() {
    if (currentState == STATE_PLAYING && input->wasPressed(BTN_ID_START)) {
        pauseStartTime = millis();
        pauseSelection = PAUSE_OPTION_RESUME;
        sound->play(MELODY_PAUSE, MELODY_PAUSE_LENGTH);
        changeState(STATE_PAUSED);
    }
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
    TextRenderer::draw(tft, tr(STR_PAUSED), tft->width() / 2, boxY + PAUSE_TITLE_OFFSET_Y);

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
        TextRenderer::draw(tft, tr(PAUSE_OPTIONS[i]), tft->width() / 2, centerY);
    }
}

void Game::handlePausedInput() {
    if (input->isRepeated(BTN_ID_UP, 220, 130) && pauseSelection > 0) {
        pauseSelection--;
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawPauseOptions();
    }
    else if (input->isRepeated(BTN_ID_DOWN, 220, 130) && pauseSelection < PAUSE_OPTION_COUNT - 1) {
        pauseSelection++;
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawPauseOptions();
    }
    else if (input->wasPressed(BTN_ID_START)) {
        // [START] toggles the pause off, regardless of the highlighted option
        resumeFromPause();
    }
    else if (input->wasPressed(BTN_ID_A)) {
        if (pauseSelection == PAUSE_OPTION_RESUME) {
            resumeFromPause();
        } else if (pauseSelection == PAUSE_OPTION_QUIT) {
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            _exit_request = true;
        }
    }
}

void Game::resumeFromPause() {
    pausedTotalMs += millis() - pauseStartTime;
    sound->play(MELODY_RESUME, MELODY_RESUME_LENGTH);

    // Consumes held buttons via changeState() so the resume press never leaks into gameplay
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
    drawTitleOptions();
    drawBottomBar(tr(STR_HINT_TITLE), TFT_WHITE);
}

void Game::drawTitleOptions() {
    int optX = (tft->width() - TITLE_OPT_W) / 2;

    tft->setTextDatum(MC_DATUM);
    tft->setTextSize(2);

    for (int i = 0; i < TITLE_OPTION_COUNT; i++) {
        int centerY = TITLE_FIRST_OPT_Y + (i * TITLE_OPT_SPACING);
        int optY = centerY - (TITLE_OPT_H / 2);
        bool selected = (i == titleSelection);

        // Highlight language matching Settings rows and pause menu: double-thick yellow outline
        tft->fillRect(optX, optY, TITLE_OPT_W, TITLE_OPT_H, TFT_BLACK);
        if (selected) {
            tft->drawRoundRect(optX, optY, TITLE_OPT_W, TITLE_OPT_H, 4, TFT_YELLOW);
            tft->drawRoundRect(optX + 1, optY + 1, TITLE_OPT_W - 2, TITLE_OPT_H - 2, 3, TFT_YELLOW);
        }

        tft->setTextColor(selected ? TFT_YELLOW : TFT_WHITE);
        TextRenderer::draw(tft, tr(TITLE_OPTIONS[i]), tft->width() / 2, centerY);
    }
}

void Game::handleInputTitle() {
    if (input->isRepeated(BTN_ID_UP, 220, 130) && titleSelection > 0) {
        titleSelection--;
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawTitleOptions();
    }
    else if (input->isRepeated(BTN_ID_DOWN, 220, 130) && titleSelection < TITLE_OPTION_COUNT - 1) {
        titleSelection++;
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawTitleOptions();
    }
    else if (input->wasPressed(BTN_ID_A)) {
        if (titleSelection == TITLE_OPTION_PLAY) {
            changeState(STATE_PLAYING);
            tft->fillScreen(TFT_BLACK); 
        } else if (titleSelection == TITLE_OPTION_TUTORIAL) {
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            changeState(STATE_TUTORIAL);
        } else if (titleSelection == TITLE_OPTION_HIGHSCORES) {
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            changeState(STATE_HIGHSCORE_VIEW);
        }
    }
    else if (input->wasPressed(BTN_ID_B)) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        _exit_request = true;
    }
}

void Game::drawTutorialScreen() {
    tft->fillScreen(TFT_BLACK);
    drawTopBar(tr(STR_TUTORIAL_TITLE), TFT_YELLOW);
    drawTutorialContent();
    drawBottomBar(tr(STR_HINT_BACK), TFT_WHITE);
}

void Game::handleTutorialInput() {
    if (input->wasPressed(BTN_ID_B)) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        changeState(STATE_TITLE);
    }
}

void Game::drawGameOverScreen() {
    // The box grows with the translated title (text size 3); stays 175 px wide in English
    tft->setTextSize(3);
    const char* title = tr(STR_GAME_OVER);
    int boxW = max(175, TextRenderer::width(tft, title) + 20);
    int boxX = tft->width() / 2 - 90 - (boxW - 175) / 2;

    tft->fillRect(boxX, tft->height() / 2 - 40, boxW, 80, TFT_BLACK);
    tft->drawRect(boxX, tft->height() / 2 - 40, boxW, 80, TFT_WHITE);
    
    tft->setTextColor(TFT_RED);
    tft->setTextDatum(MC_DATUM);
    TextRenderer::draw(tft, title, tft->width() / 2, tft->height() / 2 - 10);
    
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    TextRenderer::draw(tft, I18n::fmt(STR_SCORE_FMT, score), tft->width() / 2, tft->height() / 2 + 20);
}

void Game::handleGameOverState() {
    if (millis() - stateTimer > 1667) { // 2/3 of previous 2500ms duration
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
    drawTopBar(tr(STR_HIGHSCORES_TITLE), TFT_YELLOW);

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

    drawBottomBar(tr(STR_HINT_BACK), TFT_WHITE);
}

void Game::handleHighscoreViewInput() {
    if (input->wasPressed(BTN_ID_B)) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        changeState(STATE_TITLE);
    }
}

// =================================================================
// ENTRY SCREEN (INPUT INITIALS)
// =================================================================

void Game::drawHighscoreEntryScreen() {
    tft->fillScreen(TFT_BLACK);
    drawTopBar(tr(STR_NEW_TOP_SCORE), TFT_GREEN);
    
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    TextRenderer::draw(tft, I18n::fmt(STR_SCORE_FMT, score), tft->width() / 2, 52);

    drawBottomBar(tr(STR_HINT_CONFIRM), TFT_WHITE);

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
    bool redraw = false;

    if (input->isRepeated(BTN_ID_UP, 200, 120)) {
        currentInitials[currentInitialIndex]++;
        if (currentInitials[currentInitialIndex] > 'Z') currentInitials[currentInitialIndex] = 'A';
        redraw = true;
    } 
    else if (input->isRepeated(BTN_ID_DOWN, 200, 120)) {
        currentInitials[currentInitialIndex]--;
        if (currentInitials[currentInitialIndex] < 'A') currentInitials[currentInitialIndex] = 'Z';
        redraw = true;
    } 
    else if (input->isRepeated(BTN_ID_LEFT, 200, 120)) {
        currentInitialIndex--;
        if (currentInitialIndex < 0) currentInitialIndex = 2;
        redraw = true;
    } 
    else if (input->isRepeated(BTN_ID_RIGHT, 200, 120)) {
        currentInitialIndex++;
        if (currentInitialIndex > 2) currentInitialIndex = 0;
        redraw = true;
    }

    if (input->wasPressed(BTN_ID_A)) {
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
        return;
    }

    if (redraw) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawInitials();
    }
}


// =================================================================
// UI HELPERS (STANDARDIZED BARS)
// =================================================================

void Game::drawTopBar(const String& title, uint16_t color) {
    tft->drawFastHLine(0, BAR_HEIGHT - 1, tft->width(), TFT_WHITE);
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(color);
    tft->setTextSize(2);
    TextRenderer::draw(tft, title, tft->width() / 2, BAR_HEIGHT / 2);
}

void Game::drawBottomBar(const String& text, uint16_t color) {
    tft->drawFastHLine(0, tft->height() - BAR_HEIGHT, tft->width(), TFT_WHITE);
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(color);
    tft->setTextSize(1);
    TextRenderer::draw(tft, text, tft->width() / 2, tft->height() - (BAR_HEIGHT / 2));
}

void Game::drawTutorialHeader(const char* title, int y) {
    tft->setTextDatum(TL_DATUM);
    tft->setTextColor(TFT_YELLOW);
    tft->setTextSize(1);
    TextRenderer::draw(tft, title, 16, y);
}

void Game::drawTutorialLine(const char* text, int y, uint16_t color) {
    tft->setTextDatum(TL_DATUM);
    tft->setTextColor(color);
    tft->setTextSize(1);
    TextRenderer::draw(tft, text, 20, y);
}