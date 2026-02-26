#ifndef FLAPPYBIRDGAME_H
#define FLAPPYBIRDGAME_H

#include "Game.h"
#include "SoundManager.h"
#include <TFT_eSPI.h>

class FlappyBirdGame : public Game {
public:
    FlappyBirdGame(TFT_eSPI* tft_display, SoundManager* sound_manager);

    void setup() override;
    void loop() override;
    bool shouldExit() const override;

private:
    // Estrutura para representar um par de canos
    struct Pipe {
        float x;
        float x_prev; 
        int gap_y;
        bool scored;
    };

    // Funções de lógica interna do jogo
    void resetGame();
    void handleInput();
    void updateGame();
    void draw();
    void checkCollisions();
    void drawHUD();
    void drawTitleScreen();
    void drawGameOverScreen();

    // Ponteiros para os objetos principais do sistema
    TFT_eSPI* tft;
    SoundManager* sound;

    // Variáveis de estado do jogo
    enum GameState { STATE_TITLE, STATE_PLAYING, STATE_GAME_OVER };
    GameState currentState;
    bool _exit_request;
    int score;
    unsigned long gameOverTime;

    // Propriedades do jogador
    float player_y;
    float player_y_prev;
    float player_vel_y;

    // Propriedades dos canos
    Pipe pipes[3]; // Usamos um array de 3 canos que se reciclam
    // Flag para forçar um redesenho completo da tela
    bool fullRedrawRequired;
};

#endif // FLAPPYBIRDGAME_H