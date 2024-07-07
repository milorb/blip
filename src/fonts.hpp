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

  int get_char_advance(char c, TTF_Font* f);
};

#endif