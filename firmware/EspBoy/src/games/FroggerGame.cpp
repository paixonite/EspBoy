#include "FroggerGame.h"
#include "../core/audio_assets.h"
#include "../core/pins.h"

static const uint16_t COLOR_SAFE_BANK     = 0x4208; // Dark slate/purple-grey safe zone
static const uint16_t COLOR_LOG_BROWN     = 0x8200; // Warm arcade wood brown
static const uint16_t COLOR_ARCADE_ORANGE = 0xFA00; // Deep vivid arcade orange

// X coordinates for the 5 Lilypad Goal Bays across the 320px width
// 6 water gaps of 30px + 5 bays of 28px = 180 + 140 = 320px
static const int BAY_X[5] = {30, 88, 146, 204, 262};

FroggerGame::FroggerGame(TFT_eSPI* tft_display, SoundManager* sound_manager, InputManager* input_manager)
    : Game(tft_display, sound_manager, input_manager, "Frogger") {
}

int FroggerGame::rowToY(int row) const {
    return HUD_HEIGHT + (row * ROW_HEIGHT) + 1;
}

uint16_t FroggerGame::getRowBgColor(int row) const {
    if (row >= 0 && row <= 5) return TFT_NAVY;
    if (row == 6 || row == 12) return COLOR_SAFE_BANK;
    return TFT_BLACK;
}

float FroggerGame::getSpeedMultiplier() const {
    float mult = 1.0f + (score * 0.06f);
    return (mult > 2.2f) ? 2.2f : mult;
}

uint16_t FroggerGame::getObstacleColor(const Lane& lane, const Obstacle& obs) const {
    if (!lane.isTurtle || !obs.isDivingGroup) {
        return lane.color;
    }
    if (obs.diveState == TURTLE_SUBMERGED) {
        return TFT_NAVY;
    }
    if (obs.diveState == TURTLE_WARNING) {
        // Blink between vivid orange and dark green every 180ms to warn before diving
        return ((gameMillis() / 180) % 2 == 0) ? COLOR_ARCADE_ORANGE : TFT_DARKGREEN;
    }
    return lane.color;
}

void FroggerGame::respawnFrog() {
    player_x = (DISPLAY_WIDTH - FROG_SIZE) / 2.0f;
    player_row = 12;
}

void FroggerGame::initLanes() {
    // 5 River Lanes (Rows 1..5)
    lanes[0] = {1, true,  false,  0.85f, 54, 3, COLOR_LOG_BROWN, {}};
    lanes[1] = {2, true,  true,  -0.75f, 36, 3, TFT_DARKGREEN,   {}};
    lanes[2] = {3, true,  false,  1.15f, 72, 2, COLOR_LOG_BROWN, {}};
    lanes[3] = {4, true,  false,  0.65f, 42, 3, COLOR_LOG_BROWN, {}};
    lanes[4] = {5, true,  true,  -0.90f, 36, 3, TFT_DARKGREEN,   {}};

    // 5 Road Lanes (Rows 7..11)
    lanes[5] = {7,  false, false, -0.90f, 44, 2, TFT_WHITE,           {}};
    lanes[6] = {8,  false, false,  1.40f, 18, 2, TFT_CYAN,            {}};
    lanes[7] = {9,  false, false, -0.80f, 20, 3, TFT_MAGENTA,         {}};
    lanes[8] = {10, false, false,  0.60f, 22, 3, COLOR_ARCADE_ORANGE, {}};
    lanes[9] = {11, false, false, -0.75f, 18, 3, TFT_RED,             {}};

    for (int l = 0; l < NUM_ACTIVE_LANES; l++) {
        float period = (float)(DISPLAY_WIDTH + lanes[l].width);
        float spacing = period / lanes[l].count;
        float offset = (l * 37) % 60;

        for (int i = 0; i < lanes[l].count; i++) {
            float startX = fmod(i * spacing + offset, period) - lanes[l].width;
            lanes[l].obstacles[i].x = startX;
            lanes[l].obstacles[i].x_prev = startX;
            lanes[l].obstacles[i].isDivingGroup = (lanes[l].isTurtle && i == 0);
            lanes[l].obstacles[i].diveState = TURTLE_SURFACED;
            lanes[l].obstacles[i].prevDiveState = TURTLE_SURFACED;
            lanes[l].obstacles[i].prevColor = lanes[l].color;
        }
    }
}

