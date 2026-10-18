#include "CrossyRoadGame.h"
#include "audio_assets.h"
#include "pins.h"

// Custom RGB565 colors for crisp minimalist readability
static const uint16_t COLOR_RIVER_WATER = 0x0011; // Deep dark navy blue for deadly water
static const uint16_t COLOR_LOG         = 0x79C0; // Rich dark wood brown (high contrast with yellow player)
static const uint16_t COLOR_CAR         = TFT_RED;
static const uint16_t COLOR_TRUCK       = 0x90DF; // Vibrant purple for buses/trucks
static const uint16_t COLOR_TRAIN       = TFT_CYAN;
static const uint16_t COLOR_RAIL_DIM    = 0x39E7; // Dim grey for inactive rail signal

static const float WRAP_SPAN = 400.0f; // 320px screen + 80px off-screen buffer

CrossyRoadGame::CrossyRoadGame(TFT_eSPI* tft_display, SoundManager* sound_manager)
    : Game(tft_display, sound_manager, "Crossy Road") {
    defaultScores[0] = 45;
    defaultScores[1] = 25;
    defaultScores[2] = 12;
}

uint16_t CrossyRoadGame::getLaneBgColor(LaneType type) const {
    return (type == LANE_RIVER) ? COLOR_RIVER_WATER : TFT_BLACK;
}

void CrossyRoadGame::resetGame() {
    score = 0;
    prevScore = -1;
    currentWorldRow = 0;
    maxWorldRow = 0;

    playerRow = 8; // Near bottom of the 10-lane playfield (rows 0..9)
    playerRowPrev = playerRow;
    playerX = 10 * COL_WIDTH + 2; // Centered at column 10
    playerXPrev = playerX;

    fullRedrawRequired = true;
    cameraShiftedThisFrame = false;
    playerHoppedThisFrame = false;

    // Generate initial 10 visible lanes from bottom (row 9 = worldDist -1) to top (row 0 = worldDist 8)
    for (int r = NUM_LANES - 1; r >= 0; r--) {
        int worldDist = (8 - r);
        const Lane* belowLane = (r < NUM_LANES - 1) ? &lanes[r + 1] : nullptr;
        generateLane(r, worldDist, belowLane);
    }

    // Ensure player's starting tile and immediate surroundings at row 8 are completely clear
    lanes[8].trees[9]  = false;
    lanes[8].trees[10] = false;
    lanes[8].trees[11] = false;
    lanes[7].trees[10] = false;
}

