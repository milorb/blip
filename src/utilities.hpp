#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <iostream>

#include "SDL.h"

#define MACOS_LEFT_COMMAND 1073742051
#define MACOS_RIGHT_COMMAND 1073742055

struct Vec2 {
  Vec2() : x(0), y(0) {};
  Vec2(int a, int b) : x(a), y(b) {};
  Vec2(const Vec2& other) : x(other.x), y(other.y) {};
  void operator=(const Vec2& other) {
    x = other.x;
    y = other.y;
  }
  int x;
  int y;
};

class Utilities {
 public:
  static void sdl_error() {
    std::cout << "error: " << SDL_GetError() << std::endl;
    exit(1);
  }
};

#endif