void FroggerGame::resetGame() {
    score = 0;
    prev_score = -1;
    fullRedrawRequired = true;

    respawnFrog();
    player_x_prev = player_x;
    player_row_prev = player_row;

    initLanes();
}

void FroggerGame::handleInputPlaying() {
    bool hopped = false;

    if (input->wasPressed(BTN_ID_UP) || input->wasPressed(BTN_ID_A)) {
        if (player_row > 0) {
            player_row--;
            hopped = true;
        }
    } else if (input->wasPressed(BTN_ID_DOWN)) {
        if (player_row < NUM_ROWS - 1) {
            player_row++;
            hopped = true;
        }
    } else if (input->wasPressed(BTN_ID_LEFT)) {
        player_x -= ROW_HEIGHT;
        if (player_x < 0.0f) player_x = 0.0f;
        hopped = true;
    } else if (input->wasPressed(BTN_ID_RIGHT)) {
        player_x += ROW_HEIGHT;
        if (player_x > DISPLAY_WIDTH - FROG_SIZE) {
            player_x = DISPLAY_WIDTH - FROG_SIZE;
        }
        hopped = true;
    }

    if (hopped) {
        sound->play(MELODY_FROGGER_HOP, MELODY_FROGGER_HOP_LENGTH);
    }
}

void FroggerGame::onResume() {
    // Repaint background, lanes, frog and HUD over the pause popup
    fullRedrawRequired = true;
}

void FroggerGame::drawTutorialContent() {
    drawTutorialHeader(tr(STR_TUT_SEC_CONTROLS), 37);
    drawTutorialLine(tr(STR_FR_TUT_C1), 49);
    drawTutorialLine(tr(STR_FR_TUT_C2), 61);

    drawTutorialHeader(tr(STR_TUT_SEC_OBJECTIVE), 77);
    drawTutorialLine(tr(STR_FR_TUT_O1), 89);
    drawTutorialLine(tr(STR_FR_TUT_O2), 101);
    drawTutorialLine(tr(STR_FR_TUT_O3), 113);
    drawTutorialLine(tr(STR_FR_TUT_O4), 125);
}

void FroggerGame::updateTurtleDiveStates() {
    float mult = getSpeedMultiplier();
    unsigned long surfacedLen = (unsigned long)(3000.0f / mult);
    unsigned long warningLen  = (unsigned long)(1200.0f / mult);
    unsigned long submergeLen = (unsigned long)(1100.0f / mult);
    unsigned long cycleLen    = surfacedLen + warningLen + submergeLen;

    unsigned long now = gameMillis();

    for (int l = 0; l < NUM_ACTIVE_LANES; l++) {
        if (!lanes[l].isTurtle) continue;

        // Phase-shift each turtle lane so both lanes don't submerge simultaneously
        unsigned long t = (now + (unsigned long)(lanes[l].row * 1700UL)) % cycleLen;
        TurtleDiveState state = TURTLE_SURFACED;
        if (t >= surfacedLen + warningLen) {
            state = TURTLE_SUBMERGED;
        } else if (t >= surfacedLen) {
            state = TURTLE_WARNING;
        }

        for (int i = 0; i < lanes[l].count; i++) {
            if (lanes[l].obstacles[i].isDivingGroup) {
                lanes[l].obstacles[i].diveState = state;
            }
        }
    }
}