void CrossyRoadGame::generateLane(int rowIdx, int worldDist, const Lane* adjacentLane) {
    Lane& lane = lanes[rowIdx];
    int effDist = constrain(worldDist, 0, 60);

    // Clear default fields
    lane.obsCount = 0;
    lane.obsWidth = 0;
    lane.obsColor = TFT_WHITE;
    lane.speed = 0.0f;
    for (int c = 0; c < NUM_COLS; c++) {
        lane.trees[c] = false;
    }
    lane.railState = RAIL_COOLDOWN;
    lane.railTimer = millis();
    lane.cooldownDuration = 2500;
    lane.trainX = -TRAIN_WIDTH;
    lane.trainXPrev = -TRAIN_WIDTH;
    lane.trainNeedsErase = false;
    lane.warningLightOn = false;
    lane.warningLightPrev = false;

    // Determine lane type based on distance & solvability rules
    LaneType chosenType = LANE_GRASS;

    if (worldDist <= 1) {
        // Safe starting zone (rows 9, 8, 7)
        chosenType = LANE_GRASS;
    } else {
        // Count consecutive non-grass or river lanes to prevent impossible walls
        int consecutiveHazards = 0;
        int consecutiveRivers = 0;
        for (int checkR = rowIdx + 1; checkR < NUM_LANES; checkR++) {
            if (lanes[checkR].type != LANE_GRASS) consecutiveHazards++;
            else break;
        }
        for (int checkR = rowIdx + 1; checkR < NUM_LANES; checkR++) {
            if (lanes[checkR].type == LANE_RIVER) consecutiveRivers++;
            else break;
        }

        if (consecutiveHazards >= 4) {
            chosenType = LANE_GRASS;
        } else {
            int roll = random(0, 100);
            if (effDist < 10) {
                // Tier 1 (0..9): Gentle warmup (Grass, slow Roads, calm Rivers)
                if (roll < 42) chosenType = LANE_GRASS;
                else if (roll < 78) chosenType = LANE_ROAD;
                else chosenType = LANE_RIVER;
            } else if (effDist < 40) {
                // Tier 2 (10..39): Trains unlock, balanced arcade mix
                if (roll < 30) chosenType = LANE_GRASS;
                else if (roll < 63) chosenType = LANE_ROAD;
                else if (roll < 84) chosenType = LANE_RIVER;
                else chosenType = LANE_RAIL;
            } else {
                // Tier 3 (40..60+): High-intensity arcade mix
                if (roll < 24) chosenType = LANE_GRASS;
                else if (roll < 58) chosenType = LANE_ROAD;
                else if (roll < 80) chosenType = LANE_RIVER;
                else chosenType = LANE_RAIL;
            }

            if (chosenType == LANE_RIVER && consecutiveRivers >= 2) {
                chosenType = LANE_ROAD;
            }
        }
    }

    lane.type = chosenType;

    // Configure specific lane properties
    if (chosenType == LANE_GRASS) {
        int treeChance = (worldDist <= 1) ? 15 : (22 + (effDist * 14) / 60);
        int consecutiveTrees = 0;

        for (int c = 0; c < NUM_COLS; c++) {
            // Always frame outer border columns occasionally, but prevent 3+ adjacent trees
            if (consecutiveTrees >= 2) {
                lane.trees[c] = false;
                consecutiveTrees = 0;
            } else if (random(0, 100) < treeChance) {
                lane.trees[c] = true;
                consecutiveTrees++;
            } else {
                lane.trees[c] = false;
                consecutiveTrees = 0;
            }
        }

        // Solvability guarantee: ensure at least 3 open vertical pass-through columns in [2..17]
        int openPassColumns = 0;
        for (int c = 2; c <= 17; c++) {
            bool adjBlocked = (adjacentLane != nullptr && adjacentLane->type == LANE_GRASS && adjacentLane->trees[c]);
            if (!lane.trees[c] && !adjBlocked) {
                openPassColumns++;
            }
        }
        while (openPassColumns < 3) {
            int pickCol = random(3, 17);
            lane.trees[pickCol] = false;
            bool adjBlocked = (adjacentLane != nullptr && adjacentLane->type == LANE_GRASS && adjacentLane->trees[pickCol]);
            if (!adjBlocked) {
                openPassColumns++;
            }
        }
    }
    else if (chosenType == LANE_ROAD) {
        bool isTruckLane = (random(0, 100) < 35);
        lane.obsWidth = isTruckLane ? 32 : 18;
        lane.obsColor = isTruckLane ? COLOR_TRUCK : COLOR_CAR;
        lane.obsCount = isTruckLane ? 2 : random(2, 4); // 2 trucks or 2-3 cars

        float baseSpeed = (isTruckLane ? 0.85f : 1.05f) + (effDist / 60.0f) * 1.55f;
        float speedMag = baseSpeed + (random(0, 45) / 100.0f);

        // Alternate direction from adjacent road lane most of the time
        int dir = (random(0, 2) == 0) ? 1 : -1;
        if (adjacentLane != nullptr && adjacentLane->type == LANE_ROAD) {
            dir = (adjacentLane->speed > 0) ? -1 : 1;
        }
        lane.speed = speedMag * dir;

        float spacing = WRAP_SPAN / lane.obsCount;
        float startOffset = random(0, 100);
        for (int i = 0; i < lane.obsCount; i++) {
            lane.obsX[i] = fmod(startOffset + i * spacing, WRAP_SPAN) - lane.obsWidth;
            lane.obsXPrev[i] = lane.obsX[i];
        }
    }
    else if (chosenType == LANE_RIVER) {
        lane.obsWidth = 54 - (effDist * 12) / 60; // 54px in early game down to 42px in late game
        lane.obsColor = COLOR_LOG;
        lane.obsCount = 3;

        float speedMag = 0.70f + (effDist / 60.0f) * 1.10f + (random(0, 25) / 100.0f);
        int dir = (random(0, 2) == 0) ? 1 : -1;
        // Always alternate log direction on consecutive river lanes so logs cross cleanly
        if (adjacentLane != nullptr && adjacentLane->type == LANE_RIVER) {
            dir = (adjacentLane->speed > 0) ? -1 : 1;
        }
        lane.speed = speedMag * dir;

        float spacing = WRAP_SPAN / lane.obsCount;
        float startOffset = random(0, 120);
        for (int i = 0; i < lane.obsCount; i++) {
            lane.obsX[i] = fmod(startOffset + i * spacing, WRAP_SPAN) - lane.obsWidth;
            lane.obsXPrev[i] = lane.obsX[i];
        }
    }
    else if (chosenType == LANE_RAIL) {
        int dir = (random(0, 2) == 0) ? 1 : -1;
        lane.speed = 14.0f * dir;
        lane.railState = RAIL_COOLDOWN;
        lane.cooldownDuration = 2400 - (effDist * 900) / 60 + random(0, 800);
        // Stagger initial timer so newly spawned rails don't all fire at once
        lane.railTimer = millis() - random(0, lane.cooldownDuration / 2);
        lane.trainX = (dir > 0) ? -TRAIN_WIDTH : DISPLAY_WIDTH;
        lane.trainXPrev = lane.trainX;
    }
}

