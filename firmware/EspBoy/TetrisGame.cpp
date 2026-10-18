#include "TetrisGame.h"
#include "audio_assets.h"
#include "pins.h"

// Custom RGB565 Palette for Crisp Retro Arcade Aesthetics
static const uint16_t COLOR_BG_NAVY      = 0x0045; // Really dark almost black navy blue (RGB: ~0, 8, 41)
static const uint16_t COLOR_GRID_DOT     = 0x2104; // Subtle dark slate guide dot
static const uint16_t COLOR_BORDER_OUTER = 0x4A69; // Steel grey outer frame
static const uint16_t COLOR_L_ORANGE     = 0xFB60; // Title screen orange (RGB: 255, 55, 0)

// 7 Tetromino Solid Colors: I, O, T, S, Z, J (Inverted L), L
static const uint16_t PIECE_COLORS[7] = {
    TFT_CYAN,       // 0: I - Cyan
    TFT_YELLOW,     // 1: O - Yellow
    0xC81F,         // 2: T - Purple
    TFT_GREEN,      // 3: S - Green
    TFT_RED,        // 4: Z - Red
    TFT_BLUE,       // 5: J (Inverted L) - Deep Blue
    COLOR_L_ORANGE  // 6: L - Orange
};

// Dimmed colors for Ghost Piece outline & locked Hold preview
static const uint16_t PIECE_DIM_COLORS[7] = {
    0x03EF, // 0: I
    0x7BE0, // 1: O
    0x6010, // 2: T
    0x03E0, // 3: S
    0x7800, // 4: Z
    0x0014, // 5: J (Deep Blue dimmed)
    0x79A0  // 6: L (Orange dimmed: RGB 128, 27, 0)
};