void FroggerGame::updatePlaying() {
    updateTurtleDiveStates();

    float mult = getSpeedMultiplier();

    for (int l = 0; l < NUM_ACTIVE_LANES; l++) {
        Lane& lane = lanes[l];
        float step = lane.baseSpeed * mult;
        float period = (float)(DISPLAY_WIDTH + lane.width);

        // If the frog is on this river lane, drift horizontally with the flow
        if (lane.isRiver && player_row == lane.row) {
            player_x += step;
        }

        for (int i = 0; i < lane.count; i++) {
            Obstacle& obs = lane.obstacles[i];
            obs.x += step;

            if (step > 0.0f && obs.x >= DISPLAY_WIDTH) {
                obs.x -= period;
            } else if (step < 0.0f && obs.x <= -lane.width) {
                obs.x += period;
            }
        }
    }

    checkCollisionsAndRiver();
}

void FroggerGame::checkCollisionsAndRiver() {
    // 1. Top Goal Row (Row 0): Check if frog landed inside one of the 5 lilypad bays
    if (player_row == 0) {
        float frogCenter = player_x + (FROG_SIZE * 0.5f);
        bool landedInBay = false;

        for (int b = 0; b < NUM_BAYS; b++) {
            if (frogCenter >= BAY_X[b] - 2.0f && frogCenter <= BAY_X[b] + BAY_WIDTH + 2.0f) {
                landedInBay = true;
                break;
            }
        }

        if (landedInBay) {
            score++;
            sound->play(MELODY_FROGGER_GOAL, MELODY_FROGGER_GOAL_LENGTH);
            respawnFrog();
        } else {
            sound->play(MELODY_HIT, MELODY_HIT_LENGTH);
            endGame();
        }
        return;
    }

    // 2. River Lanes (Rows 1..5): Check screen-edge drift and platform support
    if (player_row >= 1 && player_row <= 5) {
        if (player_x < 0.0f || (player_x + FROG_SIZE) > DISPLAY_WIDTH) {
            sound->play(MELODY_HIT, MELODY_HIT_LENGTH);
            endGame();
            return;
        }

        bool onPlatform = false;
        for (int l = 0; l < NUM_ACTIVE_LANES; l++) {
            const Lane& lane = lanes[l];
            if (lane.row != player_row) continue;

            for (int i = 0; i < lane.count; i++) {
                const Obstacle& obs = lane.obstacles[i];
                if (lane.isTurtle && obs.isDivingGroup && obs.diveState == TURTLE_SUBMERGED) {
                    continue;
                }
                // Require at least 4px of horizontal overlap to stay supported
                if ((player_x + FROG_SIZE - 3.0f) > obs.x && (player_x + 3.0f) < (obs.x + lane.width)) {
                    onPlatform = true;
                    break;
                }
            }
            break;
        }

        if (!onPlatform) {
            sound->play(MELODY_HIT, MELODY_HIT_LENGTH);
            endGame();
            return;
        }
    }

    // 3. Road Lanes (Rows 7..11): Check vehicle AABB collisions
    if (player_row >= 7 && player_row <= 11) {
        for (int l = 0; l < NUM_ACTIVE_LANES; l++) {
            const Lane& lane = lanes[l];
            if (lane.row != player_row) continue;

            for (int i = 0; i < lane.count; i++) {
                const Obstacle& obs = lane.obstacles[i];
                if ((player_x + FROG_SIZE - 1.0f) > obs.x && (player_x + 1.0f) < (obs.x + lane.width)) {
                    sound->play(MELODY_HIT, MELODY_HIT_LENGTH);
                    endGame();
                    return;
                }
            }
            break;
        }
    }
}

void FroggerGame::drawSpanClippedAroundFrog(int y, int spanL, int spanR, int frogL, int frogR, bool clipFrog, uint16_t color) {
    if (spanR <= spanL) return;

    if (!clipFrog || frogR <= spanL || frogL >= spanR) {
        tft->fillRect(spanL, y, spanR - spanL, ENTITY_HEIGHT, color);
        return;
    }

    // Left segment before frog
    if (frogL > spanL) {
        tft->fillRect(spanL, y, frogL - spanL, ENTITY_HEIGHT, color);
    }
    // Right segment after frog
    if (frogR < spanR) {
        tft->fillRect(frogR, y, spanR - frogR, ENTITY_HEIGHT, color);
    }
}

