#include "GeniusGame.h"
#include "audio_assets.h"
#include "pins.h"
#include <math.h>

// Pad Colors: [0: UP/Green, 1: RIGHT/Red, 2: DOWN/Yellow, 3: LEFT/Blue]
static const uint16_t PAD_DIM_COLOR[4] = {
    0x01E0, // Dark Forest Green (RGB: ~0, 60, 0)
    0x3800, // Dark Crimson Red (RGB: ~56, 0, 0)
    0x39E0, // Dark Ochre/Olive Yellow (RGB: ~56, 60, 0)
    0x000E  // Dark Deep Blue (RGB: ~0, 0, 112)
};

static const uint16_t PAD_LIT_COLOR[4] = {
    TFT_GREEN,  // Bright Green
    TFT_RED,    // Bright Red
    TFT_YELLOW, // Bright Yellow
    0x1D5F      // Bright Electric Blue
};

GeniusGame::GeniusGame(TFT_eSPI* tft_display, SoundManager* sound_manager, InputManager* input_manager)
    : Game(tft_display, sound_manager, input_manager, "Genius") {
    defaultScores[0] = 15;
    defaultScores[1] = 10;
    defaultScores[2] = 5;
}

void GeniusGame::resetGame() {
    randomSeed(micros());

    score = 0;
    roundLength = 0;
    seqIndex = 0;
    inputIndex = 0;
    activePad = -1;
    pendingPressPad = -1;
    playerFlashStart = 0;

    appendRandomStep();

    phase = PHASE_ROUND_INTRO;
    phaseStartTime = gameMillis();

    fullRedrawRequired = true;
    initialStart = true;
    for (int i = 0; i < 4; i++) {
        renderedPadLit[i] = false;
    }

    prevScore     = -1;
    prevPhaseCode = -1;
}

void GeniusGame::appendRandomStep() {
    if (roundLength < MAX_SEQUENCE) {
        sequence[roundLength] = (uint8_t)random(0, 4);
        roundLength++;
    }
}

void GeniusGame::playPadTone(int padIndex) {
    switch (padIndex) {
        case PAD_UP:
            sound->play(MELODY_GENIUS_GREEN, MELODY_GENIUS_GREEN_LENGTH);
            break;
        case PAD_RIGHT:
            sound->play(MELODY_GENIUS_RED, MELODY_GENIUS_RED_LENGTH);
            break;
        case PAD_DOWN:
            sound->play(MELODY_GENIUS_YELLOW, MELODY_GENIUS_YELLOW_LENGTH);
            break;
        case PAD_LEFT:
            sound->play(MELODY_GENIUS_BLUE, MELODY_GENIUS_BLUE_LENGTH);
            break;
    }
}

int GeniusGame::getSpeedTier() const {
    if (roundLength >= 13) return 4;
    if (roundLength >= 9)  return 3;
    if (roundLength >= 5)  return 2;
    return 1;
}

unsigned long GeniusGame::getStepOnDuration() const {
    switch (getSpeedTier()) {
        case 1:  return 420;
        case 2:  return 330;
        case 3:  return 260;
        default: return 200;
    }
}

unsigned long GeniusGame::getStepGapDuration() const {
    switch (getSpeedTier()) {
        case 1:  return 130;
        case 2:  return 105;
        case 3:  return 85;
        default: return 70;
    }
}

bool GeniusGame::isPadHeld(int padIndex) const {
    switch (padIndex) {
        case PAD_UP:    return input->isHeld(BTN_ID_UP);
        case PAD_RIGHT: return input->isHeld(BTN_ID_RIGHT);
        case PAD_DOWN:  return input->isHeld(BTN_ID_DOWN);
        case PAD_LEFT:  return input->isHeld(BTN_ID_LEFT);
        default:        return false;
    }
}

void GeniusGame::handleInputPlaying() {
    if (initialStart) {
        pendingPressPad = -1;
        return;
    }

    if (phase == PHASE_PLAYER_INPUT) {
        if (input->wasPressed(BTN_ID_UP)) {
            pendingPressPad = PAD_UP;
        } else if (input->wasPressed(BTN_ID_RIGHT)) {
            pendingPressPad = PAD_RIGHT;
        } else if (input->wasPressed(BTN_ID_DOWN)) {
            pendingPressPad = PAD_DOWN;
        } else if (input->wasPressed(BTN_ID_LEFT)) {
            pendingPressPad = PAD_LEFT;
        }
    } else {
        pendingPressPad = -1;
    }
}

void GeniusGame::onResume() {
    fullRedrawRequired = true;
}

