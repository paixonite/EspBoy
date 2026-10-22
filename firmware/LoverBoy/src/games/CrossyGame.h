#ifndef CROSSYGAME_H
#define CROSSYGAME_H

#include "Game.h"

enum LaneType {
    LANE_GRASS,
    LANE_ROAD,
    LANE_RIVER,
    LANE_RAIL
};

enum RailState {
    RAIL_COOLDOWN,
    RAIL_WARNING,
    RAIL_TRAIN
};

class CrossyGame : public Game {
public:
    CrossyGame(TFT_eSPI* tft_display, SoundManager* sound_manager, InputManager* input_manager);

protected:
    void resetGame() override;
    void updatePlaying() override;
    void drawPlaying() override;
    void handleInputPlaying() override;
    void onResume() override;
    void drawTutorialContent() override;

private:
    // Display & Grid Geometry (320 x 170)
    static const int DISPLAY_WIDTH = 320;
    static const int DISPLAY_HEIGHT = 170;
    static const int HUD_HEIGHT = 20;
    static const int PLAYFIELD_Y = 20;
    static const int NUM_LANES = 10;
    static const int LANE_HEIGHT = 15;
    static const int NUM_COLS = 20;
    static const int COL_WIDTH = 16;

    // Entity Dimensions & Camera
    static const int PLAYER_W = 12;
    static const int PLAYER_H = 11;
    static const int ENTITY_H = 11;
    static const int CAMERA_THRESHOLD_ROW = 4;
    static const int MAX_OBS_PER_LANE = 3;
    static const int TRAIN_WIDTH = 140;

    struct Lane {
        LaneType type;
        float speed;                      // Positive = moving right, Negative = moving left
        int obsCount;                     // Number of moving cars/trucks/logs
        int obsWidth;                     // Pixel width of each moving obstacle/log
        uint16_t obsColor;                // Solid block color for vehicles/logs
        float obsX[MAX_OBS_PER_LANE];     // Current X positions
        float obsXPrev[MAX_OBS_PER_LANE]; // Previous frame X positions for delta erase
        bool trees[NUM_COLS];             // Static tree obstacles for LANE_GRASS

        // Railroad state machine
        RailState railState;
        unsigned long railTimer;
        unsigned long cooldownDuration;
        float trainX;
        float trainXPrev;
        bool trainNeedsErase;
        bool warningLightOn;
        bool warningLightPrev;
    };

    Lane lanes[NUM_LANES];

    // Player state
    int playerRow;
    int playerRowPrev;
    float playerX;
    float playerXPrev;
    int currentWorldRow;
    int maxWorldRow;
    int prevScore;

    // Frame delta flags
    bool fullRedrawRequired;
    bool cameraShiftedThisFrame;
    bool playerHoppedThisFrame;

    // Helper methods
    int laneTopY(int row) const { return PLAYFIELD_Y + row * LANE_HEIGHT; }
    uint16_t getLaneBgColor(LaneType type) const;

    void generateLane(int rowIdx, int worldDist, const Lane* adjacentLane);
    void shiftWorldDown();
    void checkCollisions();

    void drawHUD();
    void drawLaneFull(int rowIdx);
    void drawLaneDelta(int rowIdx);
    void drawClippedRect(int x, int y, int w, int h, uint16_t color);
    void drawRectExcludingPlayer(int x, int y, int w, int h, uint16_t color, int rowIdx);
    void drawPlayerAt(int x, int row, uint16_t color);
};

#endif // CROSSYGAME_H