void FroggerGame::drawGoalRow() {
    int goalY = HUD_HEIGHT;
    tft->fillRect(0, goalY, DISPLAY_WIDTH, ROW_HEIGHT, TFT_NAVY);

    for (int b = 0; b < NUM_BAYS; b++) {
        tft->fillRect(BAY_X[b], goalY, BAY_WIDTH, ROW_HEIGHT, TFT_DARKGREEN);
        tft->fillRect(BAY_X[b] + 4, goalY + 1, BAY_WIDTH - 8, ENTITY_HEIGHT, TFT_GREEN);
    }
}

void FroggerGame::drawStaticBackground() {
    // HUD bar
    tft->fillRect(0, 0, DISPLAY_WIDTH, HUD_HEIGHT, TFT_BLACK);

    // Row 0: Goal row with 5 lilypad bays
    drawGoalRow();

    // Rows 1..5: River zone (5 * 12px = 60px)
    tft->fillRect(0, HUD_HEIGHT + ROW_HEIGHT, DISPLAY_WIDTH, 5 * ROW_HEIGHT, TFT_NAVY);

    // Row 6: Median safe bank (12px)
    tft->fillRect(0, HUD_HEIGHT + 6 * ROW_HEIGHT, DISPLAY_WIDTH, ROW_HEIGHT, COLOR_SAFE_BANK);

    // Rows 7..11: Road zone (5 * 12px = 60px)
    tft->fillRect(0, HUD_HEIGHT + 7 * ROW_HEIGHT, DISPLAY_WIDTH, 5 * ROW_HEIGHT, TFT_BLACK);

    // Row 12: Start safe bank (12px)
    tft->fillRect(0, HUD_HEIGHT + 12 * ROW_HEIGHT, DISPLAY_WIDTH, ROW_HEIGHT, COLOR_SAFE_BANK);
}

void FroggerGame::drawHUD() {
    tft->fillRect(0, 0, DISPLAY_WIDTH, HUD_HEIGHT, TFT_BLACK);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->setTextSize(1);
    tft->setTextDatum(MC_DATUM);
    TextRenderer::draw(tft, I18n::fmt(STR_SCORE_FMT, score), DISPLAY_WIDTH / 2, HUD_HEIGHT / 2);
}

