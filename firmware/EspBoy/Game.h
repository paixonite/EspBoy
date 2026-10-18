#ifndef GAME_H
#define GAME_H

#include <TFT_eSPI.h>
#include <Preferences.h>
#include "SoundManager.h"
#include "InputManager.h"
#include "pins.h"

enum GameState { 
    STATE_TITLE, 
    STATE_PLAYING, 
    STATE_PAUSED,
    STATE_GAME_OVER, 
    STATE_HIGHSCORE_VIEW, 
    STATE_HIGHSCORE_ENTRY 
};

class Game {
public:
    Game(TFT_eSPI* tft_display, SoundManager* sound_manager, InputManager* input_manager, const String& game_name);
    virtual ~Game() {}

    void setup();
    void loop();
    bool shouldExit() const;

protected:
    TFT_eSPI* tft;
    SoundManager* sound;
    InputManager* input;
    String gameName;
    GameState currentState;
    bool _exit_request;
    int score;
    unsigned long stateTimer; 

    // Customizable placeholder arrays for each game
    int defaultScores[3];
    String defaultNames[3];

    virtual void resetGame() = 0;          
    virtual void updatePlaying() = 0;      
    virtual void drawPlaying() = 0;        
    virtual void handleInputPlaying() = 0; 

    // Called when gameplay continues after the pause popup. The popup has painted
    // over part of the playfield, so the game must request a full repaint here
    // (the base class calls drawPlaying() right after this returns).
    virtual void onResume() = 0;

    // Gameplay clock that stands still while paused. Use it instead of millis()
    // for every gameplay timer so nothing "jumps ahead" when the game resumes.
    unsigned long gameMillis() const;

    virtual void drawTitleScreen();
    virtual void drawGameOverScreen();
    virtual void handleInputTitle();
    virtual void handleGameOverState();

    void changeState(GameState newState);
    void checkPauseButton(); 
    
    // --- Pause Popup ---
    void drawPauseScreen();
    void drawPauseOptions();
    void handlePausedInput();
    void resumeFromPause();

    // --- Highscore Logic ---
    void endGame(); 
    void loadHighscores();
    void saveHighscores();
    
    void drawHighscoreViewScreen();
    void drawHighscoreEntryScreen();
    void drawInitials(); 
    
    void handleHighscoreViewInput();
    void handleHighscoreEntryInput();

    // --- UI Helpers ---
    void drawTopBar(const String& title, uint16_t color = TFT_YELLOW);
    void drawBottomBar(const String& text, uint16_t color = TFT_WHITE);
    static const int BAR_HEIGHT = 25;

private:
    Preferences preferences;
    int topScores[3];
    String topNames[3];
    int newHighscoreRank; 
    
    char currentInitials[3];
    int currentInitialIndex;
    
    static char lastInitials[3]; 

    // Pause bookkeeping
    unsigned long pauseStartTime; // millis() when the current pause began
    unsigned long pausedTotalMs;  // Accumulated paused time, subtracted by gameMillis()
    int pauseSelection;           // Highlighted popup option (index into the option list)
};

#endif // GAME_H