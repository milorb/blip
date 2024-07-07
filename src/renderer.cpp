#include "renderer.hpp"

Renderer::Renderer() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) Utilities::sdl_error();

  window = SDL_CreateWindow("te", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  if (!window) Utilities::sdl_error();

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) Utilities::sdl_error();

  // ***set font here***
  // will add range of fonts later (system vs. document)
  if (TTF_Init() != 0) Utilities::sdl_error();
  f = TTF_OpenFont("../resources/Rubik-Regular.TTF", 18);
  if (!f) Utilities::sdl_error();

  // number of spaces to render a tab as
  tab_size = 4;

  doc_text_color = {235, 235, 235, 255};
  editor_bg_color = {49, 45, 55, 255};
  cursor_color = {0, 225, 255, 255};
  line_number_color = {255, 255, 255, 100};

  cursor_position = Vec2(LEFT_BORDER, TOP_BORDER);

  SDL_SetRenderDrawColor(renderer, editor_bg_color.r, editor_bg_color.g,
                         editor_bg_color.b, editor_bg_color.a);
  SDL_RenderClear(renderer);
}

/*
 * primarily used to determine the camera offset 
 *   when the cursor moves out of the editor pixel space
 */
void Renderer::update() {
  if (cursor_position.x + 14 > WINDOW_WIDTH + camera_offset.x) {
    camera_offset.x += 10;
  } else if (cursor_position.x < LEFT_BORDER + camera_offset.x) {
    camera_offset.x -= 10;
  }
  if (cursor_position.y + (LINE_HEIGHT * 2) > WINDOW_HEIGHT + camera_offset.y) {
    camera_offset.y += LINE_HEIGHT;
  } else if (cursor_position.y < TOP_BORDER + camera_offset.y) {
    camera_offset.y -= LINE_HEIGHT;
  }
}

void Renderer::render_clear() {
  SDL_SetRenderDrawColor(renderer, 35, 35, 32, 255);
  SDL_RenderClear(renderer);
}

void Renderer::render_rectangle(const Vec2& pos, const Vec2& dims,
                                SDL_Color& color, bool filled) {
  SDL_Rect rect = {pos.x, pos.y, dims.x, dims.y};
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  if (filled) {
    SDL_RenderFillRect(renderer, &rect);
  } else {
    SDL_RenderDrawRect(renderer, &rect);
  }
}

/*
 * render the status bar which presents some information
 *   such as: cursor line and column, editor state,
 *   and prompt info from commands
 */
void Renderer::render_status_bar(std::string status, Vec2& cursor_pos) {
  int bar_height = LINE_HEIGHT + 1;
  Vec2 pos(0, WINDOW_HEIGHT - bar_height);
  Vec2 dims(WINDOW_WIDTH, bar_height);

  SDL_Color color = {55, 55, 55, 255};
  render_rectangle(pos, dims, color, true);
  if (status == "exit") {
    status += "? y/n";
  }
  render_string("status: " + status, 5, (WINDOW_HEIGHT - bar_height) - 1,
                line_number_color);

  std::string s_pos = "L";
  s_pos.append(std::to_string(cursor_pos.y));
  s_pos += "|C";
  s_pos.append(std::to_string(cursor_pos.x));
  render_string(s_pos, WINDOW_WIDTH - 98, (WINDOW_HEIGHT - bar_height) - 1,
                line_number_color);
}


/*
 * prompt and user input rendering
 */
void Renderer::render_command_prompt_bar(std::string status,
                                         std::string& command,
                                         std::string user_input,
                                         Vec2& cursor_pos) {
  int bar_height = LINE_HEIGHT + 1;
  Vec2 pos(0, WINDOW_HEIGHT - bar_height);
  Vec2 dims(WINDOW_WIDTH, bar_height);
  SDL_Color color = {55, 55, 55, 255};
  render_rectangle(pos, dims, color, true);
  if (command == "open") {
    status += ": open";
    std::string input_display = "file: ";
    input_display += user_input;
    render_string(input_display, WINDOW_WIDTH / 2,
                  (WINDOW_HEIGHT - bar_height) - 1, doc_text_color);
  } else if (command == "save") {
    status += ": save";
    std::string input_display = "file: ";
    input_display += user_input;
    render_string(input_display, WINDOW_WIDTH / 2,
                  (WINDOW_HEIGHT - bar_height) - 1, doc_text_color);
  }
  render_string("status: " + status, 5, (WINDOW_HEIGHT - bar_height) - 1,
                line_number_color);
}

void Renderer::render_string(const std::string& text, int x, int y,
                             SDL_Color& color) {
  SDL_Surface* surface = TTF_RenderText_Blended(f, text.c_str(), color);
  if (!surface) Utilities::sdl_error();

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) Utilities::sdl_error();

  SDL_Rect rect = {x, y, (int)(surface->w * text_scale_factor),
                   (int)(surface->h * text_scale_factor)};

  SDL_RenderCopy(renderer, texture, nullptr, &rect);
  SDL_DestroyTexture(texture);

  SDL_FreeSurface(surface);
}

