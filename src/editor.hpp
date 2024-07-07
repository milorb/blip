#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>

#include "file.hpp"
#include "renderer.hpp"
#include "states.hpp"


/**
 * Handles initializing the SDL window
 * Gathers inputs
 * Rendering endpoint
*/


class Editor {
public:
  Editor();
  ~Editor();

private:
  void run();
  void input();
  void render();
  
  void process_key_down_event(SDL_Keycode);
  void process_key_up_event(SDL_Keycode);
  void process_command(SDL_Keycode);
  void update_state(E_State);
  void reset_state(E_State);
  void cleanup();
  void move_cursor(int x, int y);
  int get_frame_count() {return frame_count;};

  void exit_query();
  void open_query();

  E_State state;
  E_State prev_state;

  int frame_count;

  Vec2 selection_start;
  bool select = false;

  std::tm* time;
  Renderer r;
  Vec2 cursor = Vec2(0, 0);
  File file;
};

#endif
