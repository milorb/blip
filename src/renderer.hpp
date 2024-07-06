#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <memory>
#include <sstream>
#include <unordered_map>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "file.hpp"
#include "fonts.hpp"
#include "states.hpp"
#include "utilities.hpp"

struct GlyphData {
    GlyphData() : 
        texture(nullptr), 
        advance(0), 
        minx(0), 
        maxx(0), 
        miny(0), 
        maxy(0),
        w(0),
        h(0) {}
    SDL_Texture* texture;
    int advance; 
    int minx; 
    int maxx; 
    int miny; 
    int maxy;
    int w;
    int h;
};

class Renderer {
public:
    Renderer();
    ~Renderer();
    void render_clear();
    void render_present();

    void render_string(const std::string& text, int x, int y, SDL_Color &color);
    void render_rectangle(const Vec2 &pos, const Vec2 &dims, SDL_Color &color, bool filled);
    
    void render_file_contents(File& f);
    void render_cursor(Vec2& c, File& f);
    void render_line_select(int row);
    void render_text_selection(Vec2 start, Vec2 end, File &file);

    void render_select_line_text(int start_x, int end_x, int line);

    void render_status_bar(std::string status, Vec2 &cursor_pos);
    void render_command_prompt_bar(std::string status, std::string &command, std::string user_input, Vec2 &cursor_pos);
    
    //getters/setters
    GlyphData get_glyph(char c);
    Vec2 get_window_dims();
    void set_window_dims(Vec2);

     // sdl window dimensions
    int WINDOW_WIDTH = 640;
    int  WINDOW_HEIGHT = 480;
    TTF_Font *f;

private:
    void render_file_character(const Character& character, int row, int col, 
                            int& penx, int peny, File& f);
    void render_file_line(const Line& line, int row, File& file);
    void render_texture(SDL_Texture* texture, const SDL_Rect& rect);// file editor functions 
    void render_char(char c, int x, int y);
    void render_file_line_number(int line);
    void invert_texture_colors(SDL_Texture *t);

    int tab_size;

    // editor window location
    int KERNING_MODIFIER = 1;
    int LEFT_BORDER = 35;
    int TOP_BORDER = 35;
    int LINE_HEIGHT = 22;

    SDL_Color doc_text_color;
    SDL_Color editor_bg_color;
    SDL_Color cursor_color;
    SDL_Color line_number_color;

    std::unordered_map<char, GlyphData> glyph_cache;

    float text_scale_factor = 1.0f;

    SDL_Renderer* renderer;
    SDL_Window* window;
};

#endif