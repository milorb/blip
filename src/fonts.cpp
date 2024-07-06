#include "fonts.hpp"

Fonts::Fonts() {
    if (TTF_Init() != 0) Utilities::sdl_error();
}

int Fonts::get_char_advance(char c, TTF_Font* f) {
    int advance;
    int minx, maxx, miny, maxy;
    if (TTF_GlyphMetrics(f, c, &minx, &maxx, &miny, &maxy, &advance) != 0) {
        Utilities::sdl_error();
    }
    return advance;
}

Fonts::~Fonts() {
    TTF_Quit();
}