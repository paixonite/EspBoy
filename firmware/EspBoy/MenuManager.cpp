#include "MenuManager.h"
#include "audio_assets.h"

#define MY_ORANGE tft->color565(255, 55, 0)
#define MY_GREY tft->color565(30, 30, 40)
const int SPLASH_DURATION_MS = 3500;
const int NAV_DEBOUNCE_DELAY_MS = 250;
const int LOCKOUT_TIMER_MS = 250;

// Global lockout timer to prevent input bleed during menu transitions
static unsigned long menuStateTimer = 0;

MenuManager::MenuManager(TFT_eSPI* tft_display, SoundManager* sound_manager, BatteryManager* battery_manager, ConfigManager* config_manager)
    : tft(tft_display), sound(sound_manager), battery(battery_manager), config(config_manager) {
    currentState = MENU_STATE_SPLASH;
    selectedGameIndex = 0;
    
    selectedSettingIndex = 0;
    settingsScrollOffset = 0;

    lastBatteryUpdate = 0;
    lastNavInputTime = 0;
    
    lastLeftState = LOW; lastRightState = LOW; 
    lastUpState = LOW; lastDownState = LOW;
    lastConfirmState = LOW; lastCancelState = LOW; lastSelectState = LOW;
}

void MenuManager::begin() {
    currentState = MENU_STATE_SPLASH;
    drawSplashScreen();
    splashStartTime = millis();
    sound->play(MELODY_SPLASH, MELODY_SPLASH_LENGTH);
}

void MenuManager::returnToMenu() {
    currentState = MENU_STATE_SELECT;
    menuStateTimer = millis(); // Start transition cooldown
    drawMenu();
    
    // Clear inputs by forcing current read to prevent bounce
    lastLeftState = digitalRead(BTN_LEFT);
    lastRightState = digitalRead(BTN_RIGHT);
    lastUpState = digitalRead(BTN_UP);
    lastDownState = digitalRead(BTN_DOWN);
    lastConfirmState = digitalRead(BTN_A);
    lastCancelState = digitalRead(BTN_B);
    lastSelectState = digitalRead(BTN_SELECT);
}

