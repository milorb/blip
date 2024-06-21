#ifndef FONTS_HPP
#define FONTS_HPP

#include <string>
#include <unordered_map>

#include "SDL_ttf.h"

#include "utilities.hpp"

#define FONT_SIZE 40
#define FONT_0 "Sansation_Regular.TTF"


class Fonts {
public:
    Fonts();
    ~Fonts();

    std::unordered_map<std::string, TTF_Font*> fonts;
};

#endif