void GeniusGame::drawTutorialContent() {
    drawTutorialHeader(tr(STR_TUT_SEC_CONTROLS), 37);
    drawTutorialLine(tr(STR_GN_TUT_C1), 49);
    drawTutorialLine(tr(STR_GN_TUT_C2), 61);

    drawTutorialHeader(tr(STR_TUT_SEC_OBJECTIVE), 77);
    drawTutorialLine(tr(STR_GN_TUT_O1), 89);
    drawTutorialLine(tr(STR_GN_TUT_O2), 101);
    drawTutorialLine(tr(STR_GN_TUT_O3), 113);
    drawTutorialLine(tr(STR_GN_TUT_O4), 125);
}

void GeniusGame::updatePlaying() {
    unsigned long now = gameMillis();

    if (initialStart) {
        randomSeed(micros());
        sequence[0] = (uint8_t)random(0, 4);
        phaseStartTime = now;
        initialStart = false;
        return;
    }

    unsigned long elapsed = now - phaseStartTime;

    switch (phase) {
        case PHASE_ROUND_INTRO:
            activePad = -1;
            if (elapsed >= 550) {
                seqIndex = 0;
                activePad = sequence[seqIndex];
                playPadTone(activePad);
                phase = PHASE_SEQ_ON;
                phaseStartTime = now;
            }
            break;

        case PHASE_SEQ_ON:
            if (elapsed >= getStepOnDuration()) {
                activePad = -1;
                phase = PHASE_SEQ_GAP;
                phaseStartTime = now;
            }
            break;

        case PHASE_SEQ_GAP:
            if (elapsed >= getStepGapDuration()) {
                seqIndex++;
                if (seqIndex < roundLength) {
                    activePad = sequence[seqIndex];
                    playPadTone(activePad);
                    phase = PHASE_SEQ_ON;
                    phaseStartTime = now;
                } else {
                    activePad = -1;
                    inputIndex = 0;
                    pendingPressPad = -1;
                    phase = PHASE_PLAYER_INPUT;
                    phaseStartTime = now;
                }
            }
            break;

        case PHASE_PLAYER_INPUT:
            if (activePad != -1) {
                unsigned long flashElapsed = now - playerFlashStart;
                if (flashElapsed >= 240 || (flashElapsed >= 95 && !isPadHeld(activePad))) {
                    activePad = -1;
                }
            }

            if (pendingPressPad != -1) {
                int pressed = pendingPressPad;
                pendingPressPad = -1;

                if (pressed == (int)sequence[inputIndex]) {
                    activePad = pressed;
                    playerFlashStart = now;
                    playPadTone(pressed);
                    inputIndex++;

                    if (inputIndex >= roundLength) {
                        phase = PHASE_ROUND_SUCCESS;
                        phaseStartTime = now;
                    }
                } else {
                    activePad = -1;
                    sound->play(MELODY_GENIUS_ERROR, MELODY_GENIUS_ERROR_LENGTH);
                    phase = PHASE_MISTAKE;
                    phaseStartTime = now;
                }
            }
            break;

        case PHASE_ROUND_SUCCESS:
            if (activePad != -1 && elapsed >= 230) {
                activePad = -1;
                score++;
                sound->play(MELODY_GENIUS_SUCCESS, MELODY_GENIUS_SUCCESS_LENGTH);
            }
            if (elapsed >= 760) {
                appendRandomStep();
                seqIndex = 0;
                inputIndex = 0;
                phase = PHASE_ROUND_INTRO;
                phaseStartTime = now;
            }
            break;

        case PHASE_MISTAKE:
            if (elapsed >= 900) {
                for (int i = 0; i < 4; i++) {
                    if (renderedPadLit[i]) {
                        drawPad(i, false);
                        renderedPadLit[i] = false;
                    }
                }
                endGame();
                return;
            }
            break;
    }
}

// =================================================================
// ZERO-FLICKER DIFFERENTIAL RENDERER
// =================================================================

int GeniusGame::getPhaseVisualCode() const {
    switch (phase) {
        case PHASE_ROUND_INTRO:
        case PHASE_SEQ_ON:
        case PHASE_SEQ_GAP:
            return 0; // Watch (Cyan)
        case PHASE_PLAYER_INPUT:
            return 1; // Player Turn (Green)
        case PHASE_ROUND_SUCCESS:
            return 2; // Round Cleared (Yellow)
        case PHASE_MISTAKE:
            return 3; // Mistake (Red)
        default:
            return 0;
    }
}