int MenuManager::update() {
    switch (currentState) {
        case MENU_STATE_SPLASH:
            if (millis() - splashStartTime > SPLASH_DURATION_MS) {
                currentState = MENU_STATE_SELECT;
                menuStateTimer = millis();
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
    bool currentLeftState = digitalRead(BTN_LEFT);
    bool currentRightState = digitalRead(BTN_RIGHT);
    bool currentConfirmState = digitalRead(BTN_A);
    bool currentSelectState = digitalRead(BTN_SELECT);

    // LOCKOUT TIMER: Ignore inputs right after state change
    if (millis() - menuStateTimer < LOCKOUT_TIMER_MS) {
        lastLeftState = currentLeftState; lastRightState = currentRightState;
        lastConfirmState = currentConfirmState; lastSelectState = currentSelectState;
        return -1;
    }

    // Go to Settings (Triggered on PRESS: LOW to HIGH)
    if (lastSelectState == LOW && currentSelectState == HIGH) {
        currentState = MENU_STATE_SETTINGS;
        menuStateTimer = millis(); 
        selectedSettingIndex = 0; 
        settingsScrollOffset = 0;
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawSettings();
        
        lastSelectState = currentSelectState;
        return -1;
    }

    // Left/Right Navigation
    if (millis() - lastNavInputTime > NAV_DEBOUNCE_DELAY_MS) {
        if (lastLeftState == LOW && currentLeftState == HIGH) {
            lastNavInputTime = millis();
            selectedGameIndex--;
            if (selectedGameIndex < 0) selectedGameIndex = GAME_COUNT - 1;
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            tft->fillScreen(MY_GREY);
            delay(40);
            drawMenu();
        }
        else if (lastRightState == LOW && currentRightState == HIGH) {
            lastNavInputTime = millis();
            selectedGameIndex++;
            if (selectedGameIndex >= GAME_COUNT) selectedGameIndex = 0;
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            tft->fillScreen(MY_GREY);
            delay(40);
            drawMenu();
        }
    }

    int selectedGame = -1;
    if (lastConfirmState == LOW && currentConfirmState == HIGH) {
        selectedGame = selectedGameIndex; 
    }

    lastLeftState = currentLeftState;
    lastRightState = currentRightState;
    lastConfirmState = currentConfirmState;
    lastSelectState = currentSelectState;

    return selectedGame;
}

// =================================================================
// 2. SETTINGS LOGIC
// =================================================================

void MenuManager::handleSettingsInput() {
    bool currentUpState = digitalRead(BTN_UP);
    bool currentDownState = digitalRead(BTN_DOWN);
    bool currentLeftState = digitalRead(BTN_LEFT);
    bool currentRightState = digitalRead(BTN_RIGHT);
    bool currentConfirmState = digitalRead(BTN_A);
    bool currentCancelState = digitalRead(BTN_B);

    // LOCKOUT TIMER
    if (millis() - menuStateTimer < LOCKOUT_TIMER_MS) {
        lastUpState = currentUpState; lastDownState = currentDownState;
        lastLeftState = currentLeftState; lastRightState = currentRightState;
        lastConfirmState = currentConfirmState; lastCancelState = currentCancelState;
        return;
    }

    // Return to Main Menu (B)
    if (lastCancelState == LOW && currentCancelState == HIGH) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        returnToMenu(); // returnToMenu updates timer
        lastCancelState = currentCancelState;
        return;
    }

    // Up/Down/Left/Right Navigation
    if (millis() - lastNavInputTime > NAV_DEBOUNCE_DELAY_MS) {
        if (lastUpState == LOW && currentUpState == HIGH) {
            lastNavInputTime = millis();
            if (selectedSettingIndex > 0) {
                selectedSettingIndex--;
                if (selectedSettingIndex < settingsScrollOffset) {
                    settingsScrollOffset = selectedSettingIndex;
                }
                sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
                drawSettings();
            }
        }
        else if (lastDownState == LOW && currentDownState == HIGH) {
            lastNavInputTime = millis();
            if (selectedSettingIndex < SETTING_COUNT - 1) {
                selectedSettingIndex++;
                if (selectedSettingIndex >= settingsScrollOffset + 3) {
                    settingsScrollOffset = selectedSettingIndex - 2;
                }
                sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
                drawSettings();
            }
        }
        else if (lastLeftState == LOW && currentLeftState == HIGH) {
            lastNavInputTime = millis();
            if (selectedSettingIndex == 0) { // Volume
                int newVol = config->getVolume() - 10;
                if (newVol < 0) newVol = 0;
                sound->setVolume(newVol);
                if (!sound->isMuted()) {
                    sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
                }
            } else if (selectedSettingIndex == 1) { // Brightness
                sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
                config->setBrightness(config->getBrightness() - 10);
            } else if (selectedSettingIndex == 2) { // Auto Sleep
                sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
                config->setSleepTimeoutIndex(config->getSleepTimeoutIndex() - 1);
            }
            drawSettings();
        }
        else if (lastRightState == LOW && currentRightState == HIGH) {
            lastNavInputTime = millis();
            if (selectedSettingIndex == 0) { // Volume
                int newVol = config->getVolume() + 10;
                if (newVol > 100) newVol = 100;
                sound->setVolume(newVol);
                if (!sound->isMuted()) {
                    sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
                }
            } else if (selectedSettingIndex == 1) { // Brightness
                sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
                config->setBrightness(config->getBrightness() + 10);
            } else if (selectedSettingIndex == 2) { // Auto Sleep
                sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
                config->setSleepTimeoutIndex(config->getSleepTimeoutIndex() + 1);
            }
            drawSettings();
        }
    }

    // Interaction with button A
    if (lastConfirmState == LOW && currentConfirmState == HIGH) {
        if (selectedSettingIndex == 0) { // Quick Mute/Unmute toggle on Volume
            config->toggleMute();
            if (config->isMuted()) {
                sound->stop();
            } else {
                sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            }
            drawSettings();
        } else if (selectedSettingIndex == 2) { // Cycle Auto Sleep preset
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            config->setSleepTimeoutIndex(config->getSleepTimeoutIndex() + 1);
            drawSettings();
        } else if (selectedSettingIndex == 3) { // Reset Highscores
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            currentState = MENU_STATE_CONFIRM_RESET;
            menuStateTimer = millis(); 
            drawConfirmReset();
        } else if (selectedSettingIndex == 4) { // Credits
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            currentState = MENU_STATE_CREDITS;
            menuStateTimer = millis();
            drawSplashScreen();
        }
    }

    lastUpState = currentUpState; lastDownState = currentDownState;
    lastLeftState = currentLeftState; lastRightState = currentRightState;
    lastConfirmState = currentConfirmState; lastCancelState = currentCancelState;
}

void MenuManager::handleConfirmResetInput() {
    bool currentConfirmState = digitalRead(BTN_A);
    bool currentCancelState = digitalRead(BTN_B);

    // LOCKOUT TIMER
    if (millis() - menuStateTimer < LOCKOUT_TIMER_MS) {
        lastConfirmState = currentConfirmState; lastCancelState = currentCancelState;
        return;
    }

    if (lastConfirmState == LOW && currentConfirmState == HIGH) {
        // Clear all game highscores in NVS via ConfigManager
        config->resetAllHighscores(gameNames, GAME_COUNT);

        // Play score sound to indicate action success
        sound->play(MELODY_SCORE, MELODY_SCORE_LENGTH); 
        
        currentState = MENU_STATE_SETTINGS;
        menuStateTimer = millis(); 
        drawSettings(); 
    }
    else if (lastCancelState == LOW && currentCancelState == HIGH) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        currentState = MENU_STATE_SETTINGS;
        menuStateTimer = millis(); 
        drawSettings();
    }

    lastConfirmState = currentConfirmState;
    lastCancelState = currentCancelState;
}

