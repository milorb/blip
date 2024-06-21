#include "renderer.hpp"

float scale_factor = 1.0f;

Renderer::Renderer(int w, int h) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) Utilities::sdl_error();
    
    window = std::make_unique<SDL_Window*>(SDL_CreateWindow(
                        "te", 
                        SDL_WINDOWPOS_CENTERED, 
                        SDL_WINDOWPOS_CENTERED,  
                        w, 
                        h, 
                        SDL_WINDOW_SHOWN));

    if (!window) Utilities::sdl_error();

    renderer = std::make_unique<SDL_Renderer*>(SDL_CreateRenderer(*window, -1, 
                        SDL_RENDERER_ACCELERATED));
    if (!renderer) Utilities::sdl_error();

    if (TTF_Init() != 0) Utilities::sdl_error();
    f = TTF_OpenFont("OpenSans-Medium.TTF", 16);
    if (!f) Utilities::sdl_error();

    SDL_SetRenderDrawColor(*renderer, 35, 35, 32, 255);
    SDL_RenderClear(*renderer);
}


void Renderer::render_clear() {
    SDL_SetRenderDrawColor(*renderer, 35, 35, 32, 255);
    SDL_RenderClear(*renderer);
}

void Renderer::render_text(const std::string& text, int x, int y) {
    
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(f, text.c_str(), color);
    if (!surface) Utilities::sdl_error();

    SDL_Texture* texture = SDL_CreateTextureFromSurface(*renderer, surface);
    if (!texture) Utilities::sdl_error();

    SDL_Rect rect = {x, y, (int)(surface->w * scale_factor), (int)(surface->h * scale_factor)};
    SDL_RenderCopy(*renderer, texture, nullptr, &rect);
    SDL_DestroyTexture(texture);
    
    SDL_FreeSurface(surface);
}

std::vector<std::string> wrap_text(const std::string& text, int x, int y, TTF_Font* f, SDL_Window* window) {
    std::istringstream words(text);
    std::string word;
    std::string cur;
    std::vector<std::string> lines;
    int lw, lh = 0;

    while (words >> word) {
        std::string temp  = cur.empty() ? word : cur + " " + word;
        TTF_SizeText(f, temp.c_str(), &lw, &lh);
        int w, h = 0;
        SDL_GetWindowSize(window, &w, &h);

        if (lw > w && !cur.empty()) {
            lines.push_back(cur);
            cur = word;
        } else {
            cur = temp;
        }
    }

    if (!cur.empty()) {
        lines.push_back(cur);
    }

    return lines;
}

void Renderer::render_file_line_number(int i) {
    std::string line = "";
    line += std::to_string(i);

    int w, h;
    TTF_SizeText(f, line.c_str(), &w,& h);

    SDL_Color color = {255, 255, 255, 100};
    SDL_Surface* surface = TTF_RenderText_Blended(f, line.c_str(), color);
    if (!surface) Utilities::sdl_error();

    SDL_Texture* texture = SDL_CreateTextureFromSurface(*renderer, surface);
    if (!texture) Utilities::sdl_error();

    SDL_Rect rect = {30 - (int)(w * scale_factor), TOP_BORDER + i * 20, 
        (int)(surface->w * scale_factor), (int)(surface->h * scale_factor)};

    SDL_RenderCopy(*renderer, texture, nullptr, &rect);
    SDL_DestroyTexture(texture);
    
    SDL_FreeSurface(surface);
}

void Renderer::render_file_contents(File& f) {
    for (int i = 0; i < f.contents.size(); ++i) {
        render_file_line_number(i);
        if (f.contents[i] != "") {
            render_text(f.contents[i], LEFT_BORDER, TOP_BORDER + i * 20);
        }
    }
}

void Renderer::render_cursor(Vec2& c, File& file) {
    std::string textBeforeCursor = file.contents[c.y].substr(0, c.x);
    int w, h;
    TTF_SizeText(f, textBeforeCursor.c_str(), &w,& h);

    SDL_SetRenderDrawColor(*renderer, 255, 255, 255, 255);
    SDL_Rect r {LEFT_BORDER + (int)(w * scale_factor), TOP_BORDER + c.y * 20, 2, 20};
    SDL_RenderDrawRect(*renderer, &r);
}

void Renderer::render_present() {
    SDL_RenderPresent(*renderer);
}

Renderer::~Renderer() {
    std::cout << "cleaning up renderer resources..." << std::endl;
    TTF_CloseFont(f);
    SDL_DestroyRenderer(*renderer);
    SDL_DestroyWindow(*window);
}