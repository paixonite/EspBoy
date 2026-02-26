#ifndef SNAKEGAME_H
#define SNAKEGAME_H

#include "Game.h"

typedef struct {
    int x, y;
} Point;

enum Cell { EMPTY, WALL, SNAKE, APPLE };
enum Direction { UP, DOWN, LEFT, RIGHT };

class SnakeGame : public Game {
public:
    SnakeGame(TFT_eSPI* tft_display, SoundManager* sound_manager);

protected:
    void resetGame() override;
    void updatePlaying() override;
    void drawPlaying() override;
    void handleInputPlaying() override;

private:
    // Constantes do jogo
    static const int DISPLAY_WIDTH = 320;
    static const int DISPLAY_HEIGHT = 170;
    static const int SPEED = 200;
    static const int CELL_SIZE = 10;
    static const int Y_OFFSET = CELL_SIZE;
    static const int GRID_WIDTH = DISPLAY_WIDTH / CELL_SIZE;
    static const int GRID_HEIGHT = (DISPLAY_HEIGHT - Y_OFFSET) / CELL_SIZE;

    void drawGrid();
    Point spawnApple();
    void drawScoreHUD();

    // Estado específico do Snake
    int snakeLength;
    int grid[GRID_WIDTH][GRID_HEIGHT];
    Point snake[GRID_HEIGHT * GRID_WIDTH];
    Direction currentDirection;
    Direction pendingDirection;
    unsigned long now;
    unsigned long lastUpdate;
    bool fullRedrawRequired;
};

#endif // SNAKEGAME_H