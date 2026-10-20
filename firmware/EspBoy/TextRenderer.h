#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <TFT_eSPI.h>

// Draws UTF-8 text with the built-in 5x7 font, painting accent marks (á ã ç Ê ...)
// on top of the base letter. Honours the TFT's current datum, size and color, so it is a
// drop-in replacement for tft->drawString() for any user-visible (translated) text.
namespace TextRenderer {
    void    draw(TFT_eSPI* tft, const char* utf8, int32_t x, int32_t y);
    void    draw(TFT_eSPI* tft, const String& utf8, int32_t x, int32_t y);
    int16_t width(TFT_eSPI* tft, const char* utf8);   // Displayed chars * 6 * textsize
}

#endif // TEXTRENDERER_H
