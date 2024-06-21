#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <iostream>
#include <string>
#include <unordered_map>

#include "file.hpp"
#include "renderer.hpp"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MACOS_LEFT_COMMAND        1073742051
#define MACOS_RIGHT_COMMAND       1073742055

/**
 * Handles initializing the SDL window
 * Gathers inputs
 * Rendering endpoint
*/

/**
 * Possible additional states:
 * Edit mode
 * Select mode
 * Command mode
*/
enum E_State {COMMAND, EDIT, EXIT, RESTING};

class Editor {
public:
  Editor();
  ~Editor();

private:
  void run();
  void input();
  void render();
  void render_intro();
  void play_intro();
  
  void process_key_down_event(SDL_Keycode);
  void process_key_up_event(SDL_Keycode);
  void process_command(char command_char);
  void update_state(E_State);
  void reset_state(E_State);
  void cleanup();
  void move_cursor(int x, int y);
  int get_frame_count() {return frame_count;};

  E_State state;
  E_State prev_state;

  int frame_count;
  bool intro;

  Renderer r = Renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
  Vec2 cursor;
  File file;
};

#endif
