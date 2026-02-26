#ifndef GAME_H
#define GAME_H

#include <TFT_eSPI.h>
#include "SoundManager.h"
#include "pins.h"

// Movemos o enum para cá para ser compartilhado por todos os jogos
enum GameState { STATE_TITLE, STATE_PLAYING, STATE_GAME_OVER };

class Game {
public:
    // Construtor base exigindo o nome do jogo para as telas padrão
    Game(TFT_eSPI* tft_display, SoundManager* sound_manager, const String& game_name);
    virtual ~Game() {}

    // Métodos públicos chamados pelo EspBoy.ino. 
    // NÃO são mais virtuais puros, pois a classe Game os implementa!
    void setup();
    void loop();
    bool shouldExit() const;

protected:
    // --- Atributos Compartilhados (Nenhum jogo filho precisa declarar isso de novo) ---
    TFT_eSPI* tft;
    SoundManager* sound;
    String gameName;
    GameState currentState;
    bool _exit_request;
    int score;
    unsigned long stateTimer; // Generaliza o gameOverTime/gameOverStartTime

    // --- Template Methods: O que cada jogo DEVE implementar ---
    virtual void resetGame() = 0;          // Zera variáveis, posições, cobra, canos, etc.
    virtual void updatePlaying() = 0;      // Lógica e física (roda durante STATE_PLAYING)
    virtual void drawPlaying() = 0;        // Renderização (roda durante STATE_PLAYING)
    virtual void handleInputPlaying() = 0; // Controles (roda durante STATE_PLAYING)

    // --- Métodos de Tela e Comportamento Padrão (Podem ser sobrescritos se necessário) ---
    virtual void drawTitleScreen();
    virtual void drawGameOverScreen();
    virtual void handleInputTitle();
    virtual void handleGameOverState();

    // --- Utilitários ---
    void changeState(GameState newState);
    void checkGlobalExit(); // Verifica Start + Select
};

#endif // GAME_H