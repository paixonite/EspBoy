#ifndef GENIUSGAME_H
#define GENIUSGAME_H

#include "Game.h"

enum GeniusPhase {
    PHASE_ROUND_INTRO,
    PHASE_SEQ_ON,
    PHASE_SEQ_GAP,
    PHASE_PLAYER_INPUT,
    PHASE_ROUND_SUCCESS,
    PHASE_MISTAKE
};

class GeniusGame : public Game {
public:
    GeniusGame(TFT_eSPI* tft_display, SoundManager* sound_manager, InputManager* input_manager);

protected:
    void resetGame() override;
    void updatePlaying() override;
    void drawPlaying() override;
    void handleInputPlaying() override;
    void onResume() override;

private:
    // Display & HUD Geometry (320 x 170 landscape)
    static const int HUD_HEIGHT   = 20;

    // Simplified Genius Wheel Geometry (centered below HUD bar)
    static const int DISC_CX      = 160;
    static const int DISC_CY      = 95;
    static const int R_PAD_OUTER  = 64;
    static const int R_PAD_INNER  = 22;
    static const int SPOKE_GAP    = 5;
    static const int R_HUB        = 12;

    // Pad Identifiers (Mapped 1:1 to D-Pad directions)
    static const int PAD_UP       = 0; // Green  (North)
    static const int PAD_RIGHT    = 1; // Red    (East)
    static const int PAD_DOWN     = 2; // Yellow (South)
    static const int PAD_LEFT     = 3; // Blue   (West)

    static const int MAX_SEQUENCE = 100;

    // Sequence & Phase State
    uint8_t sequence[MAX_SEQUENCE];
    int roundLength;
    int seqIndex;
    int inputIndex;
    GeniusPhase phase;
    unsigned long phaseStartTime;

    // Active Pad & Player Input State
    int activePad;
    int pendingPressPad;
    unsigned long playerFlashStart;

    // Differential Rendering Tracking (Zero-Flicker SPI)
    bool fullRedrawRequired;
    bool initialStart;
    bool renderedPadLit[4];
    int prevScore;
    int prevPhaseCode;

    // Gameplay Helpers
    void appendRandomStep();
    void playPadTone(int padIndex);
    int getSpeedTier() const;
    unsigned long getStepOnDuration() const;
    unsigned long getStepGapDuration() const;
    bool isPadHeld(int padIndex) const;
    int getPhaseVisualCode() const;

    // Rendering Helpers
    void drawHUD();
    void drawCenterPhaseHub(int phaseCode);
    void drawPad(int padIndex, bool lit);
};

#endif // GENIUSGAME_H
