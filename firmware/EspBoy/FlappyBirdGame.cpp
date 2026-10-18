#include "FlappyBirdGame.h"
#include "audio_assets.h"
#include "pins.h"

// Game constants (Tuned)
const float GRAVITY = 0.28;
const float FLAP_STRENGTH = -4.2;
const float PIPE_SPEED = 2.0;
const int PLAYER_X = 40;
const int PLAYER_SIZE = 12;
const int PIPE_WIDTH = 30;
const int PIPE_GAP = 55;
const int PIPE_SPACING = 120;
const int HUD_HEIGHT = 20;

// Safe boundaries for random gap center generation (gap_y)
const int MIN_GAP_Y = 55;
const int MAX_GAP_Y = 135;

FlappyBirdGame::FlappyBirdGame(TFT_eSPI* tft_display, SoundManager* sound_manager)
    : Game(tft_display, sound_manager, "Flappy Bird") {
}

void FlappyBirdGame::resetGame() {
    player_y = tft->height() / 2;
    player_y_prev = player_y;
    player_vel_y = 0;
    score = 0;
    fullRedrawRequired = true;

    for (int i = 0; i < 3; i++) {
        pipes[i].x = tft->width() + (i * PIPE_SPACING);
        pipes[i].x_prev = pipes[i].x;
        // 100% random generation within visible screen boundaries
        pipes[i].gap_y = random(MIN_GAP_Y, MAX_GAP_Y + 1);
        pipes[i].scored = false;
    }
}

void FlappyBirdGame::handleInputPlaying() {
    static bool lastButtonState = LOW;
    bool currentButtonState = digitalRead(BTN_A);

    if (lastButtonState == LOW && currentButtonState == HIGH) {
        player_vel_y = FLAP_STRENGTH;
        sound->play(MELODY_FLAP, MELODY_FLAP_LENGTH);
    }
    lastButtonState = currentButtonState;
}

void FlappyBirdGame::onResume() {
    // Clear the pause popup and redraw pipes, bird and HUD at their current positions
    fullRedrawRequired = true;
}

void FlappyBirdGame::updatePlaying() {
    player_vel_y += GRAVITY;
    player_y += player_vel_y;

    for (int i = 0; i < 3; i++) {
        pipes[i].x -= PIPE_SPEED;

        // Recycle pipe that went off-screen
        if (pipes[i].x < -PIPE_WIDTH) {
            pipes[i].x = pipes[(i + 2) % 3].x + PIPE_SPACING;
            // 100% random generation for new pipe
            pipes[i].gap_y = random(MIN_GAP_Y, MAX_GAP_Y + 1);
            pipes[i].scored = false;
        }

        // Scoring
        if (!pipes[i].scored && pipes[i].x + PIPE_WIDTH < PLAYER_X) {
            score++;
            pipes[i].scored = true;
            sound->play(MELODY_SCORE, MELODY_SCORE_LENGTH);
        }
    }

    checkCollisions();
}

void FlappyBirdGame::checkCollisions() {
    // Ceiling and floor collision
    if (player_y + PLAYER_SIZE > tft->height() || player_y < HUD_HEIGHT) {
        sound->play(MELODY_HIT, MELODY_HIT_LENGTH);
        endGame();
        return;
    }

    // Pipe collisions
    for (int i = 0; i < 3; i++) {
        if (PLAYER_X + PLAYER_SIZE > pipes[i].x && PLAYER_X < pipes[i].x + PIPE_WIDTH) {
            if (player_y < pipes[i].gap_y - PIPE_GAP / 2 || player_y + PLAYER_SIZE > pipes[i].gap_y + PIPE_GAP / 2) {
                sound->play(MELODY_HIT, MELODY_HIT_LENGTH);
                endGame();
                return;
            }
        }
    }
}

void FlappyBirdGame::drawPlaying() {
    if (fullRedrawRequired) {
        tft->fillScreen(TFT_BLACK);
        drawHUD();
        fullRedrawRequired = false;
        
        for (int i = 0; i < 3; i++) {
            if (pipes[i].x < tft->width()) {
                tft->fillRect(pipes[i].x, HUD_HEIGHT, PIPE_WIDTH, pipes[i].gap_y - PIPE_GAP / 2 - HUD_HEIGHT, TFT_GREEN);
                tft->fillRect(pipes[i].x, pipes[i].gap_y + PIPE_GAP / 2, PIPE_WIDTH, tft->height() - (pipes[i].gap_y + PIPE_GAP / 2), TFT_GREEN);
            }
        }
    } else {
        // Erase player trail
        if (player_y != player_y_prev) {
            int top_prev = round(player_y_prev);
            int top_curr = round(player_y);
            int dy = top_curr - top_prev;

            if (dy > 0) {
                tft->fillRect(PLAYER_X, top_prev, PLAYER_SIZE, dy, TFT_BLACK);
            } else if (dy < 0) {
                tft->fillRect(PLAYER_X, top_curr + PLAYER_SIZE, PLAYER_SIZE, -dy, TFT_BLACK);
            }
        }

        // Erase pipe trail        
        for (int i = 0; i < 3; i++) {
            float slice_x = pipes[i].x_prev + PIPE_WIDTH - PIPE_SPEED;
            if (slice_x < tft->width()) {
                tft->fillRect(slice_x, HUD_HEIGHT, PIPE_SPEED + 1, pipes[i].gap_y - PIPE_GAP / 2 - HUD_HEIGHT, TFT_BLACK);
                tft->fillRect(slice_x, pipes[i].gap_y + PIPE_GAP / 2, PIPE_SPEED + 1, tft->height() - (pipes[i].gap_y + PIPE_GAP / 2), TFT_BLACK);
            }
        }
        
        // Draw pipe        
        for (int i = 0; i < 3; i++) {
            if (pipes[i].x < tft->width() && pipes[i].x > -PIPE_WIDTH) {
                tft->fillRect(pipes[i].x, HUD_HEIGHT, PIPE_WIDTH, pipes[i].gap_y - PIPE_GAP / 2 - HUD_HEIGHT, TFT_GREEN);
                tft->fillRect(pipes[i].x, pipes[i].gap_y + PIPE_GAP / 2, PIPE_WIDTH, tft->height() - (pipes[i].gap_y + PIPE_GAP / 2), TFT_GREEN);
            }
        }
    }

    // Draw player
    tft->fillRect(PLAYER_X, round(player_y), PLAYER_SIZE, PLAYER_SIZE, TFT_YELLOW);
    drawHUD();
    tft->drawFastHLine(0, tft->height() - 1, tft->width(), TFT_WHITE);

    // Update previous states
    player_y_prev = player_y;
    for (int i = 0; i < 3; i++) {
        pipes[i].x_prev = pipes[i].x;
    }
}

void FlappyBirdGame::drawHUD() {
    tft->fillRect(0, 0, tft->width(), HUD_HEIGHT, TFT_BLACK);
    tft->drawFastHLine(0, HUD_HEIGHT - 1, tft->width(), TFT_WHITE);
    
    tft->setTextDatum(TC_DATUM);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    tft->drawString(String(score), tft->width() / 2, 3);
}