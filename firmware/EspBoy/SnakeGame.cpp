#include "SnakeGame.h"
#include "audio_assets.h"

SnakeGame::SnakeGame(TFT_eSPI* tft_display, SoundManager* sound_manager) 
    : Game(tft_display, sound_manager, "Snake") {
}

void SnakeGame::resetGame() {
    score = 0;
    snakeLength = 4;
    currentDirection = RIGHT;
    pendingDirection = currentDirection;
    lastUpdate = 0;
    fullRedrawRequired = true; 

    for (int x = 0; x < GRID_WIDTH; x++)
        for (int y = 0; y < GRID_HEIGHT; y++)
            grid[x][y] = EMPTY;

    for (int i = 0; i < snakeLength; i++) {
        snake[i].x = (GRID_WIDTH / 2) - i;
        snake[i].y = (GRID_HEIGHT / 2);
        grid[snake[i].x][snake[i].y] = SNAKE;
    }

    for (int y = 0; y < GRID_HEIGHT; y++) {
        grid[0][y] = WALL;
        grid[GRID_WIDTH - 1][y] = WALL;
    }
    for (int x = 0; x < GRID_WIDTH; x++) {
        grid[x][0] = WALL;
        grid[x][GRID_HEIGHT - 1] = WALL;
    }

    spawnApple();
}

void SnakeGame::handleInputPlaying() {
    if (digitalRead(BTN_UP) == HIGH && currentDirection != DOWN) pendingDirection = UP;
    if (digitalRead(BTN_DOWN) == HIGH && currentDirection != UP) pendingDirection = DOWN;
    if (digitalRead(BTN_LEFT) == HIGH && currentDirection != RIGHT) pendingDirection = LEFT;
    if (digitalRead(BTN_RIGHT) == HIGH && currentDirection != LEFT) pendingDirection = RIGHT;
}

void SnakeGame::updatePlaying() {
    now = millis();
    if (now - lastUpdate < SPEED) return;
    lastUpdate = now;

    // Deleta o rabo na malha
    grid[snake[snakeLength - 1].x][snake[snakeLength - 1].y] = EMPTY;
    
    // Deleta o rabo na tela (apenas se não for redraw total)
    if (!fullRedrawRequired) {
        tft->fillRect(snake[snakeLength - 1].x * CELL_SIZE, snake[snakeLength - 1].y * CELL_SIZE + Y_OFFSET, CELL_SIZE, CELL_SIZE, TFT_BLACK);
    }

    // Anda o corpo
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Define nova posição da cabeça
    int dx = 0, dy = 0;
    currentDirection = pendingDirection;
    switch (currentDirection) {
        case UP: dy = -1; break;
        case DOWN: dy = 1; break;
        case LEFT: dx = -1; break;
        case RIGHT: dx = 1; break;
    }

    int newX = snake[0].x + dx;
    int newY = snake[0].y + dy;
    int collided = grid[newX][newY];

    // Atualiza a cabeça
    snake[0].x = newX;
    snake[0].y = newY;
    grid[newX][newY] = SNAKE;

    // Checa colisões críticas
    if (collided == SNAKE || collided == WALL) {
        sound->play(MELODY_GAME_OVER, MELODY_GAME_OVER_LENGTH);
        endGame();
        return;
    }

    // Checa colisão com maçã
    if (collided == APPLE) {
        snakeLength++;
        snake[snakeLength - 1] = snake[snakeLength - 2];
        Point newApple = spawnApple();
        score++;

        if (!fullRedrawRequired) {
            tft->fillRect(newApple.x * CELL_SIZE, newApple.y * CELL_SIZE + Y_OFFSET, CELL_SIZE, CELL_SIZE, TFT_RED);
            tft->fillRect(snake[0].x * CELL_SIZE, snake[0].y * CELL_SIZE + Y_OFFSET, CELL_SIZE, CELL_SIZE, TFT_DARKGREEN);
        }
        sound->play(MELODY_APPLE, MELODY_APPLE_LENGTH);
    } else {
        if (!fullRedrawRequired) {
            tft->fillRect(snake[0].x * CELL_SIZE, snake[0].y * CELL_SIZE + Y_OFFSET, CELL_SIZE, CELL_SIZE, TFT_GREEN);
        }
    }

    if (!fullRedrawRequired) {
        drawScoreHUD();
    }
}

void SnakeGame::drawPlaying() {
    // A maior parte do desenho acontece via deltas no updatePlaying para otimização.
    // O drawPlaying garante o setup inicial da tela quando o estado muda.
    if (fullRedrawRequired) {
        drawGrid();
        fullRedrawRequired = false;
    }
}

Point SnakeGame::spawnApple() {
    Point apple;
    do {
        apple.x = random(1, GRID_WIDTH - 1);
        apple.y = random(1, GRID_HEIGHT - 1);
    } while (grid[apple.x][apple.y] != EMPTY);

    grid[apple.x][apple.y] = APPLE;
    return apple;
}

void SnakeGame::drawGrid() {
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            uint16_t color;
            if (grid[x][y] == EMPTY) color = TFT_BLACK;
            else if (grid[x][y] == SNAKE) color = TFT_GREEN;
            else if (grid[x][y] == APPLE) color = TFT_RED;
            else if (grid[x][y] == WALL) color = TFT_WHITE;
            tft->fillRect(x * CELL_SIZE, y * CELL_SIZE + Y_OFFSET, CELL_SIZE, CELL_SIZE, color);
        }
    }
    tft->fillRect(0, 0, tft->width(), Y_OFFSET, TFT_BLACK);
    drawScoreHUD();
}

void SnakeGame::drawScoreHUD() {
    tft->fillRect(0, 0, tft->width(), Y_OFFSET, TFT_BLACK);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->setTextSize(1);
    tft->setTextDatum(MC_DATUM);
    String s = "Score: " + String(score);
    tft->drawString(s, tft->width() / 2, Y_OFFSET / 2);
}