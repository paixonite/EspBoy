#ifndef AUDIO_ASSETS_H
#define AUDIO_ASSETS_H

// =================================================================
// SECTION 1: MUSICAL NOTE DEFINITIONS
// Reference table with frequencies (in Hz) for each note.
// =================================================================

#define REST      0    // Defines a rest (silence)

// Octaves
#define NOTE_A3   220
#define NOTE_C4   262
#define NOTE_D4   294
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_G4   392
#define NOTE_A4   440
#define NOTE_B4   494
#define NOTE_C5   523
#define NOTE_CS5  554
#define NOTE_D5   587
#define NOTE_E5   659
#define NOTE_F5   698
#define NOTE_G5   784
#define NOTE_A5   880
#define NOTE_C6   1047


// =================================================================
// SECTION 2: NOTE DATA STRUCTURE
// Defines a model for creating melodies, where each note has a
// frequency (pitch) and a duration (the time it plays).
// =================================================================

struct Note {
    int frequency; // Frequency in Hz (e.g., NOTE_C4)
    int duration;  // Duration in milliseconds
};


// =================================================================
// SECTION 3: MELODIES AND SOUND EFFECTS (SFX)
// LoverBoy sound library, organized by context.
// =================================================================

// -----------------------------------------------------------------
// 3.1 System Sounds
// -----------------------------------------------------------------

// Melody for the Splash Screen ("LoverBoy Fanfare")
// An ascending A Major arpeggio, sounding "bright" and "positive".
const Note MELODY_SPLASH[] = {
    {NOTE_A3, 150},
    {NOTE_E4, 150},
    {NOTE_A4, 150},
    {NOTE_CS5, 600}
};
const int MELODY_SPLASH_LENGTH = sizeof(MELODY_SPLASH) / sizeof(Note);


// Sound effect for main menu navigation.
const Note MELODY_MENU_NAVIGATE[] = {
    {NOTE_A5, 50}
};
const int MELODY_MENU_NAVIGATE_LENGTH = sizeof(MELODY_MENU_NAVIGATE) / sizeof(Note);

// Pause chime (descending fourth) played when [START] opens the pause popup.
const Note MELODY_PAUSE[] = {
    {NOTE_E5, 60},
    {NOTE_B4, 110}
};
const int MELODY_PAUSE_LENGTH = sizeof(MELODY_PAUSE) / sizeof(Note);

// Resume chime (ascending mirror of the pause chime) played when gameplay continues.
const Note MELODY_RESUME[] = {
    {NOTE_B4, 60},
    {NOTE_E5, 110}
};
const int MELODY_RESUME_LENGTH = sizeof(MELODY_RESUME) / sizeof(Note);

// Fanfare played when the player sets a high score (Ascending arpeggio)
const Note MELODY_NEW_HIGHSCORE[] = {
    {NOTE_C5, 100},
    {NOTE_G5, 100},
    {NOTE_C6, 400}  
};
const int MELODY_NEW_HIGHSCORE_LENGTH = sizeof(MELODY_NEW_HIGHSCORE) / sizeof(Note);

// Sound effect for universal collision / game over.
const Note MELODY_HIT[] = {
    {NOTE_A3, 100},
    {REST, 50},
    {NOTE_F4, 200}
};
const int MELODY_HIT_LENGTH = sizeof(MELODY_HIT) / sizeof(Note);


// -----------------------------------------------------------------
// 3.2 Crossy Road Game Sounds
// -----------------------------------------------------------------

// Crisp, deep tactile blip for directional hops (left/right/back or non-scoring forward).
const Note MELODY_HOP[] = {
    {NOTE_C4, 30}
};
const int MELODY_HOP_LENGTH = sizeof(MELODY_HOP) / sizeof(Note);

// Deep, subtle step tone when advancing to a new forward lane (+1 point).
const Note MELODY_CROSSY_POINT[] = {
    {NOTE_E4, 35}
};
const int MELODY_CROSSY_POINT_LENGTH = sizeof(MELODY_CROSSY_POINT) / sizeof(Note);

// Urgent railroad crossing warning chime before a train streaks across.
const Note MELODY_TRAIN_WARN[] = {
    {NOTE_A5, 60},
    {REST, 40},
    {NOTE_A5, 60}
};
const int MELODY_TRAIN_WARN_LENGTH = sizeof(MELODY_TRAIN_WARN) / sizeof(Note);


// -----------------------------------------------------------------
// 3.3 Flappy Bird Game Sounds
// -----------------------------------------------------------------

// Sound effect for the "flap" (bird jump).
const Note MELODY_FLAP[] = {
    {NOTE_A4, 50}
};
const int MELODY_FLAP_LENGTH = sizeof(MELODY_FLAP) / sizeof(Note);

// Sound effect for scoring when passing through a pipe.
const Note MELODY_SCORE[] = {
    {NOTE_C6, 80}
};
const int MELODY_SCORE_LENGTH = sizeof(MELODY_SCORE) / sizeof(Note);


// -----------------------------------------------------------------
// 3.4 Frogger Game Sounds
// -----------------------------------------------------------------

