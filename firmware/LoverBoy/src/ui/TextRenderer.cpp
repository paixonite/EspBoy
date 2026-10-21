#include "TextRenderer.h"
#include "Utf8Text.h"

// Accent pixel art: 2 rows x 5 columns (bit 4 = leftmost column of the glyph cell).
struct AccentArt { uint8_t rows[2]; };

static const AccentArt ACCENT_ART[] = {
    {{0b00000, 0b00000}},   // ACCENT_NONE
    {{0b00010, 0b00100}},   // ACCENT_ACUTE
    {{0b01000, 0b00100}},   // ACCENT_GRAVE
    {{0b00100, 0b01010}},   // ACCENT_CIRCUMFLEX
    {{0b01101, 0b10110}},   // ACCENT_TILDE
    {{0b01010, 0b00000}},   // ACCENT_DIAERESIS
    {{0b00100, 0b01000}}    // ACCENT_CEDILLA (drawn below the glyph)
};

static const int GLYPH_W = 6;   // Font cell width (5 px glyph + 1 px gap)
static const int GLYPH_H = 8;   // Font cell height

static void drawAccent(TFT_eSPI* tft, int cellX, int cellY, int size, const TextGlyph& g) {
    const AccentArt& art = ACCENT_ART[g.accent];

    // Lowercase: inside the cell, rows 0-1 (above the x-height).
    // Uppercase: above the cell, rows -2..-1. Cedilla: rows 7..8 (below the baseline).
    int firstRow;
    if (g.accent == ACCENT_CEDILLA) firstRow = 7;
    else if (g.base >= 'A' && g.base <= 'Z') firstRow = -2;
    else firstRow = 0;

    for (int r = 0; r < 2; r++) {
        for (int col = 0; col < 5; col++) {
            if (art.rows[r] & (1 << (4 - col))) {
                tft->fillRect(cellX + col * size, cellY + (firstRow + r) * size, size, size, tft->textcolor);
            }
        }
    }
}

void TextRenderer::draw(TFT_eSPI* tft, const char* utf8, int32_t x, int32_t y) {
    char stripped[64];
    int count = utf8Strip(utf8, stripped, sizeof(stripped));

    // 1. Regular text pass: datum, size and color behave exactly like before
    tft->drawString(stripped, x, y);

    // 2. Resolve the top-left corner of the string the same way TFT_eSPI::drawString does
    int size = tft->textsize;
    int w = count * GLYPH_W * size;
    int h = GLYPH_H * size;
    uint8_t datum = tft->getTextDatum();
    int column = datum % 3;   // 0 = left, 1 = center, 2 = right
    int row = (datum < 9) ? datum / 3 : 0;   // 0 = top, 1 = middle, 2 = bottom

    int originX = x;
    int originY = y;
    if (column == 1) originX -= w / 2;
    else if (column == 2) originX -= w;
    if (row == 1) originY -= h / 2;
    else if (row == 2) originY -= h;

    // 3. Accent pass
    const char* p = utf8;
    TextGlyph g;
    int index = 0;
    while (index < count && utf8NextGlyph(p, g)) {
        if (g.accent != ACCENT_NONE) {
            drawAccent(tft, originX + index * GLYPH_W * size, originY, size, g);
        }
        index++;
    }
}

void TextRenderer::draw(TFT_eSPI* tft, const String& utf8, int32_t x, int32_t y) {
    draw(tft, utf8.c_str(), x, y);
}

int16_t TextRenderer::width(TFT_eSPI* tft, const char* utf8) {
    return utf8GlyphCount(utf8) * GLYPH_W * tft->textsize;
}