void CrossyRoadGame::shiftWorldDown() {
    for (int r = NUM_LANES - 1; r >= 1; r--) {
        lanes[r] = lanes[r - 1];
    }
    int topWorldDist = currentWorldRow + CAMERA_THRESHOLD_ROW;
    generateLane(0, topWorldDist, &lanes[1]);
}

void CrossyRoadGame::handleInputPlaying() {
    static bool lastUp = LOW, lastDown = LOW, lastLeft = LOW, lastRight = LOW, lastA = LOW;

    bool curUp    = digitalRead(BTN_UP);
    bool curDown  = digitalRead(BTN_DOWN);
    bool curLeft  = digitalRead(BTN_LEFT);
    bool curRight = digitalRead(BTN_RIGHT);
    bool curA     = digitalRead(BTN_A);

    // Prevent input bleed right after pressing [A] on the Title Screen
    if (millis() - stateTimer < 180) {
        lastUp = curUp; lastDown = curDown;
        lastLeft = curLeft; lastRight = curRight;
        lastA = curA;
        return;
    }

    int dRow = 0;
    int dCol = 0;

    if ((lastUp == LOW && curUp == HIGH) || (lastA == LOW && curA == HIGH)) {
        dRow = -1;
    } else if (lastDown == LOW && curDown == HIGH) {
        dRow = 1;
    } else if (lastLeft == LOW && curLeft == HIGH) {
        dCol = -1;
    } else if (lastRight == LOW && curRight == HIGH) {
        dCol = 1;
    }

    lastUp = curUp; lastDown = curDown;
    lastLeft = curLeft; lastRight = curRight;
    lastA = curA;

    if (dRow == 0 && dCol == 0) return;

    int targetRow = playerRow + dRow;
    // Block retreating off the bottom of the screen
    if (targetRow >= NUM_LANES || targetRow < 0) return;

    float targetX = playerX;

    if (dRow != 0) {
        // Vertical hop onto targetRow
        if (lanes[targetRow].type != LANE_RIVER) {
            // Snap horizontally to nearest 16px grid column when landing on Grass/Road/Rail
            int snappedCol = constrain((int)round((playerX - 2.0f) / (float)COL_WIDTH), 0, NUM_COLS - 1);
            if (lanes[targetRow].type == LANE_GRASS && lanes[targetRow].trees[snappedCol]) {
                return; // Blocked by tree
            }
            targetX = snappedCol * COL_WIDTH + 2;
        } else {
            // Landing on a River lane: preserve horizontal alignment
            targetX = constrain(playerX, 2.0f, (float)(DISPLAY_WIDTH - PLAYER_W - 2));
        }
    } else {
        // Horizontal hop (Left / Right) on current lane
        if (lanes[playerRow].type != LANE_RIVER) {
            int curCol = constrain((int)round((playerX - 2.0f) / (float)COL_WIDTH), 0, NUM_COLS - 1);
            int targetCol = curCol + dCol;
            if (targetCol < 0 || targetCol >= NUM_COLS) return;
            if (lanes[playerRow].type == LANE_GRASS && lanes[playerRow].trees[targetCol]) {
                return; // Blocked by tree
            }
            targetX = targetCol * COL_WIDTH + 2;
        } else {
            float nextX = playerX + dCol * COL_WIDTH;
            if (nextX < 0.0f || nextX > DISPLAY_WIDTH - PLAYER_W) return;
            targetX = nextX;
        }
    }

    // Apply validated hop
    playerX = targetX;
    playerHoppedThisFrame = true;

    if (dRow == -1) {
        currentWorldRow++;
        if (playerRow <= CAMERA_THRESHOLD_ROW) {
            shiftWorldDown();
            cameraShiftedThisFrame = true;
        } else {
            playerRow--;
        }

        if (currentWorldRow > maxWorldRow) {
            maxWorldRow = currentWorldRow;
            score = maxWorldRow;
            sound->play(MELODY_CROSSY_POINT, MELODY_CROSSY_POINT_LENGTH);
        } else {
            sound->play(MELODY_HOP, MELODY_HOP_LENGTH);
        }
    } else if (dRow == 1) {
        currentWorldRow--;
        playerRow++;
        sound->play(MELODY_HOP, MELODY_HOP_LENGTH);
    } else {
        sound->play(MELODY_HOP, MELODY_HOP_LENGTH);
    }
}