void MenuManager::handleCreditsInput() {
    // LOCKOUT TIMER: Prevent button bleed from settings menu entry
    if (millis() - menuStateTimer < LOCKOUT_TIMER_MS) {
        return;
    }

    // Check if any button is pressed
    bool anyPressed = digitalRead(BTN_UP) == HIGH ||
                      digitalRead(BTN_DOWN) == HIGH ||
                      digitalRead(BTN_LEFT) == HIGH ||
                      digitalRead(BTN_RIGHT) == HIGH ||
                      digitalRead(BTN_START) == HIGH ||
                      digitalRead(BTN_SELECT) == HIGH ||
                      digitalRead(BTN_A) == HIGH ||
                      digitalRead(BTN_B) == HIGH;

    if (anyPressed) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        currentState = MENU_STATE_SETTINGS;
        menuStateTimer = millis();

        // Sync button states to avoid triggering actions immediately upon return
        lastUpState = digitalRead(BTN_UP);
        lastDownState = digitalRead(BTN_DOWN);
        lastLeftState = digitalRead(BTN_LEFT);
        lastRightState = digitalRead(BTN_RIGHT);
        lastConfirmState = digitalRead(BTN_A);
        lastCancelState = digitalRead(BTN_B);
        lastSelectState = digitalRead(BTN_SELECT);

        drawSettings();
    }
}

// =================================================================
// 3. DRAWING FUNCTIONS (UI)
// =================================================================

void MenuManager::drawSettings() {
    tft->fillScreen(TFT_BLACK);
    drawTopBar("SETTINGS", TFT_YELLOW);

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
            case 0:
                label = "Volume:";
                value = config->isMuted() ? "[ OFF ]" : ("< " + String(config->getVolume()) + "% >");
                break;
            case 1:
                label = "Brightness:";
                value = "< " + String(config->getBrightness()) + "% >";
                break;
            case 2:
                label = "Auto Sleep:";
                value = config->getSleepTimeoutLabel();
                break;
            case 3:
                label = "Highscores:";
                value = "[ RESET ]";
                break;
            case 4:
                label = "Credits:";
                value = "[ VIEW ]";
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
        tft->drawString(label, boxX + 8, y);

        tft->setTextDatum(MR_DATUM);
        tft->setTextColor(isSelected ? TFT_YELLOW : TFT_WHITE);
        tft->drawString(value, boxX + boxW - 8, y);
    }

    drawBottomBar("[B] Back", TFT_WHITE);
}

void MenuManager::drawConfirmReset() {
    tft->fillRect(tft->width() / 2 - 110, tft->height() / 2 - 45, 220, 90, TFT_BLACK);
    tft->drawRect(tft->width() / 2 - 110, tft->height() / 2 - 45, 220, 90, TFT_RED);
    
    tft->setTextColor(TFT_RED);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("RESET HIGHSCORES?", tft->width() / 2, tft->height() / 2 - 15);
    
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(1);
    tft->drawString("[A] Yes    [B] No", tft->width() / 2, tft->height() / 2 + 15);
}

void MenuManager::drawMenu() {
    tft->fillScreen(TFT_BLACK);
    drawTopBar("CHOOSE A GAME", TFT_YELLOW);
    
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

    drawBottomBar("[SELECT] Settings    [A] Play", TFT_WHITE);
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

    String title = "EspBoy";
    uint16_t rainbowColors[] = {TFT_RED, MY_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_BLUE, TFT_PURPLE};
    
    int totalWidth = tft->textWidth(title);
    int cursorX = (tft->width() / 2) - (totalWidth / 2);
    int cursorY = tft->height() / 2 - 10;
    
    for (int i = 0; i < title.length(); i++) {
        String letter = String(title.charAt(i));
        tft->setTextColor(rainbowColors[i % 6]);
        tft->setTextDatum(ML_DATUM);
        tft->drawString(letter, cursorX, cursorY);
        cursorX += tft->textWidth(letter);
    }

    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_CYAN);
    tft->setTextSize(2);
    tft->drawString("by Paixonite", tft->width() / 2, tft->height() / 2 + 30);
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
    tft->drawString(title, tft->width() / 2, BAR_HEIGHT / 2);
    updateBatteryDisplay();
}

void MenuManager::drawBottomBar(const String& text, uint16_t color) {
    tft->drawFastHLine(0, tft->height() - BAR_HEIGHT, tft->width(), TFT_WHITE);
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(color);
    tft->setTextSize(1);
    tft->drawString(text, tft->width() / 2, tft->height() - (BAR_HEIGHT / 2));
}