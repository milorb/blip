#include "fonts.hpp"

Fonts::Fonts() {
    if (TTF_Init() != 0) Utilities::sdl_error();
    TTF_Font *font = TTF_OpenFont(FONT_0, FONT_SIZE);
    if (!font) Utilities::sdl_error();
    fonts[FONT_0] = font;
}

Fonts::~Fonts() {
    for (auto [name, font] : fonts) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}