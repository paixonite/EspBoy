#include "Localization.h"
#include <stdarg.h>

// Counts displayed characters in UTF-8 (continuation bytes 10xxxxxx are skipped).
constexpr int utf8Len(const char* s, int i = 0) {
    return s[i] == 0 ? 0 : (((s[i] & 0xC0) == 0x80 ? 0 : 1) + utf8Len(s, i + 1));
}

// Compile-time guard: every string must fit the slot it is drawn in.
#define X(id, maxChars, en, pt) \
    static_assert(utf8Len(en) <= maxChars, #id ": English text too long for its slot"); \
    static_assert(utf8Len(pt) <= maxChars, #id ": pt-BR text too long for its slot");
I18N_STRING_LIST(X)
#undef X

// Columns are laid out as [language][string id], built from the same table
static const char* const STRING_TABLE[LANG_COUNT][STR_COUNT] = {
    {
#define X(id, maxChars, en, pt) en,
        I18N_STRING_LIST(X)
#undef X
    },
    {
#define X(id, maxChars, en, pt) pt,
        I18N_STRING_LIST(X)
#undef X
    }
};

static const char* const LANGUAGE_NAMES[LANG_COUNT] = { "English", "Português" };

static Language currentLanguage = LANG_EN;

void I18n::setLanguage(Language lang) {
    currentLanguage = (lang < LANG_COUNT) ? lang : LANG_EN;
}

Language I18n::getLanguage() {
    return currentLanguage;
}

const char* I18n::tr(StrId id) {
    if (id >= STR_COUNT) return "";
    return STRING_TABLE[currentLanguage][id];
}

String I18n::fmt(StrId id, ...) {
    char buffer[64];
    va_list args;
    va_start(args, id);
    vsnprintf(buffer, sizeof(buffer), tr(id), args);
    va_end(args);
    return String(buffer);
}

const char* I18n::languageName(Language lang) {
    return LANGUAGE_NAMES[(lang < LANG_COUNT) ? lang : LANG_EN];
}
