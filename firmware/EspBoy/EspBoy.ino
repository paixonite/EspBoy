// BIBLIOTECAS
#include <TFT_eSPI.h>
#include <SPI.h>
#include "Game.h"
#include "pins.h"
#include "audio_assets.h"
#include "SoundManager.h"

// Jogos
#include "SnakeGame.h"
#include "FlappyBirdGame.h"

// Definições de cores personalizadas
#define MY_ORANGE tft.color565(255, 55, 0)
#define MY_GREY tft.color565(30, 30, 40)

// --- ESTADOS GLOBAIS DO SISTEMA ---
enum SystemState {
  STATE_SPLASH,
  STATE_MENU,
  STATE_IN_GAME
};
SystemState currentSystemState;

// --- OBJETOS GLOBAIS DO SISTEMA ---
TFT_eSPI tft = TFT_eSPI();
SoundManager soundManager(BUZZER_PIN);

// --- LÓGICA DA TELA DE SPLASH ---
const int SPLASH_DURATION_MS = 3500;
unsigned long splashStartTime;

// --- LÓGICA DO MENU ---
const char* gameNames[] = {"Snake", "Flappy Bird"};
const int gameCount = sizeof(gameNames) / sizeof(char*);
int selectedGameIndex = 0;
unsigned long lastBatteryUpdate = 0;

// Ponteiro para o objeto do jogo que está rodando.
Game* currentGame = nullptr;

// --- VARIÁVEIS DE ESTADO PARA O INPUT DO MENU ---
bool lastLeftState = LOW;
bool lastRightState = LOW;
bool lastConfirmState = LOW;
bool lastMuteActionState = LOW;

// --- VARIÁVEIS PARA O DEBOUNCE DO MENU ---
unsigned long lastNavInputTime = 0;
const int NAV_DEBOUNCE_DELAY_MS = 200;

// =================================================================
// FUNÇÕES DE DESENHO E LÓGICA
// =================================================================

// Esta função mapeia a faixa correta do ADC (aprox. 1900-2600)
// para a porcentagem da bateria de forma não-linear.
int calculateBatteryPercentage(int adcValue) {
    int percentage = 0;
    if (adcValue >= 2600) {
        percentage = 100;
    } else if (adcValue > 2500) {
        percentage = map(adcValue, 2500, 2600, 80, 100);
    } else if (adcValue > 2300) {
        percentage = map(adcValue, 2300, 2500, 30, 80);
    } else if (adcValue > 2000) {
        percentage = map(adcValue, 2000, 2300, 5, 30);
    } else if (adcValue > 1900) {
        percentage = map(adcValue, 1900, 2000, 0, 5);
    } else {
        percentage = 0;
    }
    return percentage;
}

// Verifica se a bateria está carregando, usando a faixa de ADC correta.
bool isCharging() {
    static int lastAdcValue = 0;
    static unsigned long lastCheckTime = 0;
    static bool chargingState = false;
    
    unsigned long currentTime = millis();

    if (currentTime - lastCheckTime > 3000) {
        int currentAdcValue = analogRead(BATTERY_SENSE_PIN);
        
        // Se a voltagem subiu, está carregando
        if (currentAdcValue > lastAdcValue && lastAdcValue > 0) {
            chargingState = true;
        } 
        // Se a voltagem caiu, está descarregando
        else if (currentAdcValue < lastAdcValue) {
            chargingState = false;
        }
        
        // Verificação extra: se a leitura do ADC está no máximo absoluto 
        // (que agora sabemos ser ~2600), consideramos que está carregando (ou cheio).
        // Usamos 2580 para ter uma pequena margem.
        if (currentAdcValue > 2580) {
            chargingState = true;
        }

        lastAdcValue = currentAdcValue;
        lastCheckTime = currentTime;
    }
    return chargingState;
}

// Agora usa as funções corrigidas.
String getBatteryStatus() {
    int currentAdcValue = analogRead(BATTERY_SENSE_PIN);
    int percentage = calculateBatteryPercentage(currentAdcValue);
    
    //if (isCharging()) {
    //   // Se estiver carregando, mostra a porcentagem e um '+'
    //   return String(percentage) + "%+"; 
    //} else {
    //   return String(percentage) + "%";
    //}

    return String(percentage) + "%";

}

void updateBatteryDisplay() {
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.fillRect(tft.width() - 60, 5, 60, 16, TFT_BLACK);
  String batteryText = getBatteryStatus();
  tft.drawString(batteryText, tft.width() - 5, 5);
}

void drawSplashScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(5);

  String title = "EspBoy";
  uint16_t rainbowColors[] = {TFT_RED, MY_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_BLUE, TFT_PURPLE};

  // Calcula a largura total da palavra para poder centralizá-la manualmente.
  int totalWidth = tft.textWidth(title);
  int cursorX = (tft.width() / 2) - (totalWidth / 2);
  int cursorY = tft.height() / 2 - 10;

  // Itera sobre cada letra para desenhá-la com uma cor diferente.
  for (int i = 0; i < title.length(); i++) {
    String letter = String(title.charAt(i));
    tft.setTextColor(rainbowColors[i % 6]);
    tft.setTextDatum(ML_DATUM);
    tft.drawString(letter, cursorX, cursorY);
    cursorX += tft.textWidth(letter);
  }

  // Desenha o texto "by Paixonite" embaixo.
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.drawString("by Paixonite", tft.width() / 2, tft.height() / 2 + 30);
}

void drawMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("SELECT A GAME", tft.width() / 2, tft.height() / 4);
  String gameText = "< " + String(gameNames[selectedGameIndex]) + " >";
  tft.setTextColor(TFT_WHITE);
  tft.drawString(gameText, tft.width() / 2, tft.height() / 2);
  updateBatteryDisplay();
}

void handleMenuInput() {
  // Lê o estado atual de todos os botões no início da função.
  bool currentLeftState = digitalRead(BTN_LEFT);
  bool currentRightState = digitalRead(BTN_RIGHT);
  bool currentConfirmState = digitalRead(BTN_A);
  bool currentMuteActionState = digitalRead(BTN_B);

  // Lógica para ativar/desativar o som (pressionando Select + B).
  if (digitalRead(BTN_SELECT) == HIGH && currentMuteActionState == HIGH) {
    if (lastMuteActionState == LOW) { 
      soundManager.setMute(!soundManager.isMuted());
      tft.setTextColor(TFT_RED);
      tft.setTextSize(1);
      tft.setTextDatum(BR_DATUM);
      if (soundManager.isMuted()) {
        tft.drawString("Sound OFF", tft.width() - 5, tft.height() - 5);
      } else {
        tft.drawString("Sound ON", tft.width() - 5, tft.height() - 5);
      }
      delay(500);
      drawMenu();
    }
  }

  // --- NAVEGAÇÃO PARA A ESQUERDA (COM DEBOUNCE) ---
  if (lastLeftState == HIGH && currentLeftState == LOW) {
    if (millis() - lastNavInputTime > NAV_DEBOUNCE_DELAY_MS) {
      lastNavInputTime = millis();
      selectedGameIndex--;
      if (selectedGameIndex < 0) {
        selectedGameIndex = gameCount - 1;
      }
      soundManager.play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
      tft.fillScreen(MY_GREY);
      delay(40);
      drawMenu();
    }
  }

  // --- NAVEGAÇÃO PARA A DIREITA (COM DEBOUNCE) ---
  if (lastRightState == HIGH && currentRightState == LOW) {
    if (millis() - lastNavInputTime > NAV_DEBOUNCE_DELAY_MS) {
      lastNavInputTime = millis();
      selectedGameIndex++;
      if (selectedGameIndex >= gameCount) {
        selectedGameIndex = 0;
      }
      soundManager.play(MELODY_MENU_NAVIGATE, MELODY_MENU_NAVIGATE_LENGTH);
      tft.fillScreen(MY_GREY);
      delay(40);
      drawMenu();
    }
  }

  // Seleção de jogo com o botão 'A'.
  if (lastConfirmState == HIGH && currentConfirmState == LOW) {
    switch (selectedGameIndex) {
      case 0: // Snake
        if (currentGame != nullptr) delete currentGame;
        currentGame = new SnakeGame(&tft, &soundManager);
        break;
      
      case 1: // Flappy
        if (currentGame != nullptr) delete currentGame;
        currentGame = new FlappyBirdGame(&tft, &soundManager);
        break;
    }

    currentGame->setup();
    currentSystemState = STATE_IN_GAME;
  }

  // Atualiza o estado anterior dos botões para o próximo ciclo do loop.
  lastLeftState = currentLeftState;
  lastRightState = currentRightState;
  lastConfirmState = currentConfirmState;
  lastMuteActionState = currentMuteActionState;
}

// =================================================================
// SETUP E LOOP PRINCIPAIS
// =================================================================

void setup() {
  // Inicialização de hardware.
  tft.init();
  tft.setRotation(1);
  Serial.begin(115200);

  // Configuração dos pinos dos botões.
  pinMode(BTN_UP, INPUT_PULLDOWN);
  pinMode(BTN_DOWN, INPUT_PULLDOWN);
  pinMode(BTN_LEFT, INPUT_PULLDOWN);
  pinMode(BTN_RIGHT, INPUT_PULLDOWN);
  pinMode(BTN_START, INPUT_PULLDOWN);
  pinMode(BTN_SELECT, INPUT_PULLDOWN);
  pinMode(BTN_A, INPUT_PULLDOWN);
  pinMode(BTN_B, INPUT_PULLDOWN);
  pinMode(BUZZER_PIN, OUTPUT);

  // Habilita o circuito de alimentação da bateria.
  pinMode(POWER_ENABLE_PIN, OUTPUT);
  digitalWrite(POWER_ENABLE_PIN, HIGH);

  // Configuração inicial do sistema.
  currentSystemState = STATE_SPLASH;
  drawSplashScreen();
  splashStartTime = millis();
  soundManager.play(MELODY_SPLASH, MELODY_SPLASH_LENGTH);
}

void loop() {
  soundManager.loop();

  // Roteia a lógica principal com base no estado atual do sistema.
  switch (currentSystemState) {
    case STATE_SPLASH:
      if (millis() - splashStartTime > SPLASH_DURATION_MS) {
        currentSystemState = STATE_MENU;
        drawMenu();
      }
      break;

    case STATE_MENU:
      handleMenuInput();
      if (millis() - lastBatteryUpdate > 5000) {
        lastBatteryUpdate = millis();
        updateBatteryDisplay(); // Reativei esta linha
      }
      break;

    case STATE_IN_GAME:
      if (currentGame != nullptr) {
        currentGame->loop();

        // Verifica se o jogo solicitou o encerramento.
        if (currentGame->shouldExit()) {
          delete currentGame;
          currentGame = nullptr;
          currentSystemState = STATE_MENU;
          drawMenu();
        }
      }
      break;
  }
}