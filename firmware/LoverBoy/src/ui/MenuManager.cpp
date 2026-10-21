#include "MenuManager.h"
#include "../core/audio_assets.h"

#define MY_ORANGE tft->color565(255, 55, 0)
#define MY_GREY tft->color565(30, 30, 40)
const int SPLASH_DURATION_MS = 3500;

// Auto-sleep value labels, indexed like ConfigManager's sleep presets
static const StrId SLEEP_LABEL_IDS[ConfigManager::SLEEP_OPTION_COUNT] = {
    STR_VAL_OFF, STR_SLEEP_30S, STR_SLEEP_1M, STR_SLEEP_3M, STR_SLEEP_5M
};

MenuManager::MenuManager(TFT_eSPI* tft_display, SoundManager* sound_manager, BatteryManager* battery_manager, ConfigManager* config_manager, InputManager* input_manager)
    : tft(tft_display), sound(sound_manager), battery(battery_manager), config(config_manager), input(input_manager) {
    currentState = MENU_STATE_SPLASH;
    selectedGameIndex = 0;
    
    selectedSettingIndex = 0;
    settingsScrollOffset = 0;

    lastBatteryUpdate = 0;
}

void MenuManager::begin() {
    currentState = MENU_STATE_SPLASH;
    drawSplashScreen();
    splashStartTime = millis();
    sound->play(MELODY_SPLASH, MELODY_SPLASH_LENGTH);
}

void MenuManager::returnToMenu() {
    currentState = MENU_STATE_SELECT;
    input->consumeAll();
    drawMenu();
}

int MenuManager::update() {
    switch (currentState) {
        case MENU_STATE_SPLASH:
            if (millis() - splashStartTime > SPLASH_DURATION_MS) {
                currentState = MENU_STATE_SELECT;
                input->consumeAll();
                drawMenu();
            }
            return -1;

        case MENU_STATE_SELECT:
            if (millis() - lastBatteryUpdate > 5000) {
                lastBatteryUpdate = millis();
                updateBatteryDisplay();
            }
            return handleSelectInput();

        case MENU_STATE_SETTINGS:
            if (millis() - lastBatteryUpdate > 5000) {
                lastBatteryUpdate = millis();
                updateBatteryDisplay();
            }
            handleSettingsInput();
            return -1;

        case MENU_STATE_CONFIRM_RESET:
            handleConfirmResetInput();
            return -1;

        case MENU_STATE_CREDITS:
            handleCreditsInput();
            return -1;
    }
    return -1;
}

// =================================================================
// 1. GAME CAROUSEL LOGIC
// =================================================================

int MenuManager::handleSelectInput() {
    // Go to Settings
    if (input->wasPressed(BTN_ID_SELECT)) {
        currentState = MENU_STATE_SETTINGS;
        input->consumeAll();
        selectedSettingIndex = 0; 
        settingsScrollOffset = 0;
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawSettings();
        return -1;
    }

    // Left/Right Navigation
    if (input->isRepeated(BTN_ID_LEFT, 250, 150)) {
        selectedGameIndex--;
        if (selectedGameIndex < 0) selectedGameIndex = GAME_COUNT - 1;
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawCarouselContent();
    }
    else if (input->isRepeated(BTN_ID_RIGHT, 250, 150)) {
        selectedGameIndex++;
        if (selectedGameIndex >= GAME_COUNT) selectedGameIndex = 0;
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawCarouselContent();
    }

    if (input->wasPressed(BTN_ID_A)) {
        return selectedGameIndex;
    }

    return -1;
}

// =================================================================
// 2. SETTINGS LOGIC
// =================================================================