void GeniusGame::drawHUD() {
    tft->fillRect(0, 0, tft->width(), HUD_HEIGHT, TFT_BLACK);
    tft->drawFastHLine(0, HUD_HEIGHT - 1, tft->width(), TFT_WHITE);

    tft->setTextDatum(TC_DATUM);
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(2);
    tft->drawString(String(score), tft->width() / 2, 3);
}

void GeniusGame::drawCenterPhaseHub(int phaseCode) {
    uint16_t phaseCol = TFT_CYAN;
    if (phaseCode == 1)      phaseCol = TFT_GREEN;
    else if (phaseCode == 2) phaseCol = TFT_MAGENTA;
    else if (phaseCode == 3) phaseCol = TFT_RED;

    tft->fillCircle(DISC_CX, DISC_CY, R_HUB, phaseCol);
}

void GeniusGame::drawPad(int padIndex, bool lit) {
    if (padIndex < 0 || padIndex > 3) return;

    uint16_t color = lit ? PAD_LIT_COLOR[padIndex] : PAD_DIM_COLOR[padIndex];
    int rInnerSq = R_PAD_INNER * R_PAD_INNER;
    int rOuterSq = R_PAD_OUTER * R_PAD_OUTER;

    if (padIndex == PAD_UP || padIndex == PAD_DOWN) {
        int signY = (padIndex == PAD_UP) ? -1 : 1;

        for (int ay = SPOKE_GAP; ay <= R_PAD_OUTER; ay++) {
            int dy = signY * ay;
            int dySq = ay * ay;

            int xOuter = (int)sqrtf((float)(rOuterSq - dySq));
            int xSpoke = ay - SPOKE_GAP;
            int xHi = (xSpoke < xOuter) ? xSpoke : xOuter;
            if (xHi < 0) continue;

            if (dySq >= rInnerSq) {
                tft->drawFastHLine(DISC_CX - xHi, DISC_CY + dy, 2 * xHi + 1, color);
            } else {
                int xInner = (int)ceilf(sqrtf((float)(rInnerSq - dySq)));
                if (xHi >= xInner) {
                    int segLen = xHi - xInner + 1;
                    tft->drawFastHLine(DISC_CX - xHi,    DISC_CY + dy, segLen, color);
                    tft->drawFastHLine(DISC_CX + xInner, DISC_CY + dy, segLen, color);
                }
            }
        }
    } else {
        int signX = (padIndex == PAD_RIGHT) ? 1 : -1;

        for (int dy = -R_PAD_OUTER; dy <= R_PAD_OUTER; dy++) {
            int ay = (dy < 0) ? -dy : dy;
            int dySq = ay * ay;

            int xHi = (int)sqrtf((float)(rOuterSq - dySq));
            int xLo = ay + SPOKE_GAP;

            if (dySq < rInnerSq) {
                int xInner = (int)ceilf(sqrtf((float)(rInnerSq - dySq)));
                if (xInner > xLo) {
                    xLo = xInner;
                }
            }

            if (xLo <= xHi) {
                int segLen = xHi - xLo + 1;
                if (signX > 0) {
                    tft->drawFastHLine(DISC_CX + xLo, DISC_CY + dy, segLen, color);
                } else {
                    tft->drawFastHLine(DISC_CX - xHi, DISC_CY + dy, segLen, color);
                }
            }
        }
    }
}

void GeniusGame::drawPlaying() {
    if (fullRedrawRequired) {
        tft->fillScreen(TFT_BLACK);
    }

    if (fullRedrawRequired || score != prevScore) {
        drawHUD();
        prevScore = score;
    }

    int phaseCode = getPhaseVisualCode();
    if (fullRedrawRequired || phaseCode != prevPhaseCode) {
        drawCenterPhaseHub(phaseCode);
        prevPhaseCode = phaseCode;
    }

    bool wantLit[4] = {false, false, false, false};
    if (phase == PHASE_MISTAKE) {
        bool blinkOn = (((gameMillis() - phaseStartTime) / 130) % 2 == 0);
        int expectedPad = sequence[inputIndex];
        if (expectedPad >= 0 && expectedPad < 4) {
            wantLit[expectedPad] = blinkOn;
        }
    } else if (activePad >= 0 && activePad < 4) {
        wantLit[activePad] = true;
    }

    for (int i = 0; i < 4; i++) {
        if (fullRedrawRequired || wantLit[i] != renderedPadLit[i]) {
            drawPad(i, wantLit[i]);
            renderedPadLit[i] = wantLit[i];
        }
    }

    fullRedrawRequired = false;
}