void CrossyRoadGame::updatePlaying() {
    unsigned long now = millis();

    for (int r = 0; r < NUM_LANES; r++) {
        Lane& lane = lanes[r];

        if (lane.type == LANE_ROAD || lane.type == LANE_RIVER) {
            for (int i = 0; i < lane.obsCount; i++) {
                lane.obsX[i] += lane.speed;

                // Wrap around cleanly while off-screen
                if (lane.speed > 0 && lane.obsX[i] >= DISPLAY_WIDTH + (WRAP_SPAN - DISPLAY_WIDTH - lane.obsWidth)) {
                    lane.obsX[i] -= WRAP_SPAN;
                } else if (lane.speed < 0 && lane.obsX[i] < -lane.obsWidth - (WRAP_SPAN - DISPLAY_WIDTH - lane.obsWidth)) {
                    lane.obsX[i] += WRAP_SPAN;
                }
            }
        }
        else if (lane.type == LANE_RAIL) {
            if (lane.railState == RAIL_COOLDOWN) {
                lane.warningLightOn = false;
                if (now - lane.railTimer >= lane.cooldownDuration) {
                    lane.railState = RAIL_WARNING;
                    lane.railTimer = now;
                    lane.warningLightOn = true;
                    sound->play(MELODY_TRAIN_WARN, MELODY_TRAIN_WARN_LENGTH);
                }
            }
            else if (lane.railState == RAIL_WARNING) {
                // Flash red signal light every 120ms for 900ms before train arrives
                lane.warningLightOn = (((now - lane.railTimer) / 120) % 2 == 0);
                if (now - lane.railTimer >= 900) {
                    lane.railState = RAIL_TRAIN;
                    lane.warningLightOn = true;
                    lane.trainX = (lane.speed > 0) ? -TRAIN_WIDTH : DISPLAY_WIDTH;
                    lane.trainXPrev = lane.trainX;
                }
            }
            else if (lane.railState == RAIL_TRAIN) {
                lane.trainX += lane.speed;
                if ((lane.speed > 0 && lane.trainX >= DISPLAY_WIDTH) ||
                    (lane.speed < 0 && lane.trainX <= -TRAIN_WIDTH)) {
                    lane.railState = RAIL_COOLDOWN;
                    lane.railTimer = now;
                    lane.warningLightOn = false;
                    lane.trainNeedsErase = true;
                }
            }
        }
    }

    // Carry player along if standing on a River lane
    if (lanes[playerRow].type == LANE_RIVER) {
        playerX += lanes[playerRow].speed;
    }

    checkCollisions();
}

