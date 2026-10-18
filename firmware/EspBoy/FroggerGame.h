#ifndef FROGGERGAME_H
#define FROGGERGAME_H

#include "Game.h"

enum TurtleDiveState {
    TURTLE_SURFACED,
    TURTLE_WARNING,
    TURTLE_SUBMERGED
};

class FroggerGame : public Game {
public:
    FroggerGame(TFT_eSPI* tft_display, SoundManager* sound_manager);

protected:
    void resetGame() override;
    void updatePlaying() override;
    void drawPlaying() override;
    void handleInputPlaying() override;

private:
    static constexpr int DISPLAY_WIDTH = 320;
    static constexpr int DISPLAY_HEIGHT = 170;
    static constexpr int HUD_HEIGHT = 14;
    static constexpr int ROW_HEIGHT = 12;
    static constexpr int NUM_ROWS = 13;
    static constexpr int ENTITY_HEIGHT = 10;
    static constexpr int FROG_SIZE = 10;
    static constexpr int NUM_BAYS = 5;
    static constexpr int BAY_WIDTH = 28;
    static constexpr int NUM_ACTIVE_LANES = 10;
    static constexpr int MAX_OBSTACLES_PER_LANE = 3;

    struct Obstacle {
        float x;
        float x_prev;
        bool isDivingGroup;
        TurtleDiveState diveState;
        TurtleDiveState prevDiveState;
        uint16_t prevColor;
    };

    struct Lane {
        int row;                // Playfield row index (1..5 river, 7..11 road)
        bool isRiver;           // true = river platform, false = road hazard
        bool isTurtle;          // true = turtle group, false = log or vehicle
        float baseSpeed;        // Signed speed in px/frame (+ right, - left)
        int width;              // Entity width in pixels
        int count;              // Number of entities in this lane (2 or 3)
        uint16_t color;         // Base RGB565 color of the entity
        Obstacle obstacles[MAX_OBSTACLES_PER_LANE];
    };

    // Coordinate & color helpers
    int rowToY(int row) const;
    uint16_t getRowBgColor(int row) const;
    uint16_t getObstacleColor(const Lane& lane, const Obstacle& obs) const;
    float getSpeedMultiplier() const;

    // Rendering helpers
    void drawStaticBackground();
    void drawGoalRow();
    void drawHUD();
    void drawSpanClippedAroundFrog(int y, int spanL, int spanR, int frogL, int frogR, bool clipFrog, uint16_t color);

    // Logic helpers
    void initLanes();
    void updateTurtleDiveStates();
    void checkCollisionsAndRiver();
    void respawnFrog();

    // Frog state
    float player_x;
    float player_x_prev;
    int player_row;
    int player_row_prev;
    int prev_score;
    bool fullRedrawRequired;

    // Input edge-detection states
    bool lastUp;
    bool lastDown;
    bool lastLeft;
    bool lastRight;
    bool lastA;

    // Active lanes (5 river + 5 road)
    Lane lanes[NUM_ACTIVE_LANES];
};

#endif // FROGGERGAME_H