void Renderer::render_texture(SDL_Texture* texture, const SDL_Rect& rect) {
  SDL_RenderCopy(renderer, texture, nullptr, &rect);
}


/*
 * get the GlyphData from the cache or add to cache if necessary
 * this is mainly to quickly acces character textures
 */
GlyphData Renderer::get_glyph(char c) {
  if (glyph_cache.find(c) == glyph_cache.end()) {
    glyph_cache[c] = GlyphData();
    SDL_Surface* surface = TTF_RenderGlyph_Blended(f, c, doc_text_color);
    if (surface) {
      glyph_cache[c].texture = SDL_CreateTextureFromSurface(renderer, surface);
      glyph_cache[c].w = surface->w;
      glyph_cache[c].h = surface->h;
    } else {
      Utilities::sdl_error();
    }
    SDL_FreeSurface(surface);
    if (TTF_GlyphMetrics(f, c, &glyph_cache[c].minx, &glyph_cache[c].maxx,
                         &glyph_cache[c].miny, &glyph_cache[c].maxy,
                         &glyph_cache[c].advance) != 0) {
      Utilities::sdl_error();
    }
    glyph_cache[c].advance += KERNING_MODIFIER;
  }
  return glyph_cache[c];
}

/*
 * main call to render a file
 * only renders lines that fall within the editor file pixel space
 */
void Renderer::render_file_contents(File& file) {
  SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
  SDL_RenderDrawLine(renderer, LEFT_BORDER - 4, 0, LEFT_BORDER - 2,
                     WINDOW_HEIGHT);

  // offset of the starting render idx
  int offset = abs(camera_offset.y / LINE_HEIGHT); // this took way too long to figure out
  // number of lines from offset that should be rendererd
  // technically renders a few extra but I don't mind the wiggle room
  int num_lines =
      std::min((int)file.lines.size() - offset, WINDOW_HEIGHT / LINE_HEIGHT);

  for (int i = 0; i < num_lines; ++i) {
    Line l = file.lines[i + offset];
    render_file_line_number(i + offset);
    render_file_line(l, i + offset, file);
  }
}

/*
 * render a single character of a line in a file
 */
void Renderer::render_file_character(const Character& character, int row,
                                     int col, int& penx, int peny, File& file) {
  GlyphData gd;

  if (character.t == CHAR) {
    gd = get_glyph(character.c);
  } else {
    // if the character is a tab it will have a larger advance
    gd = get_glyph(' ');
    gd.advance += KERNING_MODIFIER * 6;
  }

  if (character.t != TAB) {
    SDL_Rect rect = {penx - camera_offset.x, peny - camera_offset.y,
                     (int)(gd.w * text_scale_factor),
                     (int)(gd.h * text_scale_factor)};
    SDL_RenderCopy(renderer, gd.texture, nullptr, &rect);
    penx += gd.advance;
  } else {
    SDL_Rect rect = {penx - camera_offset.x, peny - camera_offset.y,
                     (int)(gd.w * text_scale_factor),
                     (int)(gd.h * text_scale_factor)};
    for (int i = 0; i < tab_size; ++i) {
      SDL_RenderCopy(renderer, gd.texture, nullptr, &rect);
      penx += gd.advance;
    }
  }

  if (character.t != TAB) {
    file.lines[row].cs[col].start_pixel = Vec2(penx - gd.advance, peny);
  } else {
    file.lines[row].cs[col].start_pixel =
        Vec2(penx - (gd.advance * tab_size), peny);
  }
  file.lines[row].cs[col].end_pixel =
      Vec2((penx) - (KERNING_MODIFIER / 2), peny);
}

/* 
 * render the characters in a line
 * this currently renders off screen
 * some motivation to keep my lines short
 */
void Renderer::render_file_line(const Line& line, int row, File& file) {
  int pen_x = LEFT_BORDER;
  int pen_y = TOP_BORDER + (row * LINE_HEIGHT);
  int col = 0;
  for (Character c : line.cs) {
    render_file_character(c, row, col, pen_x, pen_y, file);
    ++col;
  }
}

/*
 * renders the line numbers on the left side of the editor window
 */
void Renderer::render_file_line_number(int i) {
  std::string line = std::to_string(i);
  int w, h;

  TTF_SizeText(f, line.c_str(), &w, &h);

  SDL_Surface* surface =
      TTF_RenderText_Blended(f, line.c_str(), line_number_color);
  if (!surface) Utilities::sdl_error();

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) Utilities::sdl_error();

  SDL_Rect rect = {LEFT_BORDER - 8 - (int)(w * text_scale_factor),
                   TOP_BORDER + (i * LINE_HEIGHT) - camera_offset.y,
                   (int)(surface->w * text_scale_factor),
                   (int)(surface->h * text_scale_factor)};

  SDL_RenderCopy(renderer, texture, nullptr, &rect);
  SDL_DestroyTexture(texture);

  SDL_FreeSurface(surface);
}


