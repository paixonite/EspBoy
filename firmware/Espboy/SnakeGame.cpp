#include "SnakeGame.h"
#include "audio_assets.h"

// A sintaxe ": tft(tft_display)" é uma "initializer list", mais eficiente em C++.
SnakeGame::SnakeGame(TFT_eSPI* tft_display, SoundManager* sound_manager) 
    : tft(tft_display), sound(sound_manager) {
    _exit_request = false;
}

void SnakeGame::setup() {
    score = 0;
    gameOver = false;
    snakeLength = 4;
    currentDirection = RIGHT;
    pendingDirection = currentDirection;
    lastUpdate = 0;

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
    // Define o estado inicial do jogo
    currentState = STATE_TITLE;
    titleScreen();
}

void SnakeGame::loop() {
    switch (currentState) {
        
        case STATE_TITLE:
            if (digitalRead(BTN_START) == HIGH) {
                currentState = STATE_PLAYING;
                drawGrid();
            }
            break;

        case STATE_PLAYING:
            readInput();

            now = millis();
            if (now - lastUpdate >= SPEED) {
                update();
                lastUpdate = now;
            }
            break;

        case STATE_GAME_OVER:
        if (millis() - gameOverStartTime > 3500) {
                setup();
            }
            break;
    }
}

void SnakeGame::update() {
    // Deleta o rabo
    grid[snake[snakeLength - 1].x][snake[snakeLength - 1].y] = EMPTY;

    tft->fillRect(snake[snakeLength - 1].x * CELL_SIZE, snake[snakeLength - 1].y * CELL_SIZE + Y_OFFSET, CELL_SIZE, CELL_SIZE, TFT_BLACK);

    // Anda o corpo
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Anda a cabeça baseado na direção
    int dx = 0, dy = 0;
    currentDirection = pendingDirection;
    switch (currentDirection) {
        case UP: dy = -1; break;
        case DOWN: dy = 1; break;
        case LEFT: dx = -1; break;
        case RIGHT: dx = 1; break;
    }

    // Checa colisões
    int newX = snake[0].x + dx;
    int newY = snake[0].y + dy;
    int collided = grid[newX][newY];

    // Anda com a cabeça 
    snake[0].x = newX;
    snake[0].y = newY;
    grid[newX][newY] = SNAKE;

    if (collided == SNAKE || collided == WALL) {
        currentState = STATE_GAME_OVER;
        gameOverStartTime = millis();
        sound->play(MELODY_GAME_OVER, MELODY_GAME_OVER_LENGTH);
        gameOverScreen();
        return;
    }

    if (collided == APPLE) {
        snakeLength++;
        snake[snakeLength - 1] = snake[snakeLength - 2];
        Point newApple = spawnApple();
        score++;

        tft->fillRect(
            newApple.x * CELL_SIZE,
            newApple.y * CELL_SIZE + Y_OFFSET,
            CELL_SIZE,
            CELL_SIZE,
            TFT_RED
        );
        
        sound->play(MELODY_APPLE, MELODY_APPLE_LENGTH);
        
        //Verde escuro caso tenha comido
        tft->fillRect(snake[0].x * CELL_SIZE, snake[0].y * CELL_SIZE + Y_OFFSET, CELL_SIZE, CELL_SIZE, TFT_DARKGREEN);
    } else  {
        tft->fillRect(snake[0].x * CELL_SIZE, snake[0].y * CELL_SIZE + Y_OFFSET, CELL_SIZE, CELL_SIZE, TFT_GREEN);
    }

    drawScoreHUD();
}

Point SnakeGame::spawnApple() {
  Point apple;
  do {
    apple.x = random(1, GRID_WIDTH - 1);
    apple.y = random(1, GRID_HEIGHT - 1);
  } while (grid[apple.x][apple.y] != EMPTY);

  grid[apple.x][apple.y] = APPLE;

  return apple; // Retorna a coordenada da nova maçã
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

void SnakeGame::titleScreen() {
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_YELLOW);
    tft->setTextSize(3);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Snake", tft->width() / 2, tft->height() / 2 - 20);
    tft->setTextSize(2);
    tft->setTextColor(TFT_WHITE);
    tft->drawString("Press 'Start'", tft->width() / 2, tft->height() / 2 + 20);
}

void SnakeGame::gameOverScreen() {
    sound->play(MELODY_GAME_OVER, MELODY_GAME_OVER_LENGTH);
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_RED);
    tft->setTextSize(3);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("GAME OVER", tft->width() / 2, tft->height() / 2 - 10);

    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    String s = "Score: " + String(score);
    tft->drawString(s, tft->width() / 2, tft->height() / 2 + 20);
}

void SnakeGame::drawScoreHUD() {
    tft->fillRect(0, 0, tft->width(), Y_OFFSET, TFT_BLACK);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->setTextSize(1);
    tft->setTextDatum(MC_DATUM);
    String s = "Score: " + String(score);
    tft->drawString(s, tft->width() / 2, Y_OFFSET / 2);
}

void SnakeGame::readInput() {
    if (digitalRead(BTN_UP) == HIGH && currentDirection != DOWN) pendingDirection = UP;
    if (digitalRead(BTN_DOWN) == HIGH && currentDirection != UP) pendingDirection = DOWN;
    if (digitalRead(BTN_LEFT) == HIGH && currentDirection != RIGHT) pendingDirection = LEFT;
    if (digitalRead(BTN_RIGHT) == HIGH && currentDirection != LEFT) pendingDirection = RIGHT;

    // Exit game (start + select)
    if (digitalRead(BTN_START) == HIGH && digitalRead(BTN_SELECT) == HIGH) {
        _exit_request = true; // levanta o sinalizador para sair
    }
}

bool SnakeGame::shouldExit() const {
    return _exit_request;
}