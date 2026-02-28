#include "MenuManager.h"
#include "audio_assets.h"

#define MY_ORANGE tft->color565(255, 55, 0)
#define MY_GREY tft->color565(30, 30, 40)
const int SPLASH_DURATION_MS = 3500;
const int NAV_DEBOUNCE_DELAY_MS = 250;
const int LOCKOUT_TIMER_MS = 250;

// Timer de bloqueio global para evitar input bleed nas transições de menu
static unsigned long menuStateTimer = 0;

MenuManager::MenuManager(TFT_eSPI* tft_display, SoundManager* sound_manager, BatteryManager* battery_manager)
    : tft(tft_display), sound(sound_manager), battery(battery_manager) {
    currentState = MENU_STATE_SPLASH;
    selectedGameIndex = 0;
    
    selectedSettingIndex = 0;
    currentBrightness = 100; 

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
    menuStateTimer = millis(); // Inicia o cooldown de transição
    drawMenu();
    
    // Limpa os inputs forçando a leitura atual para evitar bounce
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
            handleSettingsInput();
            return -1;

        case MENU_STATE_CONFIRM_RESET:
            handleConfirmResetInput();
            return -1;
    }
    return -1;
}

// =================================================================
// 1. LÓGICA DO CARROSSEL DE JOGOS
// =================================================================

