#ifndef FLAPPYBIRDGAME_H
#define FLAPPYBIRDGAME_H

#include "Game.h"

class FlappyBirdGame : public Game {
public:
    FlappyBirdGame(TFT_eSPI* tft_display, SoundManager* sound_manager);

protected:
    // Only mandatory base class implementations
    void resetGame() override;
    void updatePlaying() override;
    void drawPlaying() override;
    void handleInputPlaying() override;
    void onResume() override;

private:
    struct Pipe {
        float x;
        float x_prev; 
        int gap_y;
        bool scored;
    };

    void checkCollisions();
    void drawHUD();

    // Flappy Bird-specific properties
    float player_y;
    float player_y_prev;
    float player_vel_y;
    Pipe pipes[3];
    bool fullRedrawRequired;
};

#endif // FLAPPYBIRDGAME_H