#ifndef TETRISGAME_H
#define TETRISGAME_H

#include "Game.h"

class TetrisGame : public Game {
public:
    TetrisGame(TFT_eSPI* tft_display, SoundManager* sound_manager, InputManager* input_manager);

protected:
    void resetGame() override;
    void updatePlaying() override;
    void drawPlaying() override;
    void handleInputPlaying() override;
    void onResume() override;
    void drawTutorialContent() override;

private:
    // Display & Playfield Geometry (320 x 170 landscape)
    static const int DISPLAY_WIDTH  = 320;
    static const int DISPLAY_HEIGHT = 170;
    static const int COLS           = 10;
    static const int ROWS           = 20;
    static const int CELL_SIZE      = 8;
    static const int BOARD_W        = COLS * CELL_SIZE; // 80px
    static const int BOARD_H        = ROWS * CELL_SIZE; // 160px
    static const int BOARD_X        = (DISPLAY_WIDTH - BOARD_W) / 2; // 120
    static const int BOARD_Y        = (DISPLAY_HEIGHT - BOARD_H) / 2; // 5

    // Side HUD Panel Geometry
    static const int LEFT_PANEL_X   = 14;
    static const int RIGHT_PANEL_X  = 216;
    static const int PANEL_W        = 90;

    // Cell Visual State Encoding for Differential Renderer
    static const uint8_t CELL_EMPTY       = 0;
    static const uint8_t CELL_GHOST_BASE  = 8;  // 8..14 = ghost for piece 0..6
    static const uint8_t CELL_FLASH_WHITE = 15;
    static const uint8_t CELL_FLASH_CYAN  = 16;

    // Timing Constants (ms)
    static const unsigned long DAS_DELAY_MS      = 165;
    static const unsigned long ARR_RATE_MS       = 48;
    static const unsigned long SOFT_DROP_RATE_MS = 45;
    static const unsigned long LOCK_DELAY_MS     = 450;
    static const int           MAX_LOCK_RESETS   = 8;
    static const unsigned long LINE_CLEAR_MS     = 160;

    // Board & Differential Display Buffers
    uint8_t board[ROWS][COLS];        // 0 = empty, 1..7 = locked piece color
    uint8_t renderedGrid[ROWS][COLS]; // Cached visual state on TFT for 0-flicker diffing

    // 7-Bag Randomizer State
    int bag[7];
    int bagIndex;

    // Active, Next & Hold Tetromino State
    int activeType;
    int activeX;
    int activeY;
    int activeRot;
    int nextType;
    int holdType;
    bool canHold;

    // Progression & Gravity State
    int level;
    int linesCleared;
    unsigned long lastGravityTime;
    unsigned long lastSoftDropTime;
    bool isSoftDropping;

    // Lock Delay State
    bool lockTimerActive;
    unsigned long lockStartTime;
    int lockResetCount;

    // Non-blocking Line Clear Flash State
    bool clearingLines;
    unsigned long lineClearStartTime;
    bool clearingRowFlags[ROWS];
    int numClearingRows;

    // Input State
    bool hardDropPending;

    // Differential HUD Tracking
    bool fullRedrawRequired;
    int prevLevel;
    int prevLines;
    int prevNextType;
    int prevHoldType;
    bool prevCanHold;

    // Gameplay Helpers
    void refillBag();
    int pullFromBag();
    void spawnNextPiece();
    bool canFit(int px, int py, int rot, int type) const;
    bool tryMove(int dx, int dy);
    void tryRotate(int dir);
    void triggerHold();
    int computeGhostY() const;
    unsigned long getGravityInterval() const;
    void notifyPieceAdjusted();
    void lockPiece();
    void finishLineClear();

    // Rendering Helpers
    void drawStaticLayout();
    void drawHUDUpdates();
    void drawPreviewBox(int panelX, int boxY, int type, bool dimmed);
    void drawMinoTile(int px, int py, uint16_t color);
    void drawCell(int r, int c, uint8_t state);
};

#endif // TETRISGAME_H
