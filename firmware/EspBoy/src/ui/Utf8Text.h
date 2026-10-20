#ifndef UTF8TEXT_H
#define UTF8TEXT_H

#include <stdint.h>
#include <stddef.h>

// Pure UTF-8 helpers (no Arduino / TFT dependency, so they can be unit tested on the host).
// The built-in 5x7 font only has ASCII-like glyphs, so accented Latin-1 letters are
// decomposed into a base letter + an accent mark that TextRenderer paints separately.

enum Accent : uint8_t {
    ACCENT_NONE = 0,
    ACCENT_ACUTE,
    ACCENT_GRAVE,
    ACCENT_CIRCUMFLEX,
    ACCENT_TILDE,
    ACCENT_DIAERESIS,
    ACCENT_CEDILLA
};

struct TextGlyph {
    char base;       // ASCII letter to draw with the regular font ('?' if unsupported)
    Accent accent;   // Mark to overlay (ACCENT_NONE for plain characters)
};

// Decodes the next character and advances p. Returns false at the end of the string.
bool utf8NextGlyph(const char*& p, TextGlyph& out);

// Number of displayed characters
int utf8GlyphCount(const char* s);

// Writes the accent-free ASCII version of s into out (same number of characters).
// Returns the character count; output is truncated to cap - 1 characters.
int utf8Strip(const char* s, char* out, size_t cap);

#endif // UTF8TEXT_H
