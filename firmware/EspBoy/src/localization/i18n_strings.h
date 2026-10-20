#ifndef I18N_STRINGS_H
#define I18N_STRINGS_H

// ONE ROW PER ON-SCREEN STRING. Columns: ID, maxChars, English, Português (pt-BR).
// maxChars = most characters that fit where the string is drawn (see budgets in
// architecture.md). Exceeding it fails compilation (see Localization.cpp).
// Forgetting a translation column also fails compilation (macro arity mismatch).
// Accents are written as normal UTF-8; TextRenderer paints them.
// Store text exactly as displayed (e.g. UPPERCASE titles) - never upper-case at runtime.
// printf-style placeholders (%d) count as 2 characters.
// To add a language: add an enum value in Localization.h and one column here.
#define I18N_STRING_LIST(X) \
  /* --- Menu --- */ \
  X(STR_SETTINGS_TITLE,    15, "SETTINGS",                      "CONFIGURAÇÕES") \
  X(STR_CHOOSE_GAME_TITLE, 15, "CHOOSE A GAME",                 "ESCOLHA UM JOGO") \
  X(STR_HINT_MENU,         50, "[SELECT] Settings    [A] Play", "[SELECT] Ajustes    [A] Jogar") \
  X(STR_HINT_BACK,         50, "[B] Back",                      "[B] Voltar") \
  X(STR_SPLASH_BY,         20, "by paixonite",                  "por paixonite") \
  /* --- Settings rows (label + value must fit ~22 chars) --- */ \
  X(STR_SET_LANGUAGE,      10, "Language:",                     "Idioma:") \
  X(STR_SET_VOLUME,        11, "Volume:",                       "Volume:") \
  X(STR_SET_BRIGHTNESS,    11, "Brightness:",                   "Brilho:") \
  X(STR_SET_AUTO_SLEEP,    11, "Auto Sleep:",                   "Suspensão:") \
  X(STR_SET_HIGHSCORES,    11, "Highscores:",                   "Recordes:") \
  X(STR_SET_CREDITS,       11, "Credits:",                      "Créditos:") \
  X(STR_VAL_OFF,           11, "[ OFF ]",                       "[ OFF ]") \
  X(STR_VAL_RESET,         11, "[ RESET ]",                     "[ ZERAR ]") \
  X(STR_VAL_VIEW,          11, "[ VIEW ]",                      "[ VER ]") \
  X(STR_SLEEP_30S,         11, "< 30s >",                       "< 30s >") \
  X(STR_SLEEP_1M,          11, "< 1m >",                        "< 1min >") \
  X(STR_SLEEP_3M,          11, "< 3m >",                        "< 3min >") \
  X(STR_SLEEP_5M,          11, "< 5m >",                        "< 5min >") \
  X(STR_RESET_TITLE,       17, "RESET HIGHSCORES?",             "ZERAR RECORDES?") \
  X(STR_HINT_YES_NO,       30, "[A] Yes    [B] No",             "[A] Sim    [B] Não") \
  /* --- Game framework (Game.cpp) --- */ \
  X(STR_PAUSED,             9, "PAUSED",                        "PAUSADO") \
  X(STR_RESUME,             8, "Resume",                        "Retomar") \
  X(STR_QUIT,               8, "Quit",                          "Sair") \
  X(STR_OPT_PLAY,          11, "Play",                          "Jogar") \
  X(STR_OPT_TUTORIAL,      11, "Tutorial",                      "Tutorial") \
  X(STR_OPT_HIGHSCORES,    11, "Highscores",                    "Recordes") \
  X(STR_HINT_TITLE,        50, "[A] Select    [B] Back",        "[A] Selecionar    [B] Voltar") \
  X(STR_GAME_OVER,         15, "GAME OVER",                     "FIM DE JOGO") \
  X(STR_SCORE_FMT,         13, "Score: %d",                     "Pontos: %d") \
  X(STR_HIGHSCORES_TITLE,  24, "HIGHSCORES",                    "RECORDES") \
  X(STR_NEW_TOP_SCORE,     24, "NEW TOP SCORE!",                "NOVO RECORDE!") \
  X(STR_HINT_CONFIRM,      50, "[A] Confirm",                   "[A] Confirmar") \
  /* --- Tutorial Shared --- */ \
  X(STR_TUTORIAL_TITLE,    15, "TUTORIAL",                      "TUTORIAL") \
  X(STR_TUT_SEC_CONTROLS,  12, "CONTROLS",                      "CONTROLES") \
  X(STR_TUT_SEC_OBJECTIVE, 12, "OBJECTIVE",                     "OBJETIVO") \
  /* --- Crossy Road Tutorial --- */ \
  X(STR_CR_TUT_C1,         30, "D-Pad : Move / Hop",            "D-Pad : Mover / Pular") \
  X(STR_CR_TUT_C2,         30, "[A]   : Hop forward",           "[A]   : Pular para frente") \
  X(STR_CR_TUT_O1,         48, "Cross roads, rivers, and train tracks.", "Atravesse ruas, rios e trilhos de trem.") \
  X(STR_CR_TUT_O2,         48, "Hop on logs to cross rivers. Don't drown!", "Pule nos troncos no rio. Não se afogue!") \
  X(STR_CR_TUT_O3,         48, "Watch for fast cars and red train lights.", "Cuidado com carros e sinais de trem.") \
  X(STR_CR_TUT_O4,         48, "Go as far as you can to score high!", "Avance o mais longe que conseguir!") \
  /* --- Flappy Bird Tutorial --- */ \
  X(STR_FB_TUT_C1,         40, "[A] : Flap wings / Gain altitude", "[A] : Bater asas / Ganhar altura") \
  X(STR_FB_TUT_O1,         48, "Tap [A] to keep the bird in the air.", "Toque [A] para manter o pássaro no ar.") \
  X(STR_FB_TUT_O2,         48, "Fly safely through gaps between pipes.", "Voe pelos vãos entre os canos verdes.") \
  X(STR_FB_TUT_O3,         48, "Avoid touching pipes, ceiling, or ground!", "Não toque nos canos, teto ou chão!") \
  X(STR_FB_TUT_O4,         48, "Each pipe cleared awards 1 point.", "Cada cano superado vale 1 ponto.") \
  /* --- Frogger Tutorial --- */ \
  X(STR_FR_TUT_C1,         35, "D-Pad : Hop in 4 directions",   "D-Pad : Pular em 4 direções") \
  X(STR_FR_TUT_C2,         30, "[A]   : Hop forward",           "[A]   : Pular para frente") \
  X(STR_FR_TUT_O1,         48, "Dodge highway traffic to reach the river.", "Desvie dos carros até chegar ao rio.") \
  X(STR_FR_TUT_O2,         48, "Ride logs and turtles to cross safely.", "Suba em troncos e tartarugas no rio.") \
  X(STR_FR_TUT_O3,         48, "Beware: turtles dive when turning red!", "Atenção: tartarugas vermelhas afundam!") \
  X(STR_FR_TUT_O4,         48, "Fill all 5 home bays to win the round!", "Preencha os 5 abrigos para pontuar!") \
  /* --- Genius Tutorial --- */ \
  X(STR_GN_TUT_C1,         40, "[UP] Green      [RIGHT] Red",   "[UP] Verde      [RIGHT] Vermelho") \
  X(STR_GN_TUT_C2,         40, "[DOWN] Yellow   [LEFT] Blue",   "[DOWN] Amarelo  [LEFT] Azul") \
  X(STR_GN_TUT_O1,         48, "Watch and listen to the color pattern.", "Observe e ouça a sequência de cores.") \
  X(STR_GN_TUT_O2,         48, "Repeat the sequence using the D-Pad.", "Repita a sequência usando o D-Pad.") \
  X(STR_GN_TUT_O3,         48, "Each round adds a new step to remember.", "Cada rodada adiciona mais uma cor.") \
  X(STR_GN_TUT_O4,         48, "One wrong press ends your game!", "Um erro encerra a partida!") \
  /* --- Snake Tutorial --- */ \
  X(STR_SN_TUT_C1,         40, "D-Pad : Change movement direction", "D-Pad : Mudar direção da cobra") \
  X(STR_SN_TUT_O1,         48, "Eat red apples to grow and score points.", "Coma maçãs vermelhas para crescer.") \
  X(STR_SN_TUT_O2,         48, "Each apple increases your body length.", "Cada maçã aumenta o tamanho da cobra.") \
  X(STR_SN_TUT_O3,         48, "Don't crash into the outer walls!", "Não bata nas paredes da arena!") \
  X(STR_SN_TUT_O4,         48, "Avoid biting your own growing tail!", "Evite morder o próprio rabo!") \
  /* --- Tetris Tutorial --- */ \
  X(STR_TT_TUT_C1,         35, "[LEFT / RIGHT] : Move piece",   "[LEFT / RIGHT] : Mover peça") \
  X(STR_TT_TUT_C2,         40, "[A / B]        : Rotate (CW / CCW)", "[A / B]        : Girar horário/anti") \
  X(STR_TT_TUT_C3,         40, "[DOWN] Soft Drop    [UP] Hard Drop", "[DOWN] Descer       [UP] Queda total") \
  X(STR_TT_TUT_C4,         35, "[SELECT]       : Hold piece",   "[SELECT]       : Guardar peça") \
  X(STR_TT_TUT_O1,         48, "Fill horizontal lines to clear them.", "Complete linhas horizontais.") \
  X(STR_TT_TUT_O2,         48, "Clear lines to score and level up.", "Elimine linhas para subir de nível.") \
  X(STR_TT_TUT_O3,         48, "Speed increases with each level!", "A velocidade aumenta a cada nível!") \
  X(STR_TT_TUT_O4,         48, "Don't let pieces stack to the top!", "Não deixe as peças baterem no topo!") \
  /* --- Tetris side panels (PANEL_W = 90 px -> 14 chars) --- */ \
  X(STR_TETRIS_HOLD,       14, "HOLD",                          "RESERVA") \
  X(STR_TETRIS_LEVEL,      14, "LEVEL",                         "NÍVEL") \
  X(STR_TETRIS_SCORE,      14, "SCORE",                         "PONTOS") \
  X(STR_TETRIS_NEXT,       14, "NEXT",                          "PRÓXIMA") \
  X(STR_TETRIS_CONTROLS,   14, "CONTROLS",                      "CONTROLES") \
  X(STR_TETRIS_ROTATE,     14, "[A/B] Rotate",                  "[A/B] Girar") \
  X(STR_TETRIS_HARD_DROP,  14, "[UP] Hard Drop",                "[UP] Queda") \
  X(STR_TETRIS_HOLD_HINT,  14, "[SELECT] Hold",                 "[SELECT] Troca")

#endif // I18N_STRINGS_H
