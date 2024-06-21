#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <memory>
#include <sstream>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "file.hpp"
#include "utilities.hpp"

#define LEFT_BORDER 35
#define TOP_BORDER 35

class Renderer {
public:
    Renderer(int w, int h);
    ~Renderer();
    void render_clear();
    void render_text(const std::string& text, int x, int y);
    
    void render_file_contents(File& f);
    void render_file_line_number(int line);
    void render_cursor(Vec2& c, File& f);
    void render_present();

    TTF_Font *f;
    int lines_rendered = 0;

    std::unique_ptr<SDL_Renderer*> renderer;
    std::unique_ptr<SDL_Window*> window;
};

#endif