// SRS-style Tetromino Lookup Table: [7 pieces][4 rotations][4 minoes][2 coords: {col, row}]
static const int8_t TETROMINOES[7][4][4][2] = {
    // 0: I-Piece
    {
        {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
        {{2, 0}, {2, 1}, {2, 2}, {2, 3}},
        {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
        {{1, 0}, {1, 1}, {1, 2}, {1, 3}}
    },
    // 1: O-Piece
    {
        {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
        {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
        {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
        {{1, 0}, {2, 0}, {1, 1}, {2, 1}}
    },
    // 2: T-Piece
    {
        {{1, 0}, {0, 1}, {1, 1}, {2, 1}},
        {{1, 0}, {1, 1}, {2, 1}, {1, 2}},
        {{0, 1}, {1, 1}, {2, 1}, {1, 2}},
        {{1, 0}, {0, 1}, {1, 1}, {1, 2}}
    },
    // 3: S-Piece
    {
        {{1, 0}, {2, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {1, 1}, {2, 1}, {2, 2}},
        {{1, 1}, {2, 1}, {0, 2}, {1, 2}},
        {{0, 0}, {0, 1}, {1, 1}, {1, 2}}
    },
    // 4: Z-Piece
    {
        {{0, 0}, {1, 0}, {1, 1}, {2, 1}},
        {{2, 0}, {1, 1}, {2, 1}, {1, 2}},
        {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
        {{1, 0}, {0, 1}, {1, 1}, {0, 2}}
    },
    // 5: J-Piece (Inverted L)
    {
        {{0, 0}, {0, 1}, {1, 1}, {2, 1}},
        {{1, 0}, {2, 0}, {1, 1}, {1, 2}},
        {{0, 1}, {1, 1}, {2, 1}, {2, 2}},
        {{1, 0}, {1, 1}, {0, 2}, {1, 2}}
    },
    // 6: L-Piece
    {
        {{2, 0}, {0, 1}, {1, 1}, {2, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {2, 2}},
        {{0, 1}, {1, 1}, {2, 1}, {0, 2}},
        {{0, 0}, {1, 0}, {1, 1}, {1, 2}}
    }
};

// Candidate wall/floor kick offsets tested during rotation: {dx, dy}
static const int8_t WALL_KICKS[8][2] = {
    { 0,  0}, // 0: Basic rotation in place
    {-1,  0}, // 1: Kick left 1
    { 1,  0}, // 2: Kick right 1
    { 0, -1}, // 3: Floor kick up 1
    {-2,  0}, // 4: Kick left 2 (for I-piece near right wall)
    { 2,  0}, // 5: Kick right 2 (for I-piece near left wall)
    {-1, -1}, // 6: Diagonal up-left kick
    { 1, -1}  // 7: Diagonal up-right kick
};

TetrisGame::TetrisGame(TFT_eSPI* tft_display, SoundManager* sound_manager, InputManager* input_manager)
    : Game(tft_display, sound_manager, input_manager, "Tetris") {
}

void TetrisGame::refillBag() {
    for (int i = 0; i < 7; i++) {
        bag[i] = i;
    }
    // Fisher-Yates shuffle
    for (int i = 6; i > 0; i--) {
        int j = random(0, i + 1);
        int tmp = bag[i];
        bag[i] = bag[j];
        bag[j] = tmp;
    }
    bagIndex = 0;
}

int TetrisGame::pullFromBag() {
    if (bagIndex >= 7) {
        refillBag();
    }
    return bag[bagIndex++];
}

void TetrisGame::resetGame() {
    randomSeed(micros());

    score = 0;
    level = 1;
    linesCleared = 0;

    for (int r = 0; r < ROWS; r++) {
        clearingRowFlags[r] = false;
        for (int c = 0; c < COLS; c++) {
            board[r][c] = CELL_EMPTY;
            renderedGrid[r][c] = 255; // Force initial cell draw
        }
    }

    bagIndex = 7;
    refillBag();

    nextType = pullFromBag();
    holdType = -1;
    canHold = true;

    clearingLines = false;
    numClearingRows = 0;
    isSoftDropping = false;

    fullRedrawRequired = true;
    prevLevel = -1;
    prevLines = -1;
    prevNextType = -2;
    prevHoldType = -2;
    prevCanHold = true;

    // Sync input state
    hardDropPending = false;

    spawnNextPiece();
}

void TetrisGame::spawnNextPiece() {
    activeType = nextType;
    nextType   = pullFromBag();
    activeX    = 3;
    activeY    = 0;
    activeRot  = 0;

    unsigned long now = gameMillis();
    lastGravityTime  = now;
    lastSoftDropTime = now;
    lockTimerActive  = false;
    lockResetCount   = 0;

    if (!canFit(activeX, activeY, activeRot, activeType)) {
        // Flush final locked board & HUD before Game Over overlay is drawn
        if (!fullRedrawRequired) {
            drawHUDUpdates();
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    if (board[r][c] != renderedGrid[r][c]) {
                        drawCell(r, c, board[r][c]);
                        renderedGrid[r][c] = board[r][c];
                    }
                }
            }
        }
        sound->play(MELODY_GAME_OVER, MELODY_GAME_OVER_LENGTH);
        endGame();
    }
}

bool TetrisGame::canFit(int px, int py, int rot, int type) const {
    for (int i = 0; i < 4; i++) {
        int c = px + TETROMINOES[type][rot][i][0];
        int r = py + TETROMINOES[type][rot][i][1];
        if (c < 0 || c >= COLS || r < 0 || r >= ROWS) {
            return false;
        }
        if (board[r][c] != CELL_EMPTY) {
            return false;
        }
    }
    return true;
}

void TetrisGame::notifyPieceAdjusted() {
    if (lockTimerActive && lockResetCount < MAX_LOCK_RESETS) {
        lockStartTime = gameMillis();
        lockResetCount++;
    }
}

bool TetrisGame::tryMove(int dx, int dy) {
    if (canFit(activeX + dx, activeY + dy, activeRot, activeType)) {
        activeX += dx;
        activeY += dy;
        if (dx != 0) {
            notifyPieceAdjusted();
        }
        return true;
    }
    return false;
}

void TetrisGame::tryRotate(int dir) {
    int targetRot = (activeRot + dir + 4) % 4;

    for (int k = 0; k < 8; k++) {
        int kx = WALL_KICKS[k][0];
        int ky = WALL_KICKS[k][1];
        if (canFit(activeX + kx, activeY + ky, targetRot, activeType)) {
            activeX += kx;
            activeY += ky;
            activeRot = targetRot;
            notifyPieceAdjusted();
            sound->play(MELODY_TETRIS_ROTATE, MELODY_TETRIS_ROTATE_LENGTH);
            return;
        }
    }
}

void TetrisGame::triggerHold() {
    if (!canHold || clearingLines) return;

    sound->play(MELODY_TETRIS_HOLD, MELODY_TETRIS_HOLD_LENGTH);
    canHold = false;

    if (holdType == -1) {
        holdType = activeType;
        spawnNextPiece();
    } else {
        int swapType = activeType;
        activeType = holdType;
        holdType = swapType;

        activeX = 3;
        activeY = 0;
        activeRot = 0;
        unsigned long now = gameMillis();
        lastGravityTime  = now;
        lastSoftDropTime = now;
        lockTimerActive  = false;
        lockResetCount   = 0;

        if (!canFit(activeX, activeY, activeRot, activeType)) {
            sound->play(MELODY_GAME_OVER, MELODY_GAME_OVER_LENGTH);
            endGame();
        }
    }
}

int TetrisGame::computeGhostY() const {
    int gy = activeY;
    while (canFit(activeX, gy + 1, activeRot, activeType)) {
        gy++;
    }
    return gy;
}

unsigned long TetrisGame::getGravityInterval() const {
    static const unsigned long LEVEL_SPEEDS[15] = {
        750, 650, 560, 480, 410,
        350, 295, 245, 200, 165,
        135, 110,  90,  75,  60
    };
    int idx = constrain(level - 1, 0, 14);
    return LEVEL_SPEEDS[idx];
}

void TetrisGame::handleInputPlaying() {
    if (clearingLines) {
        isSoftDropping = false;
        return;
    }

    // 1. Hold Piece ([SELECT])
    if (input->wasPressed(BTN_ID_SELECT)) {
        triggerHold();
        if (currentState == STATE_GAME_OVER) return;
    }

    // 2. Rotations ([A] = Clockwise, [B] = Counter-Clockwise)
    if (input->wasPressed(BTN_ID_A)) {
        tryRotate(1);
    } else if (input->wasPressed(BTN_ID_B)) {
        tryRotate(-1);
    }

    // 3. Horizontal Movement with Delayed Auto Shift (DAS) & Auto Repeat Rate (ARR)
    if (input->isRepeated(BTN_ID_LEFT, DAS_DELAY_MS, ARR_RATE_MS)) {
        tryMove(-1, 0);
    } else if (input->isRepeated(BTN_ID_RIGHT, DAS_DELAY_MS, ARR_RATE_MS)) {
        tryMove(1, 0);
    }

    // 4. Soft Drop ([DOWN] held)
    isSoftDropping = input->isHeld(BTN_ID_DOWN);

    // 5. Hard Drop ([UP] edge-triggered, processed in updatePlaying)
    if (input->wasPressed(BTN_ID_UP)) {
        hardDropPending = true;
    }
}

void TetrisGame::onResume() {
    // Repaint frame, side panels and every board cell over the pause popup
    fullRedrawRequired = true;
}

void TetrisGame::lockPiece() {
    for (int i = 0; i < 4; i++) {
        int c = activeX + TETROMINOES[activeType][activeRot][i][0];
        int r = activeY + TETROMINOES[activeType][activeRot][i][1];
        if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
            board[r][c] = (uint8_t)(activeType + 1);
        }
    }

    canHold = true;
    lockTimerActive = false;

    // Scan for completed horizontal lines
    numClearingRows = 0;
    for (int r = 0; r < ROWS; r++) {
        bool rowFull = true;
        for (int c = 0; c < COLS; c++) {
            if (board[r][c] == CELL_EMPTY) {
                rowFull = false;
                break;
            }
        }
        clearingRowFlags[r] = rowFull;
        if (rowFull) {
            numClearingRows++;
        }
    }

    if (numClearingRows > 0) {
        clearingLines = true;
        lineClearStartTime = gameMillis();
        if (numClearingRows == 4) {
            sound->play(MELODY_TETRIS_FOUR, MELODY_TETRIS_FOUR_LENGTH);
        } else {
            sound->play(MELODY_TETRIS_LINE, MELODY_TETRIS_LINE_LENGTH);
        }
    } else {
        sound->play(MELODY_TETRIS_DROP, MELODY_TETRIS_DROP_LENGTH);
        spawnNextPiece();
    }
}

void TetrisGame::finishLineClear() {
    // Collapse non-cleared rows downward
    int writeRow = ROWS - 1;
    for (int readRow = ROWS - 1; readRow >= 0; readRow--) {
        if (!clearingRowFlags[readRow]) {
            if (writeRow != readRow) {
                for (int c = 0; c < COLS; c++) {
                    board[writeRow][c] = board[readRow][c];
                }
            }
            writeRow--;
        }
    }
    while (writeRow >= 0) {
        for (int c = 0; c < COLS; c++) {
            board[writeRow][c] = CELL_EMPTY;
        }
        writeRow--;
    }

    // Score is strictly the number of lines cleared
    linesCleared += numClearingRows;
    score = linesCleared;
    level = 1 + (linesCleared / 10);

    for (int r = 0; r < ROWS; r++) {
        clearingRowFlags[r] = false;
    }
    numClearingRows = 0;
    clearingLines = false;

    spawnNextPiece();
}

void TetrisGame::updatePlaying() {
    if (currentState == STATE_GAME_OVER) return;

    unsigned long now = gameMillis();

    if (clearingLines) {
        if (now - lineClearStartTime >= LINE_CLEAR_MS) {
            finishLineClear();
        }
        return;
    }

    if (hardDropPending) {
        hardDropPending = false;
        activeY = computeGhostY();
        lockPiece();
        return;
    }

    // Check if piece is resting on floor or locked stack
    bool grounded = !canFit(activeX, activeY + 1, activeRot, activeType);
    if (grounded) {
        if (!lockTimerActive) {
            lockTimerActive = true;
            lockStartTime = now;
        } else if (now - lockStartTime >= LOCK_DELAY_MS) {
            lockPiece();
            return;
        }
    } else {
        lockTimerActive = false;

        if (isSoftDropping) {
            if (now - lastSoftDropTime >= SOFT_DROP_RATE_MS) {
                lastSoftDropTime = now;
                lastGravityTime  = now;
                tryMove(0, 1);
            }
        } else {
            if (now - lastGravityTime >= getGravityInterval()) {
                lastGravityTime = now;
                tryMove(0, 1);
            }
        }
    }
}

// =================================================================
// ZERO-FLICKER DIFFERENTIAL RENDERER
// =================================================================

void TetrisGame::drawMinoTile(int px, int py, uint16_t color) {
    // Simple 7x7 solid color block with 1px black seam on right and bottom edges
    tft->fillRect(px, py, 7, 7, color);
    tft->drawFastHLine(px, py + 7, 8, TFT_BLACK);
    tft->drawFastVLine(px + 7, py, 7, TFT_BLACK);
}

void TetrisGame::drawCell(int r, int c, uint8_t state) {
    int px = BOARD_X + c * CELL_SIZE;
    int py = BOARD_Y + r * CELL_SIZE;

    if (state == CELL_EMPTY) {
        tft->fillRect(px, py, CELL_SIZE, CELL_SIZE, TFT_BLACK);
        tft->drawPixel(px + 3, py + 3, COLOR_GRID_DOT);
    }
    else if (state >= 1 && state <= 7) {
        int type = state - 1;
        drawMinoTile(px, py, PIECE_COLORS[type]);
    }
    else if (state >= CELL_GHOST_BASE && state < CELL_GHOST_BASE + 7) {
        int type = state - CELL_GHOST_BASE;
        tft->fillRect(px, py, CELL_SIZE, CELL_SIZE, TFT_BLACK);
        tft->drawRect(px, py, 7, 7, PIECE_DIM_COLORS[type]);
    }
    else if (state == CELL_FLASH_WHITE || state == CELL_FLASH_CYAN) {
        uint16_t flashCol = (state == CELL_FLASH_WHITE) ? TFT_WHITE : TFT_CYAN;
        drawMinoTile(px, py, flashCol);
    }
}

void TetrisGame::drawPreviewBox(int panelX, int boxY, int type, bool dimmed) {
    // Clear inner preview canvas (86 x 34 px)
    int innerX = panelX + 2;
    int innerY = boxY + 15;
    int innerW = PANEL_W - 4;
    int innerH = 34;
    tft->fillRect(innerX, innerY, innerW, innerH, TFT_BLACK);

    if (type < 0 || type > 6) return;

    // Compute tight bounding box of the piece in Rotation 0
    int minC = 4, maxC = 0, minR = 4, maxR = 0;
    for (int i = 0; i < 4; i++) {
        int c = TETROMINOES[type][0][i][0];
        int r = TETROMINOES[type][0][i][1];
        if (c < minC) minC = c;
        if (c > maxC) maxC = c;
        if (r < minR) minR = r;
        if (r > maxR) maxR = r;
    }

    int pieceW = (maxC - minC + 1) * CELL_SIZE;
    int pieceH = (maxR - minR + 1) * CELL_SIZE;
    int startX = innerX + (innerW - pieceW) / 2;
    int startY = innerY + (innerH - pieceH) / 2;

    uint16_t col = dimmed ? PIECE_DIM_COLORS[type] : PIECE_COLORS[type];

    for (int i = 0; i < 4; i++) {
        int c = TETROMINOES[type][0][i][0] - minC;
        int r = TETROMINOES[type][0][i][1] - minR;
        drawMinoTile(startX + c * CELL_SIZE, startY + r * CELL_SIZE, col);
    }
}

void TetrisGame::drawStaticLayout() {
    tft->fillScreen(COLOR_BG_NAVY);

    // Center Playfield Double Frame & Black Interior
    tft->fillRect(BOARD_X, BOARD_Y, BOARD_W, BOARD_H, TFT_BLACK);
    tft->drawRect(BOARD_X - 1, BOARD_Y - 1, BOARD_W + 2, BOARD_H + 2, TFT_WHITE);
    tft->drawRect(BOARD_X - 2, BOARD_Y - 2, BOARD_W + 4, BOARD_H + 4, COLOR_BORDER_OUTER);

    tft->setTextSize(1);
    tft->setTextDatum(MC_DATUM);

    // --- LEFT PANEL: HOLD, LEVEL, SCORE ---
    // 1. HOLD Box (y = 5..56)
    tft->fillRect(LEFT_PANEL_X, 5, PANEL_W, 52, TFT_BLACK);
    tft->drawRect(LEFT_PANEL_X, 5, PANEL_W, 52, TFT_WHITE);
    tft->drawFastHLine(LEFT_PANEL_X, 19, PANEL_W, TFT_WHITE);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->drawString("HOLD", LEFT_PANEL_X + PANEL_W / 2, 13);

    // 2. LEVEL Box (y = 64..110)
    tft->fillRect(LEFT_PANEL_X, 64, PANEL_W, 47, TFT_BLACK);
    tft->drawRect(LEFT_PANEL_X, 64, PANEL_W, 47, TFT_WHITE);
    tft->drawFastHLine(LEFT_PANEL_X, 78, PANEL_W, TFT_WHITE);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->drawString("LEVEL", LEFT_PANEL_X + PANEL_W / 2, 72);

    // 3. SCORE Box (y = 118..164)
    tft->fillRect(LEFT_PANEL_X, 118, PANEL_W, 47, TFT_BLACK);
    tft->drawRect(LEFT_PANEL_X, 118, PANEL_W, 47, TFT_WHITE);
    tft->drawFastHLine(LEFT_PANEL_X, 132, PANEL_W, TFT_WHITE);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->drawString("SCORE", LEFT_PANEL_X + PANEL_W / 2, 126);

    // --- RIGHT PANEL: NEXT & CONTROLS ---
    // 1. NEXT Box (y = 5..56)
    tft->fillRect(RIGHT_PANEL_X, 5, PANEL_W, 52, TFT_BLACK);
    tft->drawRect(RIGHT_PANEL_X, 5, PANEL_W, 52, TFT_WHITE);
    tft->drawFastHLine(RIGHT_PANEL_X, 19, PANEL_W, TFT_WHITE);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->drawString("NEXT", RIGHT_PANEL_X + PANEL_W / 2, 13);

    // 2. CONTROLS Box (y = 64..164)
    tft->fillRect(RIGHT_PANEL_X, 64, PANEL_W, 101, TFT_BLACK);
    tft->drawRect(RIGHT_PANEL_X, 64, PANEL_W, 101, TFT_WHITE);
    tft->drawFastHLine(RIGHT_PANEL_X, 78, PANEL_W, TFT_WHITE);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->drawString("CONTROLS", RIGHT_PANEL_X + PANEL_W / 2, 72);

    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->drawString("[A/B] Rotate",   RIGHT_PANEL_X + PANEL_W / 2, 96);
    tft->drawString("[UP] Hard Drop", RIGHT_PANEL_X + PANEL_W / 2, 120);
    tft->drawString("[SELECT] Hold",  RIGHT_PANEL_X + PANEL_W / 2, 144);
}

void TetrisGame::drawHUDUpdates() {
    tft->setTextDatum(MC_DATUM);

    // Update HOLD preview only when held piece or hold lock changes
    if (fullRedrawRequired || holdType != prevHoldType || canHold != prevCanHold) {
        drawPreviewBox(LEFT_PANEL_X, 5, holdType, !canHold);
        prevHoldType = holdType;
        prevCanHold  = canHold;
    }

    // Update NEXT preview only when next piece changes
    if (fullRedrawRequired || nextType != prevNextType) {
        drawPreviewBox(RIGHT_PANEL_X, 5, nextType, false);
        prevNextType = nextType;
    }

    // Update LEVEL value only when changed
    if (fullRedrawRequired || level != prevLevel) {
        tft->fillRect(LEFT_PANEL_X + 2, 79, PANEL_W - 4, 28, TFT_BLACK);
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
        tft->setTextSize(2);
        tft->drawString(String(level), LEFT_PANEL_X + PANEL_W / 2, 94);
        prevLevel = level;
    }

    // Update LINES value only when changed
    if (fullRedrawRequired || linesCleared != prevLines) {
        tft->fillRect(LEFT_PANEL_X + 2, 133, PANEL_W - 4, 28, TFT_BLACK);
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
        tft->setTextSize(2);
        tft->drawString(String(linesCleared), LEFT_PANEL_X + PANEL_W / 2, 148);
        prevLines = linesCleared;
    }
}

void TetrisGame::drawPlaying() {
    if (fullRedrawRequired) {
        drawStaticLayout();
    }

    drawHUDUpdates();

    // 1. Compose target 20x10 visual state in RAM
    uint8_t nextView[ROWS][COLS];
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            nextView[r][c] = board[r][c];
        }
    }

    if (clearingLines) {
        uint8_t flashState = (((gameMillis() - lineClearStartTime) / 55) % 2 == 0)
                             ? CELL_FLASH_WHITE
                             : CELL_FLASH_CYAN;
        for (int r = 0; r < ROWS; r++) {
            if (clearingRowFlags[r]) {
                for (int c = 0; c < COLS; c++) {
                    nextView[r][c] = flashState;
                }
            }
        }
    } else {
        // Project Ghost Piece first
        int ghostY = computeGhostY();
        if (ghostY != activeY) {
            uint8_t ghostState = (uint8_t)(CELL_GHOST_BASE + activeType);
            for (int i = 0; i < 4; i++) {
                int c = activeX + TETROMINOES[activeType][activeRot][i][0];
                int r = ghostY  + TETROMINOES[activeType][activeRot][i][1];
                if (r >= 0 && r < ROWS && c >= 0 && c < COLS && nextView[r][c] == CELL_EMPTY) {
                    nextView[r][c] = ghostState;
                }
            }
        }

        // Project Active Tetromino on top
        uint8_t activeState = (uint8_t)(activeType + 1);
        for (int i = 0; i < 4; i++) {
            int c = activeX + TETROMINOES[activeType][activeRot][i][0];
            int r = activeY + TETROMINOES[activeType][activeRot][i][1];
            if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                nextView[r][c] = activeState;
            }
        }
    }

    // 2. Differential SPI Flush: redraw ONLY cells whose visual state changed
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (fullRedrawRequired || nextView[r][c] != renderedGrid[r][c]) {
                drawCell(r, c, nextView[r][c]);
                renderedGrid[r][c] = nextView[r][c];
            }
        }
    }

    fullRedrawRequired = false;
}
