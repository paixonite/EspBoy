#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <Arduino.h>
#include "i18n_strings.h"

// Supported languages. NEVER reorder: the value is persisted in NVS.
enum Language : uint8_t { LANG_EN = 0, LANG_PT_BR = 1, LANG_COUNT };

// One enum entry per row of i18n_strings.h
enum StrId : uint16_t {
#define X(id, maxChars, en, pt) id,
    I18N_STRING_LIST(X)
#undef X
    STR_COUNT
};

class I18n {
public:
    static void        setLanguage(Language lang);   // RAM only; ConfigManager persists it
    static Language    getLanguage();
    static const char* tr(StrId id);                 // UTF-8, never nullptr
    static String      fmt(StrId id, ...);           // printf-style, e.g. fmt(STR_SCORE_FMT, score)
    static const char* languageName(Language lang);  // Always written in its own language
};

// Short alias for call sites
inline const char* tr(StrId id) { return I18n::tr(id); }

#endif // LOCALIZATION_H