void Renderer::render_line_select(int row) {
  SDL_SetRenderDrawColor(renderer, 30, 30, 30, 20);
  SDL_Rect rect = {LEFT_BORDER - 4,
                   TOP_BORDER + (row * LINE_HEIGHT) - camera_offset.y,
                   WINDOW_WIDTH - LEFT_BORDER + 4, LINE_HEIGHT + 4};
  SDL_RenderFillRect(renderer, &rect);
}

/*
 * render the text selection. this follows a pattern similar to
 *   File::copy() and File::delete_selection()
 *   it first checks the top line
 *   then the middle lines, if any,
 *   and finally the bottome line, if distinct from the top
 */
void Renderer::render_text_selection(Vec2 start, Vec2 end, File& file) {
  Vec2 upper;
  Vec2 lower;
  // need to find the highest idx for top-down rendering
  if (start.y <= end.y) {
    upper = start;
    lower = end;
  } else {
    upper = end;
    lower = start;
  }
  // NOTE: the x_cootds.y value refers to the rightmost pixel of a selection
  // render the top (or only) line select
  Line l = file.lines[upper.y];
  if (!l.cs.empty()) {
    if (upper.y == lower.y) {
      Vec2 x_coords = l.get_subline_pxs(upper.x, lower.x);
      if (lower.x == l.cs.size() && (upper.x != lower.x)) {
        x_coords.y += 5;
      }
      render_select_line_text(x_coords.x, x_coords.y, upper.y);
      return;
    } else {
      Vec2 x_coords = l.get_subline_pxs(upper.x, l.cs.size());
      if (upper.x == l.cs.size()) {
        int x = (l.cs.end() - 1)->end_pixel.x;
        render_select_line_text(x, x + 5, upper.y);
      } else {
        render_select_line_text(x_coords.x, x_coords.y + 5, upper.y);
      }
    }
  } else if (upper.y != lower.y) {
    // render small selection for when the line is empty
    render_select_line_text(LEFT_BORDER, LEFT_BORDER + 5, upper.y);
  }
  for (int i = upper.y + 1; i < lower.y; ++i) {
    l = file.lines[i];
    if (!l.cs.empty()) {
      Vec2 x_coords = l.get_subline_pxs(0, l.cs.size());
      render_select_line_text(x_coords.x, x_coords.y + 5, i);
    } else {
      render_select_line_text(LEFT_BORDER, LEFT_BORDER + 5, i);
    }
  }

  l = file.lines[lower.y];

  if (!l.cs.empty() && upper.y != lower.y) {
    Vec2 x_coords = l.get_subline_pxs(0, lower.x);
    if (lower.x == l.cs.size() && end.y > start.y) {
      x_coords.y += 5;
    }
    render_select_line_text(x_coords.x, x_coords.y, lower.y);
  } else {
    render_select_line_text(LEFT_BORDER, LEFT_BORDER + 5, lower.y);
  }
}

/*
 * renders the text selection of a single given line and range
 */
void Renderer::render_select_line_text(int start_x, int end_x, int line) {
  if (start_x == end_x) {
    return;
  }
  int w;
  SDL_Rect rect;
  if (start_x < end_x) {
    w = end_x - start_x;
    rect = {start_x - camera_offset.x,
            TOP_BORDER + (line * LINE_HEIGHT) - camera_offset.y, w,
            LINE_HEIGHT};
  } else if (start_x > end_x) {
    w = end_x - start_x;
    rect = {end_x - camera_offset.x,
            TOP_BORDER + (line * LINE_HEIGHT) - camera_offset.y, w,
            LINE_HEIGHT};
  }

  SDL_SetRenderDrawColor(renderer, 100, 150, 255, 155);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BlendMode::SDL_BLENDMODE_BLEND);
  SDL_RenderFillRect(renderer, &rect);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BlendMode::SDL_BLENDMODE_NONE);
}

void Renderer::render_cursor(Vec2& c, File& file, int frame) {
  Vec2 dif = Vec2(c.x - cursor_idxs.x, c.y - cursor_idxs.y);

  if (c.x == 0) {
    if (file.lines[c.y].cs.empty()) {
      cursor_position.x = LEFT_BORDER;
    } else {
      cursor_position.x = file.lines[c.y].cs[0].start_pixel.x;
    }
  } else if (c.x == file.lines[c.y].cs.size()) {
    cursor_position.x = file.lines[c.y].cs[c.x - 1].end_pixel.x;
  } else {
    cursor_position.x = file.lines[c.y].cs[c.x].start_pixel.x;
  }

  cursor_position.y += (dif.y * LINE_HEIGHT);
  if (frame % 180 >= 90) {
    SDL_SetRenderDrawColor(renderer, cursor_color.r, cursor_color.g,
                           cursor_color.b, cursor_color.a);
    SDL_Rect r{cursor_position.x - camera_offset.x,
               cursor_position.y - camera_offset.y, 10, LINE_HEIGHT};
    SDL_RenderDrawRect(renderer, &r);
  }

  cursor_idxs.x = c.x;
  cursor_idxs.y = c.y;
}

void Renderer::render_present() { SDL_RenderPresent(renderer); }

Renderer::~Renderer() {
  std::cout << "cleaning up renderer resources..." << std::endl;
  TTF_CloseFont(f);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}