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
// EspBoy sound library, organized by context.
// =================================================================

// -----------------------------------------------------------------
// 3.1 System Sounds
// -----------------------------------------------------------------

// Melody for the Splash Screen ("EspBoy Fanfare")
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

// Fanfare played when the player sets a high score (Ascending arpeggio)
const Note MELODY_NEW_HIGHSCORE[] = {
    {NOTE_C5, 100},
    {NOTE_G5, 100},
    {NOTE_C6, 400}  
};
const int MELODY_NEW_HIGHSCORE_LENGTH = sizeof(MELODY_NEW_HIGHSCORE) / sizeof(Note);

// -----------------------------------------------------------------
// 3.2 Snake Game Sounds
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


// Sound effect for collision / game over.
const Note MELODY_HIT[] = {
    {NOTE_A3, 100},
    {REST, 50},
    {NOTE_F4, 200}
};
const int MELODY_HIT_LENGTH = sizeof(MELODY_HIT) / sizeof(Note);


#endif // AUDIO_ASSETS_H