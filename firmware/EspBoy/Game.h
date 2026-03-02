#ifndef GAME_H
#define GAME_H

#include <TFT_eSPI.h>
#include <Preferences.h>
#include "SoundManager.h"
#include "pins.h"

enum GameState { 
    STATE_TITLE, 
    STATE_PLAYING, 
    STATE_GAME_OVER, 
    STATE_HIGHSCORE_VIEW, 
    STATE_HIGHSCORE_ENTRY 
};

class Game {
public:
    Game(TFT_eSPI* tft_display, SoundManager* sound_manager, const String& game_name);
    virtual ~Game() {}

    void setup();
    void loop();
    bool shouldExit() const;

protected:
    TFT_eSPI* tft;
    SoundManager* sound;
    String gameName;
    GameState currentState;
    bool _exit_request;
    int score;
    unsigned long stateTimer; 

    // Arrays para placeholders customizáveis por cada jogo
    int defaultScores[3];
    String defaultNames[3];

    virtual void resetGame() = 0;          
    virtual void updatePlaying() = 0;      
    virtual void drawPlaying() = 0;        
    virtual void handleInputPlaying() = 0; 

    virtual void drawTitleScreen();
    virtual void drawGameOverScreen();
    virtual void handleInputTitle();
    virtual void handleGameOverState();

    void changeState(GameState newState);
    void checkGlobalExit(); 
    
    // --- Lógica de Highscores ---
    void endGame(); 
    void loadHighscores();
    void saveHighscores();
    
    void drawHighscoreViewScreen();
    void drawHighscoreEntryScreen();
    void drawInitials(); 
    
    void handleHighscoreViewInput();
    void handleHighscoreEntryInput();

private:
    Preferences preferences;
    int topScores[3];
    String topNames[3];
    int newHighscoreRank; 
    
    char currentInitials[3];
    int currentInitialIndex;
    
    static char lastInitials[3]; 
};

#endif // GAME_H