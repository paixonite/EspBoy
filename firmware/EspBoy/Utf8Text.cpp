#include "Utf8Text.h"

// Latin-1 uppercase block U+00C0..U+00DC. Lowercase (U+00E0..U+00FC) uses the same
// table at (cp - 0x20) with a lowercase base. base == 0 means "no glyph available".
struct LatinEntry { char base; Accent accent; };

static const LatinEntry LATIN_TABLE[0xDC - 0xC0 + 1] = {
    {'A', ACCENT_GRAVE},      // C0 À
    {'A', ACCENT_ACUTE},      // C1 Á
    {'A', ACCENT_CIRCUMFLEX}, // C2 Â
    {'A', ACCENT_TILDE},      // C3 Ã
    {'A', ACCENT_DIAERESIS},  // C4 Ä
    {0,   ACCENT_NONE},       // C5 Å
    {0,   ACCENT_NONE},       // C6 Æ
    {'C', ACCENT_CEDILLA},    // C7 Ç
    {'E', ACCENT_GRAVE},      // C8 È
    {'E', ACCENT_ACUTE},      // C9 É
    {'E', ACCENT_CIRCUMFLEX}, // CA Ê
    {'E', ACCENT_DIAERESIS},  // CB Ë
    {'I', ACCENT_GRAVE},      // CC Ì
    {'I', ACCENT_ACUTE},      // CD Í
    {'I', ACCENT_CIRCUMFLEX}, // CE Î
    {'I', ACCENT_DIAERESIS},  // CF Ï
    {0,   ACCENT_NONE},       // D0 Ð
    {'N', ACCENT_TILDE},      // D1 Ñ
    {'O', ACCENT_GRAVE},      // D2 Ò
    {'O', ACCENT_ACUTE},      // D3 Ó
    {'O', ACCENT_CIRCUMFLEX}, // D4 Ô
    {'O', ACCENT_TILDE},      // D5 Õ
    {'O', ACCENT_DIAERESIS},  // D6 Ö
    {0,   ACCENT_NONE},       // D7 ×
    {0,   ACCENT_NONE},       // D8 Ø
    {'U', ACCENT_GRAVE},      // D9 Ù
    {'U', ACCENT_ACUTE},      // DA Ú
    {'U', ACCENT_CIRCUMFLEX}, // DB Û
    {'U', ACCENT_DIAERESIS}   // DC Ü
};

bool utf8NextGlyph(const char*& p, TextGlyph& out) {
    uint8_t c = (uint8_t)*p;
    if (c == 0) return false;
    p++;

    out.accent = ACCENT_NONE;
    out.base = '?';

    if (c < 0x80) {                    // Plain ASCII
        out.base = (char)c;
        return true;
    }

    if ((c & 0xE0) == 0xC0 && ((uint8_t)*p & 0xC0) == 0x80) {   // 2-byte sequence
        uint16_t cp = ((c & 0x1F) << 6) | ((uint8_t)*p & 0x3F);
        p++;

        bool lower = (cp >= 0xE0);
        uint16_t upperCp = lower ? (cp - 0x20) : cp;
        if (upperCp >= 0xC0 && upperCp <= 0xDC) {
            const LatinEntry& e = LATIN_TABLE[upperCp - 0xC0];
            if (e.base != 0) {
                out.base = lower ? (char)(e.base + ('a' - 'A')) : e.base;
                out.accent = e.accent;
            }
        }
        return true;
    }

    // Any other multi-byte sequence: swallow its continuation bytes and show '?'
    while (((uint8_t)*p & 0xC0) == 0x80) p++;
    return true;
}

int utf8GlyphCount(const char* s) {
    TextGlyph g;
    int n = 0;
    while (utf8NextGlyph(s, g)) n++;
    return n;
}

int utf8Strip(const char* s, char* out, size_t cap) {
    if (cap == 0) return 0;
    TextGlyph g;
    size_t n = 0;
    while (n + 1 < cap && utf8NextGlyph(s, g)) out[n++] = g.base;
    out[n] = '\0';
    return (int)n;
}
