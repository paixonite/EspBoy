#include "MenuManager.h"
#include "audio_assets.h"

#define MY_ORANGE tft->color565(255, 55, 0)
#define MY_GREY tft->color565(30, 30, 40)
const int SPLASH_DURATION_MS = 3500;
const int NAV_DEBOUNCE_DELAY_MS = 200;

MenuManager::MenuManager(TFT_eSPI* tft_display, SoundManager* sound_manager, BatteryManager* battery_manager)
    : tft(tft_display), sound(sound_manager), battery(battery_manager) {
    currentState = MENU_STATE_SPLASH;
    selectedGameIndex = 0;
    lastBatteryUpdate = 0;
    lastNavInputTime = 0;
    lastLeftState = LOW;
    lastRightState = LOW;
    lastConfirmState = LOW;
    lastMuteActionState = LOW;
}

void MenuManager::begin() {
    currentState = MENU_STATE_SPLASH;
    drawSplashScreen();
    splashStartTime = millis();
    sound->play(MELODY_SPLASH, MELODY_SPLASH_LENGTH);
}

void MenuManager::returnToMenu() {
    currentState = MENU_STATE_SELECT;
    drawMenu();
    
    // Reset inputs to prevent input bleed from the game exit
    lastLeftState = digitalRead(BTN_LEFT);
    lastRightState = digitalRead(BTN_RIGHT);
    lastConfirmState = digitalRead(BTN_A);
    lastMuteActionState = digitalRead(BTN_B);
}

int MenuManager::update() {
    switch (currentState) {
        case MENU_STATE_SPLASH:
            if (millis() - splashStartTime > SPLASH_DURATION_MS) {
                currentState = MENU_STATE_SELECT;
                drawMenu();
            }
            return -1;

        case MENU_STATE_SELECT:
            if (millis() - lastBatteryUpdate > 5000) {
                lastBatteryUpdate = millis();
                updateBatteryDisplay();
            }
            return handleInput();
    }
    return -1;
}

int MenuManager::handleInput() {
    bool currentLeftState = digitalRead(BTN_LEFT);
    bool currentRightState = digitalRead(BTN_RIGHT);
    bool currentConfirmState = digitalRead(BTN_A);
    bool currentMuteActionState = digitalRead(BTN_B);

    // Mute toggle (Select + B)
    if (digitalRead(BTN_SELECT) == HIGH && currentMuteActionState == HIGH) {
        if (lastMuteActionState == LOW) { 
            sound->setMute(!sound->isMuted());
            tft->setTextColor(TFT_RED);
            tft->setTextSize(1);
            tft->setTextDatum(BR_DATUM);
            tft->drawString(sound->isMuted() ? "Sound OFF" : "Sound ON", tft->width() - 5, tft->height() - 5);
            delay(500);
            drawMenu();
        }
    }

    // Navegação Direita/Esquerda com Debounce
    if (lastLeftState == HIGH && currentLeftState == LOW) {
        if (millis() - lastNavInputTime > NAV_DEBOUNCE_DELAY_MS) {
            lastNavInputTime = millis();
            selectedGameIndex--;
            if (selectedGameIndex < 0) selectedGameIndex = GAME_COUNT - 1;
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            tft->fillScreen(MY_GREY);
            delay(40);
            drawMenu();
        }
    }

    if (lastRightState == HIGH && currentRightState == LOW) {
        if (millis() - lastNavInputTime > NAV_DEBOUNCE_DELAY_MS) {
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
    if (lastConfirmState == HIGH && currentConfirmState == LOW) {
        selectedGame = selectedGameIndex; 
    }

    lastLeftState = currentLeftState;
    lastRightState = currentRightState;
    lastConfirmState = currentConfirmState;
    lastMuteActionState = currentMuteActionState;

    return selectedGame;
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

void MenuManager::drawMenu() {
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_YELLOW);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("SELECT A GAME", tft->width() / 2, tft->height() / 4);
    
    String gameText = "< " + String(gameNames[selectedGameIndex]) + " >";
    tft->setTextColor(TFT_WHITE);
    tft->drawString(gameText, tft->width() / 2, tft->height() / 2);
    updateBatteryDisplay();
}

void MenuManager::updateBatteryDisplay() {
    tft->setTextDatum(TR_DATUM);
    tft->setTextColor(TFT_GREEN);
    tft->setTextSize(2);
    tft->fillRect(tft->width() - 60, 5, 60, 16, TFT_BLACK);
    tft->drawString(battery->getStatusString(), tft->width() - 5, 5);
}