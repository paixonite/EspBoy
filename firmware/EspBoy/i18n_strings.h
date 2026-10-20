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
  X(STR_PLAY,              15, "[A] Play",                      "[A] Jogar") \
  X(STR_HINT_TITLE,        50, "[SELECT] Highscores    [B] Back", "[SELECT] Recordes    [B] Voltar") \
  X(STR_GAME_OVER,         15, "GAME OVER",                     "FIM DE JOGO") \
  X(STR_SCORE_FMT,         13, "Score: %d",                     "Pontos: %d") \
  X(STR_HIGHSCORES_TITLE,  24, "HIGHSCORES",                    "RECORDES") \
  X(STR_NEW_TOP_SCORE,     24, "NEW TOP SCORE!",                "NOVO RECORDE!") \
  X(STR_HINT_CONFIRM,      50, "[A] Confirm",                   "[A] Confirmar") \
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