void MenuManager::handleSettingsInput() {
    // Return to Main Menu (B)
    if (input->wasPressed(BTN_ID_B)) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        returnToMenu();
        return;
    }

    // Up/Down/Left/Right Navigation
    if (input->isRepeated(BTN_ID_UP, 220, 130)) {
        if (selectedSettingIndex > 0) {
            selectedSettingIndex--;
            if (selectedSettingIndex < settingsScrollOffset) {
                settingsScrollOffset = selectedSettingIndex;
            }
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            drawSettings();
        }
    }
    else if (input->isRepeated(BTN_ID_DOWN, 220, 130)) {
        if (selectedSettingIndex < SETTING_COUNT - 1) {
            selectedSettingIndex++;
            if (selectedSettingIndex >= settingsScrollOffset + 3) {
                settingsScrollOffset = selectedSettingIndex - 2;
            }
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            drawSettings();
        }
    }
    else if (input->isRepeated(BTN_ID_LEFT, 220, 130)) {
        if (selectedSettingIndex == SETTING_LANGUAGE) {
            cycleLanguage(-1);
        } else if (selectedSettingIndex == SETTING_VOLUME) {
            int newVol = config->getVolume() - 10;
            if (newVol < 0) newVol = 0;
            sound->setVolume(newVol);
            if (!sound->isMuted()) {
                sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            }
            drawSettings();
        } else if (selectedSettingIndex == SETTING_BRIGHTNESS) {
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            config->setBrightness(config->getBrightness() - 10);
            drawSettings();
        } else if (selectedSettingIndex == SETTING_AUTO_SLEEP) {
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            config->setSleepTimeoutIndex(config->getSleepTimeoutIndex() - 1);
            drawSettings();
        }
    }
    else if (input->isRepeated(BTN_ID_RIGHT, 220, 130)) {
        if (selectedSettingIndex == SETTING_LANGUAGE) {
            cycleLanguage(1);
        } else if (selectedSettingIndex == SETTING_VOLUME) {
            int newVol = config->getVolume() + 10;
            if (newVol > 100) newVol = 100;
            sound->setVolume(newVol);
            if (!sound->isMuted()) {
                sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            }
            drawSettings();
        } else if (selectedSettingIndex == SETTING_BRIGHTNESS) {
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            config->setBrightness(config->getBrightness() + 10);
            drawSettings();
        } else if (selectedSettingIndex == SETTING_AUTO_SLEEP) {
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            config->setSleepTimeoutIndex(config->getSleepTimeoutIndex() + 1);
            drawSettings();
        }
    }

    // Interaction with button A
    if (input->wasPressed(BTN_ID_A)) {
        if (selectedSettingIndex == SETTING_LANGUAGE) {
            cycleLanguage(1);
        } else if (selectedSettingIndex == SETTING_VOLUME) { // Quick Mute/Unmute toggle
            config->toggleMute();
            if (config->isMuted()) {
                sound->stop();
            } else {
                sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            }
            drawSettings();
        } else if (selectedSettingIndex == SETTING_AUTO_SLEEP) { // Alternate to OFF / last active preset
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            config->toggleSleepTimeout();
            drawSettings();
        } else if (selectedSettingIndex == SETTING_HIGHSCORES) { // Reset Highscores
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            currentState = MENU_STATE_CONFIRM_RESET;
            input->consumeAll();
            drawConfirmReset();
        } else if (selectedSettingIndex == SETTING_CREDITS) {
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            currentState = MENU_STATE_CREDITS;
            input->consumeAll();
            drawSplashScreen();
        }
    }
}

void MenuManager::cycleLanguage(int direction) {
    int next = ((int)config->getLanguage() + direction + LANG_COUNT) % LANG_COUNT;
    config->setLanguage((Language)next);
    sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
    drawSettings(); // Full redraw: every label switches immediately
}

void MenuManager::handleConfirmResetInput() {
    if (input->wasPressed(BTN_ID_A)) {
        // Clear all game highscores in NVS via ConfigManager
        config->resetAllHighscores(gameNames, GAME_COUNT);

        // Play score sound to indicate action success
        sound->play(MELODY_SCORE, MELODY_SCORE_LENGTH); 
        
        currentState = MENU_STATE_SETTINGS;
        input->consumeAll();
        drawSettings(); 
    }
    else if (input->wasPressed(BTN_ID_B)) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        currentState = MENU_STATE_SETTINGS;
        input->consumeAll();
        drawSettings();
    }
}

void MenuManager::handleCreditsInput() {
    if (input->anyPressed()) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        currentState = MENU_STATE_SETTINGS;
        input->consumeAll();
        drawSettings();
    }
}

// =================================================================
// 3. DRAWING FUNCTIONS (UI)
// =================================================================

