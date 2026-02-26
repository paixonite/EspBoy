#ifndef AUDIO_ASSETS_H
#define AUDIO_ASSETS_H

// =================================================================
// SEÇÃO 1: DEFINIÇÕES DAS NOTAS MUSICAIS
// Tabela de referência com as frequências (em Hz) de cada nota.
// =================================================================

#define REST      0    // Define uma pausa (silêncio)

// Oitavas
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
// SEÇÃO 2: ESTRUTURA DE DADOS PARA UMA NOTA
// Define um modelo para criar melodias, onde cada nota tem uma
// frequência (o tom) e uma duração (o tempo que ela toca).
// =================================================================

struct Note {
    int frequency; // Frequência em Hz (ex: NOTE_C4)
    int duration;  // Duração em milissegundos
};


// =================================================================
// SEÇÃO 3: MELODIAS E EFEITOS SONOROS (SFX)
// Biblioteca de sons do EspBoy, separados por contexto.
// =================================================================

// -----------------------------------------------------------------
// 3.1 Sons do Sistema
// -----------------------------------------------------------------

// Melodia para a Splash Screen ("EspBoy Fanfare")
// Um arpejo ascendente de Lá Maior, que soa "brilhante" e "positivo".
const Note MELODY_SPLASH[] = {
    {NOTE_A3, 150},
    {NOTE_E4, 150},
    {NOTE_A4, 150},
    {NOTE_CS5, 600}
};
const int MELODY_SPLASH_LENGTH = sizeof(MELODY_SPLASH) / sizeof(Note);


// Efeito sonoro para navegação no menu principal.
const Note MELODY_MENU_NAVIGATE[] = {
    {NOTE_A5, 50}
};
const int MELODY_MENU_NAVIGATE_LENGTH = sizeof(MELODY_MENU_NAVIGATE) / sizeof(Note);


// -----------------------------------------------------------------
// 3.2 Sons do Jogo Snake
// -----------------------------------------------------------------

// Efeito sonoro para quando a cobra come uma maçã.
const Note MELODY_APPLE[] = {
    {NOTE_C5, 100},
    {NOTE_G5, 100}
};
const int MELODY_APPLE_LENGTH = sizeof(MELODY_APPLE) / sizeof(Note);


// Melodia para a tela de Game Over do Snake.
const Note MELODY_GAME_OVER[] = {
    {NOTE_G4, 250},
    {NOTE_F4, 250},
    {NOTE_E4, 250},
    {NOTE_D4, 500}
};
const int MELODY_GAME_OVER_LENGTH = sizeof(MELODY_GAME_OVER) / sizeof(Note);


// -----------------------------------------------------------------
// 3.3 Sons do Jogo Flappy Bird
// -----------------------------------------------------------------

// Efeito sonoro para o "flap" (pulo do pássaro).
const Note MELODY_FLAP[] = {
    {NOTE_A4, 50}
};
const int MELODY_FLAP_LENGTH = sizeof(MELODY_FLAP) / sizeof(Note);


// Efeito sonoro para marcar ponto ao passar por um cano.
const Note MELODY_SCORE[] = {
    {NOTE_C6, 80}
};
const int MELODY_SCORE_LENGTH = sizeof(MELODY_SCORE) / sizeof(Note);


// Efeito sonoro para a colisão/game over.
const Note MELODY_HIT[] = {
    {NOTE_A3, 100},
    {REST, 50},
    {NOTE_F4, 200}
};
const int MELODY_HIT_LENGTH = sizeof(MELODY_HIT) / sizeof(Note);


#endif // AUDIO_ASSETS_H