void CrossyRoadGame::checkCollisions() {
    const Lane& curLane = lanes[playerRow];
    int pLeft  = (int)round(playerX);
    int pRight = pLeft + PLAYER_W;

    if (curLane.type == LANE_ROAD) {
        // Forgiving 1px inset hitbox for vehicles
        for (int i = 0; i < curLane.obsCount; i++) {
            int oLeft  = (int)round(curLane.obsX[i]);
            int oRight = oLeft + curLane.obsWidth;
            if (pRight - 1 > oLeft && pLeft + 1 < oRight) {
                sound->play(MELODY_HIT, MELODY_HIT_LENGTH);
                endGame();
                return;
            }
        }
    }
    else if (curLane.type == LANE_RIVER) {
        // Drift off-screen check
        if (pLeft < 0 || pRight > DISPLAY_WIDTH) {
            sound->play(MELODY_HIT, MELODY_HIT_LENGTH);
            endGame();
            return;
        }

        // Must overlap at least one floating log (require central 6px of player on log)
        bool onLog = false;
        for (int i = 0; i < curLane.obsCount; i++) {
            int logLeft  = (int)round(curLane.obsX[i]);
            int logRight = logLeft + curLane.obsWidth;
            if (pRight - 3 > logLeft && pLeft + 3 < logRight) {
                onLog = true;
                break;
            }
        }
        if (!onLog) {
            sound->play(MELODY_HIT, MELODY_HIT_LENGTH);
            endGame();
            return;
        }
    }
    else if (curLane.type == LANE_RAIL && curLane.railState == RAIL_TRAIN) {
        int tLeft  = (int)round(curLane.trainX);
        int tRight = tLeft + TRAIN_WIDTH;
        if (pRight - 1 > tLeft && pLeft + 1 < tRight) {
            sound->play(MELODY_HIT, MELODY_HIT_LENGTH);
            endGame();
            return;
        }
    }
}

// =================================================================
// ZERO-FLICKER DIFFERENTIAL RENDERER
// =================================================================

void CrossyRoadGame::drawClippedRect(int x, int y, int w, int h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    int x0 = max(0, x);
    int x1 = min(DISPLAY_WIDTH, x + w);
    if (x1 > x0) {
        tft->fillRect(x0, y, x1 - x0, h, color);
    }
}

void CrossyRoadGame::drawRectExcludingPlayer(int x, int y, int w, int h, uint16_t color, int rowIdx) {
    if (w <= 0 || h <= 0) return;
    int x0 = max(0, x);
    int x1 = min(DISPLAY_WIDTH, x + w);
    if (x1 <= x0) return;

    if (rowIdx != playerRow) {
        tft->fillRect(x0, y, x1 - x0, h, color);
        return;
    }

    int pLeft  = (int)round(playerX);
    int pRight = pLeft + PLAYER_W;

    if (x1 <= pLeft || x0 >= pRight) {
        tft->fillRect(x0, y, x1 - x0, h, color);
        return;
    }

    // Split horizontal span around player so player pixels are never overwritten with log/water color
    if (x0 < pLeft) {
        tft->fillRect(x0, y, pLeft - x0, h, color);
    }
    if (x1 > pRight) {
        tft->fillRect(pRight, y, x1 - pRight, h, color);
    }
}

void CrossyRoadGame::drawPlayerAt(int x, int row, uint16_t color) {
    drawClippedRect(x, laneTopY(row) + 2, PLAYER_W, PLAYER_H, color);
}

void CrossyRoadGame::drawHUD() {
    if (!fullRedrawRequired && score == prevScore) return;

    tft->fillRect(0, 0, DISPLAY_WIDTH, HUD_HEIGHT - 1, TFT_BLACK);
    tft->drawFastHLine(0, HUD_HEIGHT - 1, DISPLAY_WIDTH, TFT_WHITE);

    tft->setTextDatum(TC_DATUM);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    tft->drawString(String(score), DISPLAY_WIDTH / 2, 2);

    prevScore = score;
}