int MenuManager::handleSelectInput() {
    bool currentLeftState = digitalRead(BTN_LEFT);
    bool currentRightState = digitalRead(BTN_RIGHT);
    bool currentConfirmState = digitalRead(BTN_A);
    bool currentSelectState = digitalRead(BTN_SELECT);

    // LOCKOUT TIMER: Ignora inputs logo após a mudança de estado
    if (millis() - menuStateTimer < LOCKOUT_TIMER_MS) {
        lastLeftState = currentLeftState; lastRightState = currentRightState;
        lastConfirmState = currentConfirmState; lastSelectState = currentSelectState;
        return -1;
    }

    // Ir para Configurações (Gatilho no PRESS: LOW para HIGH)
    if (lastSelectState == LOW && currentSelectState == HIGH) {
        currentState = MENU_STATE_SETTINGS;
        menuStateTimer = millis(); 
        selectedSettingIndex = 0; 
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        drawSettings();
        
        lastSelectState = currentSelectState;
        return -1;
    }

    // Navegação Direita/Esquerda
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
// 2. LÓGICA DE CONFIGURAÇÕES
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

    // Voltar para o Menu Principal (B)
    if (lastCancelState == LOW && currentCancelState == HIGH) {
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
        returnToMenu(); // returnToMenu atualiza o timer
        lastCancelState = currentCancelState;
        return;
    }

    // Navegação Cima/Baixo/Esquerda/Direita
    if (millis() - lastNavInputTime > NAV_DEBOUNCE_DELAY_MS) {
        if (lastUpState == LOW && currentUpState == HIGH) {
            lastNavInputTime = millis();
            selectedSettingIndex--;
            if (selectedSettingIndex < 0) selectedSettingIndex = SETTING_COUNT - 1;
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            drawSettings();
        }
        else if (lastDownState == LOW && currentDownState == HIGH) {
            lastNavInputTime = millis();
            selectedSettingIndex++;
            if (selectedSettingIndex >= SETTING_COUNT) selectedSettingIndex = 0;
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            drawSettings();
        }
        else if (lastLeftState == LOW && currentLeftState == HIGH) {
            lastNavInputTime = millis();
            if (selectedSettingIndex == 0) sound->setMute(!sound->isMuted());
            else if (selectedSettingIndex == 1) {
                currentBrightness -= 10;
                if (currentBrightness < 10) currentBrightness = 10;

                // Mapeamento Exponencial (Gamma) com piso de 40 e teto de 254
                float normalized = currentBrightness / 100.0;
                int pwm = 40 + (int)(214.0 * normalized * normalized);
                analogWrite(TFT_BL_PIN, pwm);
            }
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            drawSettings();
        }
        else if (lastRightState == LOW && currentRightState == HIGH) {
            lastNavInputTime = millis();
            if (selectedSettingIndex == 0) sound->setMute(!sound->isMuted());
            else if (selectedSettingIndex == 1) {
                currentBrightness += 10;
                if (currentBrightness > 100) currentBrightness = 100;

                float normalized = currentBrightness / 100.0;
                int pwm = 54.9 + (int)(200 * normalized * normalized);
                analogWrite(TFT_BL_PIN, pwm);
            }
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            drawSettings();
        }
    }

    // Interação com o botão A (Reset Highscores)
    if (lastConfirmState == LOW && currentConfirmState == HIGH) {
        if (selectedSettingIndex == 2) {
            sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
            currentState = MENU_STATE_CONFIRM_RESET;
            menuStateTimer = millis(); 
            drawConfirmReset();
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
        // TODO: Limpar EEPROM/Preferences
        sound->play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
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

// =================================================================
// 3. FUNÇÕES DE DESENHO (UI)
// =================================================================

void MenuManager::drawSettings() {
    tft->fillScreen(TFT_BLACK);
    
    tft->setTextColor(TFT_YELLOW);
    tft->setTextSize(2);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("-- SETTINGS --", tft->width() / 2, 10);

    int startY = 60;
    int spacingY = 30;

    // 0: Sound
    tft->setTextDatum(ML_DATUM);
    tft->setTextColor(selectedSettingIndex == 0 ? TFT_YELLOW : TFT_WHITE);
    tft->drawString(selectedSettingIndex == 0 ? "> Sound:" : "  Sound:", 20, startY);
    String soundStatus = sound->isMuted() ? "[ OFF ]" : "[ ON ]";
    tft->setTextDatum(MR_DATUM);
    tft->drawString(soundStatus, tft->width() - 20, startY);

    // 1: Brightness
    tft->setTextDatum(ML_DATUM);
    tft->setTextColor(selectedSettingIndex == 1 ? TFT_YELLOW : TFT_WHITE);
    tft->drawString(selectedSettingIndex == 1 ? "> Brightness:" : "  Brightness:", 20, startY + spacingY);
    String brightStatus = "< " + String(currentBrightness) + "% >";
    tft->setTextDatum(MR_DATUM);
    tft->drawString(brightStatus, tft->width() - 20, startY + spacingY);

    // 2: Reset Highscores
    tft->setTextDatum(ML_DATUM);
    tft->setTextColor(selectedSettingIndex == 2 ? TFT_YELLOW : TFT_WHITE);
    tft->drawString(selectedSettingIndex == 2 ? "> Highscores:" : "  Highscores:", 20, startY + spacingY * 2);
    tft->setTextDatum(MR_DATUM);
    tft->drawString("[ RESET ]", tft->width() - 20, startY + spacingY * 2);

    // Rodapé
    tft->drawFastHLine(0, tft->height() - 25, tft->width(), TFT_WHITE);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(1);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("[B] Back", tft->width() / 2, tft->height() - 12);
}

void MenuManager::drawConfirmReset() {
    tft->fillRect(tft->width() / 2 - 100, tft->height() / 2 - 45, 200, 90, TFT_BLACK);
    tft->drawRect(tft->width() / 2 - 100, tft->height() / 2 - 45, 200, 90, TFT_RED);
    
    tft->setTextColor(TFT_RED);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("RESET SCORES?", tft->width() / 2, tft->height() / 2 - 15);
    
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(1);
    tft->drawString("[A] Yes    [B] No", tft->width() / 2, tft->height() / 2 + 15);
}

void MenuManager::drawMenu() {
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_YELLOW);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("-- SELECT A GAME --", tft->width() / 2, tft->height() / 4);
    
    String gameText = "< " + String(gameNames[selectedGameIndex]) + " >";
    tft->setTextColor(TFT_WHITE);
    tft->drawString(gameText, tft->width() / 2, tft->height() / 2);
    
    tft->drawFastHLine(0, tft->height() - 25, tft->width(), TFT_WHITE);
    tft->setTextSize(1);
    tft->drawString("[A] Play    [SELECT] Settings", tft->width() / 2, tft->height() - 12);
    
    updateBatteryDisplay();
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
    tft->setTextDatum(TR_DATUM);
    tft->setTextColor(TFT_GREEN);
    tft->setTextSize(2);
    tft->fillRect(tft->width() - 60, 5, 60, 16, TFT_BLACK);
    tft->drawString(battery->getStatusString(), tft->width() - 5, 5);
}