void MenuManager::drawSettings() {
    tft->fillScreen(TFT_BLACK);
    drawTopBar(tr(STR_SETTINGS_TITLE), TFT_YELLOW);

    int startY = 55;
    int spacingY = 30;

    // Up arrow indicating lines above (centered horizontally in left margin at x = 13)
    if (settingsScrollOffset > 0) {
        tft->fillTriangle(13, 43, 9, 51, 17, 51, TFT_WHITE);
    }

    // Down arrow indicating lines below (centered horizontally in left margin at x = 13)
    if (settingsScrollOffset + 3 < SETTING_COUNT) {
        tft->fillTriangle(13, 127, 9, 119, 17, 119, TFT_WHITE);
    }

    const int boxX = 26;
    const int boxW = tft->width() - 34;
    const int boxH = 26;

    for (int s = 0; s < 3; s++) {
        int idx = settingsScrollOffset + s;
        if (idx >= SETTING_COUNT) break;

        int y = startY + s * spacingY;
        bool isSelected = (selectedSettingIndex == idx);

        String label;
        String value;

        switch (idx) {
            case SETTING_LANGUAGE:
                label = tr(STR_SET_LANGUAGE);
                value = String("< ") + I18n::languageName(config->getLanguage()) + " >"; // i18n-ignore (arrows)
                break;
            case SETTING_VOLUME:
                label = tr(STR_SET_VOLUME);
                value = config->isMuted() ? String(tr(STR_VAL_OFF)) : ("< " + String(config->getVolume()) + "% >");
                break;
            case SETTING_BRIGHTNESS:
                label = tr(STR_SET_BRIGHTNESS);
                value = "< " + String(config->getBrightness()) + "% >";
                break;
            case SETTING_AUTO_SLEEP:
                label = tr(STR_SET_AUTO_SLEEP);
                value = tr(SLEEP_LABEL_IDS[config->getSleepTimeoutIndex()]);
                break;
            case SETTING_HIGHSCORES:
                label = tr(STR_SET_HIGHSCORES);
                value = tr(STR_VAL_RESET);
                break;
            case SETTING_CREDITS:
                label = tr(STR_SET_CREDITS);
                value = tr(STR_VAL_VIEW);
                break;
        }

        // Clear selection box around selected configuration (pure black background)
        if (isSelected) {
            tft->fillRoundRect(boxX, y - (boxH / 2), boxW, boxH, 4, TFT_BLACK);
            tft->drawRoundRect(boxX, y - (boxH / 2), boxW, boxH, 4, TFT_YELLOW);
            tft->drawRoundRect(boxX + 1, y - (boxH / 2) + 1, boxW - 2, boxH - 2, 3, TFT_YELLOW);
        }

        tft->setTextSize(2);
        tft->setTextDatum(ML_DATUM);
        tft->setTextColor(isSelected ? TFT_YELLOW : TFT_WHITE);
        TextRenderer::draw(tft, label, boxX + 8, y);

        tft->setTextDatum(MR_DATUM);
        tft->setTextColor(isSelected ? TFT_YELLOW : TFT_WHITE);
        TextRenderer::draw(tft, value, boxX + boxW - 8, y);
    }

    drawBottomBar(tr(STR_HINT_BACK), TFT_WHITE);
}

void MenuManager::drawConfirmReset() {
    tft->fillRect(tft->width() / 2 - 110, tft->height() / 2 - 45, 220, 90, TFT_BLACK);
    tft->drawRect(tft->width() / 2 - 110, tft->height() / 2 - 45, 220, 90, TFT_RED);
    
    tft->setTextColor(TFT_RED);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    TextRenderer::draw(tft, tr(STR_RESET_TITLE), tft->width() / 2, tft->height() / 2 - 15);
    
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(1);
    TextRenderer::draw(tft, tr(STR_HINT_YES_NO), tft->width() / 2, tft->height() / 2 + 15);
}

void MenuManager::drawMenu() {
    tft->fillScreen(TFT_BLACK);
    drawTopBar(tr(STR_CHOOSE_GAME_TITLE), TFT_YELLOW);
    drawCarouselContent();
    drawBottomBar(tr(STR_HINT_MENU), TFT_WHITE);
}

void MenuManager::drawCarouselContent() {
    // Clear only the carousel region between top and bottom bars (no full-screen flicker)
    tft->fillRect(0, BAR_HEIGHT, tft->width(), tft->height() - (2 * BAR_HEIGHT), TFT_BLACK);

    int gameY = (tft->height() / 2) - 2;

    // Fixed navigation arrows on display sides
    tft->setTextSize(3);
    tft->setTextColor(TFT_WHITE);

    tft->setTextDatum(ML_DATUM);
    tft->drawString("<", 16, gameY);

    tft->setTextDatum(MR_DATUM);
    tft->drawString(">", tft->width() - 16, gameY);

    // Centered game name (size 3)
    tft->setTextDatum(MC_DATUM);
    tft->drawString(gameNames[selectedGameIndex], tft->width() / 2, gameY);

    drawCarouselIndicators();
}