void CrossyRoadGame::drawLaneFull(int rowIdx) {
    Lane& lane = lanes[rowIdx];
    int yTop = laneTopY(rowIdx);
    int yEnt = yTop + 2;
    uint16_t bgColor = getLaneBgColor(lane.type);

    // Paint lane strip background (excluding player's current box if on this row so player never blinks)
    if (rowIdx != playerRow) {
        tft->fillRect(0, yTop, DISPLAY_WIDTH, LANE_HEIGHT, bgColor);
    } else {
        tft->fillRect(0, yTop, DISPLAY_WIDTH, 2, bgColor);
        tft->fillRect(0, yEnt + ENTITY_H, DISPLAY_WIDTH, LANE_HEIGHT - 2 - ENTITY_H, bgColor);
        drawRectExcludingPlayer(0, yEnt, DISPLAY_WIDTH, ENTITY_H, bgColor, rowIdx);
    }

    if (lane.type == LANE_GRASS) {
        for (int c = 0; c < NUM_COLS; c++) {
            if (lane.trees[c]) {
                drawRectExcludingPlayer(c * COL_WIDTH + 2, yEnt, PLAYER_W, ENTITY_H, TFT_GREEN, rowIdx);
            }
        }
    }
    else if (lane.type == LANE_ROAD || lane.type == LANE_RIVER) {
        for (int i = 0; i < lane.obsCount; i++) {
            int currX = (int)round(lane.obsX[i]);
            drawRectExcludingPlayer(currX, yEnt, lane.obsWidth, ENTITY_H, lane.obsColor, rowIdx);
            lane.obsXPrev[i] = lane.obsX[i];
        }
    }
    else if (lane.type == LANE_RAIL) {
        uint16_t lightCol = lane.warningLightOn ? TFT_RED : COLOR_RAIL_DIM;
        tft->fillRect(2, yTop + 4, 6, 6, lightCol);
        tft->fillRect(DISPLAY_WIDTH - 8, yTop + 4, 6, 6, lightCol);
        lane.warningLightPrev = lane.warningLightOn;

        if (lane.railState == RAIL_TRAIN) {
            int currX = (int)round(lane.trainX);
            drawRectExcludingPlayer(currX, yEnt, TRAIN_WIDTH, ENTITY_H, COLOR_TRAIN, rowIdx);
            lane.trainXPrev = lane.trainX;
        }
        lane.trainNeedsErase = false;
    }

    if (rowIdx == playerRow) {
        drawPlayerAt((int)round(playerX), playerRow, TFT_YELLOW);
    }
}

void CrossyRoadGame::drawLaneDelta(int rowIdx) {
    Lane& lane = lanes[rowIdx];
    int yTop = laneTopY(rowIdx);
    int yEnt = yTop + 2;
    uint16_t bgColor = getLaneBgColor(lane.type);

    if (lane.type == LANE_ROAD || lane.type == LANE_RIVER) {
        for (int i = 0; i < lane.obsCount; i++) {
            int prevX = (int)round(lane.obsXPrev[i]);
            int currX = (int)round(lane.obsX[i]);

            if (currX != prevX) {
                if (abs(currX - prevX) > 60) {
                    // Wrapped around off-screen: clear any edge remnant at prevX
                    drawRectExcludingPlayer(prevX, yEnt, lane.obsWidth, ENTITY_H, bgColor, rowIdx);
                } else if (currX > prevX) {
                    // Moved right: erase left trailing slice
                    drawRectExcludingPlayer(prevX, yEnt, currX - prevX, ENTITY_H, bgColor, rowIdx);
                } else {
                    // Moved left: erase right trailing slice
                    drawRectExcludingPlayer(currX + lane.obsWidth, yEnt, prevX - currX, ENTITY_H, bgColor, rowIdx);
                }
            }

            // Draw current obstacle block excluding player's box so the bird never flickers!
            drawRectExcludingPlayer(currX, yEnt, lane.obsWidth, ENTITY_H, lane.obsColor, rowIdx);
            lane.obsXPrev[i] = lane.obsX[i];
        }
    }
    else if (lane.type == LANE_RAIL) {
        if (lane.trainNeedsErase) {
            int prevX = (int)round(lane.trainXPrev);
            drawRectExcludingPlayer(prevX, yEnt, TRAIN_WIDTH, ENTITY_H, TFT_BLACK, rowIdx);
            lane.trainNeedsErase = false;
            lane.warningLightPrev = !lane.warningLightOn;
        }

        if (lane.railState == RAIL_TRAIN) {
            int prevX = (int)round(lane.trainXPrev);
            int currX = (int)round(lane.trainX);
            if (currX > prevX) {
                drawRectExcludingPlayer(prevX, yEnt, currX - prevX, ENTITY_H, TFT_BLACK, rowIdx);
            } else if (currX < prevX) {
                drawRectExcludingPlayer(currX + TRAIN_WIDTH, yEnt, prevX - currX, ENTITY_H, TFT_BLACK, rowIdx);
            }
            drawRectExcludingPlayer(currX, yEnt, TRAIN_WIDTH, ENTITY_H, COLOR_TRAIN, rowIdx);
            lane.trainXPrev = lane.trainX;
        }

        if (lane.warningLightOn != lane.warningLightPrev || lane.railState == RAIL_TRAIN || playerHoppedThisFrame) {
            uint16_t lightCol = lane.warningLightOn ? TFT_RED : COLOR_RAIL_DIM;
            tft->fillRect(2, yTop + 4, 6, 6, lightCol);
            tft->fillRect(DISPLAY_WIDTH - 8, yTop + 4, 6, 6, lightCol);
            lane.warningLightPrev = lane.warningLightOn;
        }
    }
}