void FroggerGame::drawPlaying() {
    int newFrogL = round(player_x);
    int newFrogR = newFrogL + FROG_SIZE;
    int oldFrogL = round(player_x_prev);
    int oldFrogR = oldFrogL + FROG_SIZE;

    if (fullRedrawRequired) {
        drawStaticBackground();
        drawHUD();
        prev_score = score;

        for (int l = 0; l < NUM_ACTIVE_LANES; l++) {
            Lane& lane = lanes[l];
            int y = rowToY(lane.row);
            bool frogOnRow = (player_row == lane.row);

            for (int i = 0; i < lane.count; i++) {
                Obstacle& obs = lane.obstacles[i];
                int ix = round(obs.x);
                int visL = max(0, ix);
                int visR = min(DISPLAY_WIDTH, ix + lane.width);
                uint16_t col = getObstacleColor(lane, obs);

                if (col != TFT_NAVY) {
                    drawSpanClippedAroundFrog(y, visL, visR, newFrogL, newFrogR, frogOnRow, col);
                }
                obs.x_prev = obs.x;
                obs.prevDiveState = obs.diveState;
                obs.prevColor = col;
            }
        }

        tft->fillRect(newFrogL, rowToY(player_row), FROG_SIZE, FROG_SIZE, TFT_YELLOW);
        player_x_prev = player_x;
        player_row_prev = player_row;
        fullRedrawRequired = false;
        return;
    }

    // 1. Erase uncovered frog footprint
    if (player_row != player_row_prev) {
        int oldClampedL = max(0, oldFrogL);
        int oldClampedR = min(DISPLAY_WIDTH, oldFrogR);
        if (oldClampedR > oldClampedL) {
            tft->fillRect(oldClampedL, rowToY(player_row_prev), oldClampedR - oldClampedL, FROG_SIZE, getRowBgColor(player_row_prev));
        }
    } else if (newFrogL != oldFrogL) {
        uint16_t bg = getRowBgColor(player_row_prev);
        int y = rowToY(player_row_prev);
        if (newFrogL > oldFrogL) {
            int eraseL = max(0, oldFrogL);
            int eraseR = min(DISPLAY_WIDTH, min(oldFrogR, newFrogL));
            if (eraseR > eraseL) {
                tft->fillRect(eraseL, y, eraseR - eraseL, FROG_SIZE, bg);
            }
        } else {
            int eraseL = max(0, max(oldFrogL, newFrogR));
            int eraseR = min(DISPLAY_WIDTH, oldFrogR);
            if (eraseR > eraseL) {
                tft->fillRect(eraseL, y, eraseR - eraseL, FROG_SIZE, bg);
            }
        }
    }

    // 2. Differential update for all 10 active obstacle lanes
    for (int l = 0; l < NUM_ACTIVE_LANES; l++) {
        Lane& lane = lanes[l];
        int y = rowToY(lane.row);
        uint16_t laneBg = getRowBgColor(lane.row);
        bool frogOnRow = (player_row == lane.row);

        for (int i = 0; i < lane.count; i++) {
            Obstacle& obs = lane.obstacles[i];
            int oldX = round(obs.x_prev);
            int newX = round(obs.x);

            int oldL = max(0, oldX);
            int oldR = min(DISPLAY_WIDTH, oldX + lane.width);
            int newL = max(0, newX);
            int newR = min(DISPLAY_WIDTH, newX + lane.width);

            uint16_t curColor = getObstacleColor(lane, obs);

            // Erase trailing slice if previously visible
            if (oldR > oldL && !(lane.isTurtle && obs.isDivingGroup && obs.prevDiveState == TURTLE_SUBMERGED)) {
                if (newR <= newL || newL >= oldR || newR <= oldL) {
                    // Disjoint / wrapped off-screen
                    drawSpanClippedAroundFrog(y, oldL, oldR, newFrogL, newFrogR, frogOnRow, laneBg);
                } else {
                    if (newL > oldL) {
                        drawSpanClippedAroundFrog(y, oldL, newL, newFrogL, newFrogR, frogOnRow, laneBg);
                    }
                    if (newR < oldR) {
                        drawSpanClippedAroundFrog(y, newR, oldR, newFrogL, newFrogR, frogOnRow, laneBg);
                    }
                }
            }

            // Draw current obstacle span (or erase if turtle just submerged)
            if (newR > newL) {
                if (lane.isTurtle && obs.isDivingGroup && obs.diveState == TURTLE_SUBMERGED) {
                    if (obs.prevDiveState != TURTLE_SUBMERGED) {
                        drawSpanClippedAroundFrog(y, newL, newR, newFrogL, newFrogR, frogOnRow, TFT_NAVY);
                    }
                } else {
                    drawSpanClippedAroundFrog(y, newL, newR, newFrogL, newFrogR, frogOnRow, curColor);
                }
            }

            obs.x_prev = obs.x;
            obs.prevDiveState = obs.diveState;
            obs.prevColor = curColor;
        }
    }

    // 3. Draw Frog at current position
    int drawFrogL = max(0, newFrogL);
    int drawFrogR = min(DISPLAY_WIDTH, newFrogR);
    if (drawFrogR > drawFrogL) {
        tft->fillRect(drawFrogL, rowToY(player_row), drawFrogR - drawFrogL, FROG_SIZE, TFT_YELLOW);
    }

    // 4. Update HUD only when score changes
    if (score != prev_score) {
        drawHUD();
        prev_score = score;
    }

    player_x_prev = player_x;
    player_row_prev = player_row;
}
