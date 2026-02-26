#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include <TFT_eSPI.h>
#include "pins.h"
#include "SoundManager.h" 
#include "Game.h"

typedef struct {
    int x, y;
} Point;

enum GameState { STATE_TITLE, STATE_PLAYING, STATE_GAME_OVER };
enum Cell { EMPTY, WALL, SNAKE, APPLE };
enum Direction { UP, DOWN, LEFT, RIGHT };


class SnakeGame : public Game {
public:
    SnakeGame(TFT_eSPI* tft_display, SoundManager* sound_manager);

    void setup() override;
    void loop() override;
    // 'const' significa que esta função não modifica o estado do jogo.
    bool shouldExit() const override;

private:
    // --- VARIÁVEIS DE MEMBRO ---
    TFT_eSPI* tft;
    SoundManager* sound;

    bool _exit_request;
    // Constantes do jogo
    static const int DISPLAY_WIDTH = 320;
    static const int DISPLAY_HEIGHT = 170;
    static const int SPEED = 200;
    static const int CELL_SIZE = 10;
    static const int Y_OFFSET = CELL_SIZE;
    static const int GRID_WIDTH = DISPLAY_WIDTH / CELL_SIZE;
    static const int GRID_HEIGHT = (DISPLAY_HEIGHT - Y_OFFSET) / CELL_SIZE;

    // Estado do jogo
    int snakeLength;
    int grid[GRID_WIDTH][GRID_HEIGHT];
    Point snake[GRID_HEIGHT * GRID_WIDTH];
    bool gameOver;
    Direction currentDirection;
    Direction pendingDirection;
    unsigned long now;
    unsigned long lastUpdate;
    int score;
    GameState currentState;
    unsigned long gameOverStartTime;

    // --- FUNÇÕES PRIVADAS ---
    void update();
    void drawGrid();
    Point spawnApple();
    void titleScreen();
    void gameOverScreen();
    void drawScoreHUD();
    void readInput();
};

#endif // SNAKEGAME_H