void CrossyRoadGame::drawPlaying() {
    drawHUD();

    if (fullRedrawRequired || cameraShiftedThisFrame) {
        for (int r = 0; r < NUM_LANES; r++) {
            drawLaneFull(r);
        }
        fullRedrawRequired = false;
        cameraShiftedThisFrame = false;
        playerHoppedThisFrame = false;
    } else {
        int pxPrev = (int)round(playerXPrev);
        int pxCurr = (int)round(playerX);

        if (playerHoppedThisFrame && (pxCurr != pxPrev || playerRow != playerRowPrev)) {
            uint16_t oldBg = getLaneBgColor(lanes[playerRowPrev].type);
            drawRectExcludingPlayer(pxPrev, laneTopY(playerRowPrev) + 2, PLAYER_W, PLAYER_H, oldBg, playerRowPrev);
        } else if (pxCurr != pxPrev && lanes[playerRow].type == LANE_RIVER) {
            // Player is drifting on a River log: only erase trailing pixels that hang over water
            // (any trailing pixels on the log itself will be painted COLOR_LOG by drawLaneDelta!)
            int sliceX0 = (pxCurr > pxPrev) ? pxPrev : (pxCurr + PLAYER_W);
            int sliceX1 = (pxCurr > pxPrev) ? pxCurr : (pxPrev + PLAYER_W);
            int yEnt = laneTopY(playerRow) + 2;
            const Lane& rLane = lanes[playerRow];

            for (int x = sliceX0; x < sliceX1; x++) {
                if (x < 0 || x >= DISPLAY_WIDTH) continue;
                bool coveredByLog = false;
                for (int i = 0; i < rLane.obsCount; i++) {
                    int logL = (int)round(rLane.obsX[i]);
                    int logR = logL + rLane.obsWidth;
                    if (x >= logL && x < logR) {
                        coveredByLog = true;
                        break;
                    }
                }
                if (!coveredByLog) {
                    tft->fillRect(x, yEnt, 1, PLAYER_H, COLOR_RIVER_WATER);
                }
            }
        }

        // Update all moving lanes (automatically clips around the player's current box)
        for (int r = 0; r < NUM_LANES; r++) {
            drawLaneDelta(r);
        }

        // Draw player only when position changed (or on initial frame)
        if (pxCurr != pxPrev || playerRow != playerRowPrev || playerHoppedThisFrame) {
            drawPlayerAt(pxCurr, playerRow, TFT_YELLOW);
        }
        playerHoppedThisFrame = false;
    }

    playerXPrev = playerX;
    playerRowPrev = playerRow;
}