// Crisp two-note upward chirp when the frog hops.
const Note MELODY_FROGGER_HOP[] = {
    {NOTE_E4, 25},
    {NOTE_A4, 35}
};
const int MELODY_FROGGER_HOP_LENGTH = sizeof(MELODY_FROGGER_HOP) / sizeof(Note);

// Bright ascending arpeggio when reaching a lilypad goal bay.
const Note MELODY_FROGGER_GOAL[] = {
    {NOTE_E4, 55},
    {NOTE_G4, 55},
    {NOTE_C5, 55},
    {NOTE_G5, 130}
};
const int MELODY_FROGGER_GOAL_LENGTH = sizeof(MELODY_FROGGER_GOAL) / sizeof(Note);


// -----------------------------------------------------------------
// 3.5 Snake Game Sounds
// -----------------------------------------------------------------

// Sound effect for when the snake eats an apple.
const Note MELODY_APPLE[] = {
    {NOTE_C5, 100},
    {NOTE_G5, 100}
};
const int MELODY_APPLE_LENGTH = sizeof(MELODY_APPLE) / sizeof(Note);

// Melody for the Snake Game Over screen.
const Note MELODY_GAME_OVER[] = {
    {NOTE_G4, 250},
    {NOTE_F4, 250},
    {NOTE_E4, 250},
    {NOTE_D4, 500}
};
const int MELODY_GAME_OVER_LENGTH = sizeof(MELODY_GAME_OVER) / sizeof(Note);


// -----------------------------------------------------------------
// 3.6 Tetris Game Sounds
// -----------------------------------------------------------------

// Crisp upward blip when rotating a tetromino.
const Note MELODY_TETRIS_ROTATE[] = {
    {NOTE_E5, 22},
    {NOTE_G5, 28}
};
const int MELODY_TETRIS_ROTATE_LENGTH = sizeof(MELODY_TETRIS_ROTATE) / sizeof(Note);

// Punchy low thud when a piece locks or hard-drops into place.
const Note MELODY_TETRIS_DROP[] = {
    {NOTE_E4, 25},
    {NOTE_A3, 40}
};
const int MELODY_TETRIS_DROP_LENGTH = sizeof(MELODY_TETRIS_DROP) / sizeof(Note);

// Quick two-note swap chirp when holding a piece.
const Note MELODY_TETRIS_HOLD[] = {
    {NOTE_A4, 25},
    {NOTE_E5, 35}
};
const int MELODY_TETRIS_HOLD_LENGTH = sizeof(MELODY_TETRIS_HOLD) / sizeof(Note);

// Bright ascending chime for clearing 1 to 3 lines.
const Note MELODY_TETRIS_LINE[] = {
    {NOTE_C5, 50},
    {NOTE_E5, 50},
    {NOTE_G5, 100}
};
const int MELODY_TETRIS_LINE_LENGTH = sizeof(MELODY_TETRIS_LINE) / sizeof(Note);

// Celebratory arcade fanfare for clearing 4 lines at once (TETRIS!).
const Note MELODY_TETRIS_FOUR[] = {
    {NOTE_E5, 45},
    {NOTE_G5, 45},
    {NOTE_A5, 45},
    {NOTE_C6, 65},
    {NOTE_A5, 45},
    {NOTE_C6, 150}
};
const int MELODY_TETRIS_FOUR_LENGTH = sizeof(MELODY_TETRIS_FOUR) / sizeof(Note);


// -----------------------------------------------------------------
// 3.7 Genius Game Sounds
// -----------------------------------------------------------------

// Classic harmonic A-major triad tones for the 4 colored Genius pads.
const Note MELODY_GENIUS_GREEN[] = {
    {NOTE_E5, 190}
};
const int MELODY_GENIUS_GREEN_LENGTH = sizeof(MELODY_GENIUS_GREEN) / sizeof(Note);

const Note MELODY_GENIUS_RED[] = {
    {NOTE_A4, 190}
};
const int MELODY_GENIUS_RED_LENGTH = sizeof(MELODY_GENIUS_RED) / sizeof(Note);

const Note MELODY_GENIUS_YELLOW[] = {
    {NOTE_CS5, 190}
};
const int MELODY_GENIUS_YELLOW_LENGTH = sizeof(MELODY_GENIUS_YELLOW) / sizeof(Note);

const Note MELODY_GENIUS_BLUE[] = {
    {NOTE_E4, 190}
};
const int MELODY_GENIUS_BLUE_LENGTH = sizeof(MELODY_GENIUS_BLUE) / sizeof(Note);

// Uplifting arpeggio played upon completing a full sequence round.
const Note MELODY_GENIUS_SUCCESS[] = {
    {NOTE_A4, 50},
    {NOTE_CS5, 50},
    {NOTE_E5, 50},
    {NOTE_A5, 130}
};
const int MELODY_GENIUS_SUCCESS_LENGTH = sizeof(MELODY_GENIUS_SUCCESS) / sizeof(Note);

// Low dissonant buzz when the player presses the wrong pad.
const Note MELODY_GENIUS_ERROR[] = {
    {NOTE_D4, 160},
    {NOTE_A3, 360}
};
const int MELODY_GENIUS_ERROR_LENGTH = sizeof(MELODY_GENIUS_ERROR) / sizeof(Note);

#endif // AUDIO_ASSETS_H