void MenuManager::drawCarouselIndicators() {
    const int dotRadius = 3;
    const int dotDiameter = 6;
    const int dashWidth = 18;
    const int gap = 8;
    const int indicatorY = 118;

    int totalWidth = dashWidth + (GAME_COUNT - 1) * dotDiameter + (GAME_COUNT - 1) * gap;
    int currentX = (tft->width() - totalWidth) / 2;
    uint16_t inactiveColor = tft->color565(110, 110, 120);

    for (int i = 0; i < GAME_COUNT; i++) {
        if (i == selectedGameIndex) {
            tft->fillRoundRect(currentX, indicatorY - dotRadius, dashWidth, dotDiameter, dotRadius, TFT_WHITE);
            currentX += dashWidth + gap;
        } else {
            tft->fillCircle(currentX + dotRadius, indicatorY, dotRadius, inactiveColor);
            currentX += dotDiameter + gap;
        }
    }
}

void MenuManager::drawSplashScreen() {
    tft->fillScreen(TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->setTextSize(5);

    String title = "LoverBoy"; // i18n-ignore (brand name)
    // Logo palette:
    // L: R3 Pure Red (255, 0, 0)
    // o: MY_ORANGE (reverted to original)
    // v: TFT_YELLOW (reverted to original)
    // e: G3 Pure Green (0, 255, 0)
    // r: B2 Pure Blue (0, 100, 255)
    // B: Pure Blue (0, 0, 255)
    // o: U1 Indigo (100, 20, 190)
    // y: P2 Deep Rose (230, 30, 100)
    uint16_t rainbowColors[] = {
        tft->color565(255,   0,   0), // L: R3
        MY_ORANGE,                    // o: original MY_ORANGE
        TFT_YELLOW,                   // v: original TFT_YELLOW
        tft->color565(  0, 255,   0), // e: G3
        tft->color565(  0, 100, 255), // r: B2
        tft->color565(  0,   0, 255), // B: Pure Blue (0, 0, 255)
        tft->color565(100,  20, 190), // o: U1
        tft->color565(230,  30, 100)  // y: P2
    };
    
    int totalWidth = tft->textWidth(title);
    int cursorX = (tft->width() / 2) - (totalWidth / 2);
    int cursorY = tft->height() / 2 - 10;
    
    for (int i = 0; i < title.length(); i++) {
        String letter = String(title.charAt(i));
        tft->setTextColor(rainbowColors[i % 8]);
        tft->setTextDatum(ML_DATUM);
        tft->drawString(letter, cursorX, cursorY);
        cursorX += tft->textWidth(letter);
    }

    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    TextRenderer::draw(tft, tr(STR_SPLASH_BY), tft->width() / 2, tft->height() / 2 + 30);
}

void MenuManager::updateBatteryDisplay() {
    lastBatteryUpdate = millis();
    tft->fillRect(tft->width() - 65, 0, 65, BAR_HEIGHT - 1, TFT_BLACK);
    
    int pct = battery->getPercentage();
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    // Battery dimensions (vertically centered in 25px top bar)
    const int batW = 22;
    const int batH = 12;
    const int termW = 2;
    const int termH = 6;
    const int batX = tft->width() - batW - termW - 5;
    const int batY = (BAR_HEIGHT / 2) - (batH / 2);

    // 1. Percentage text
    String pctStr = String(pct) + "%";
    tft->setTextDatum(MR_DATUM);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(1);
    tft->drawString(pctStr, batX - 5, BAR_HEIGHT / 2);

    // 2. Battery outline & terminal cap
    tft->drawRoundRect(batX, batY, batW, batH, 2, TFT_WHITE);
    tft->fillRect(batX + batW, batY + (batH - termH) / 2, termW, termH, TFT_WHITE);

    // 3. Battery interior fill
    const int innerW = batW - 4;
    const int innerH = batH - 4;
    int fillW = map(pct, 0, 100, 0, innerW);
    if (fillW > innerW) fillW = innerW;
    if (fillW < 0) fillW = 0;

    uint16_t fillColor = TFT_GREEN;
    if (pct <= 20) {
        fillColor = TFT_RED;
    } else if (pct <= 45) {
        fillColor = TFT_YELLOW;
    }

    if (fillW > 0) {
        tft->fillRect(batX + 2, batY + 2, fillW, innerH, fillColor);
    }
}

void MenuManager::drawTopBar(const String& title, uint16_t color) {
    tft->drawFastHLine(0, BAR_HEIGHT - 1, tft->width(), TFT_WHITE);
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(color);
    tft->setTextSize(2);
    TextRenderer::draw(tft, title, tft->width() / 2, BAR_HEIGHT / 2);
    updateBatteryDisplay();
}

void MenuManager::drawBottomBar(const String& text, uint16_t color) {
    tft->drawFastHLine(0, tft->height() - BAR_HEIGHT, tft->width(), TFT_WHITE);
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(color);
    tft->setTextSize(1);
    TextRenderer::draw(tft, text, tft->width() / 2, tft->height() - (BAR_HEIGHT / 2));
}