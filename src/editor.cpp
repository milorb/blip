#include "editor.hpp"

Editor::Editor() {
    frame_count = 0;
    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);
    time = std::localtime(&now_t);
    // start with a new file so that renderer can start properly
    file.new_file();
    SDL_StartTextInput();
    run();
    SDL_StopTextInput();
}

/* used for state management 
 * states are differentiated by what effects a key press will have in it 
 */
void Editor::update_state(E_State state_new) {
    if (state_new != state) {
        prev_state = state;
    }
    state = state_new;
}

void Editor::reset_state(E_State state_new) {
    state = state_new;
    prev_state = state_new;
}

/*
 * main loop of the editor!
 */
void Editor::run() {
    reset_state(EDIT);

    while (state != EXIT) {
        input();
        r.update();
        render();

        if (state == EXIT) {
           exit_query();
        }
        ++frame_count;
    }
}

void Editor::input() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT: {
                update_state(EXIT);
                break;
            }
            case SDL_WINDOWEVENT: {
                if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    r.WINDOW_WIDTH = e.window.data1;
                    r.WINDOW_HEIGHT = e.window.data2;
                }
                break;
            } // resizing the sdl window
            case SDL_TEXTINPUT: {
                if (select) {
                    select = false;
                    file.delete_selection(selection_start, cursor);
                    reset_state(EDIT);
                } // replace selection with the input character
                if (state == SELECT) {
                    reset_state(EDIT);
                } // switch back to edit state
                if (state == EDIT) {
                    file.place_char(e.text.text[0], cursor);
                }
                break;
            }
            case SDL_KEYDOWN: { 
                if (state == COMMAND) {
                    process_command(e.key.keysym.sym);
                } // process the key pressed while holding cmd
                process_key_down_event(e.key.keysym.sym);
                break;
            }
            case SDL_KEYUP: {
                process_key_up_event(e.key.keysym.sym);
                break;
            }
            default:
                break;
        }
    }
}

/*
 * y/n prompt presented on cmd q;
 * can also exit again to quit
 */
void Editor::exit_query() {
    bool selected = false;
    SDL_Event e;
    // freeze input while the prompt is up
    SDL_StopTextInput();
    while (!selected) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                selected = true;
                break;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_y) {
                    selected = true;
                    break;
                } else if (e.key.keysym.sym == SDLK_n) {
                    selected = true;
                    reset_state(EDIT);
                    break;
                }
            }
        }
    }
    SDL_StartTextInput();
}

/*
 * lock into this query upon cmd o
 * press enter confirm selection
 * creates a new file with the name
 *     if existing name isn't found
 */
void Editor::open_query() {
    bool selected = false;
    SDL_Event e;
    std::string file_name = "";
    std::string t = "open";

    while (!selected) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || e.type == SDLK_ESCAPE) {
                selected = true;
                break;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    selected = true;
                    file.open_file(file_name);
                    reset_state(EDIT);
                    break;
                } else if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    file_name.pop_back();
                    break;
                }
            } else if (e.type == SDL_TEXTINPUT) {
                file_name.append(e.text.text);
                break;
            }
        }
        r.render_command_prompt_bar(e_sstrings[state], t, file_name, cursor);
        r.render_present();
    }
}


void Editor::render() {
    r.render_clear();
    if (file.is_open()) {
        if (select) {
            // renders the selection rectangles made while holding shift
            r.render_text_selection(selection_start, cursor, file);
        } else {
            // renders the dark rectangle, contrasting text on current line
            r.render_line_select(cursor.y);
        }
        r.render_file_contents(file);
    }
    // render & update cursor, should probably divide this functionality
    r.render_cursor(cursor, file, frame_count);
    r.render_status_bar(e_sstrings[state], cursor);
    r.render_present();
}

void Editor::process_command(SDL_Keycode command_char) {
    switch(command_char) {
        case SDLK_c: {
            if (select) {
                file.copy(selection_start, cursor);
                select = false;
                reset_state(EDIT);
            }
            break;
        } // copy 
        case SDLK_v: {
            // delete selected area to be replaced
            if (select) {
                file.delete_selection(selection_start, cursor);
                select = false;
            }
            file.paste(cursor);
            break;
        } // paste
        case SDLK_x: {
            if (select) {
                file.cut(selection_start, cursor);
                select = false;
                reset_state(EDIT);
            }
            break;
        } // cut
        case SDLK_r: {
            reset_state(EDIT);
            break;
        } // refresh
        case SDLK_s: {
            if (file.is_open()) file.save_file();
            reset_state(prev_state);
            break;
        } // save
        case SDLK_o: {
            open_query();
            break;
        } // open
        case SDLK_n: {
            if (file.is_open()) file.save_file();
            cursor = Vec2(0,0);
            file.new_file();
            reset_state(EDIT);
            break;
        } // new file
        case SDLK_q: {
            update_state(EXIT);
            break;  
        } // quit
        default:
            break;
    }
}

void Editor::process_key_down_event(SDL_Keycode kc) {
    switch (kc) {
        case SDLK_ESCAPE: {
            break;
        }
        case SDLK_LSHIFT: {
            // command overrides starting a selection
            if (state != COMMAND) {
                if (!select && state == EDIT) {
                    selection_start = cursor;
                }
                update_state(SELECT);
            }
            break;
        }
        case SDLK_RSHIFT: {
            if (state != COMMAND) {
                if (!select && state == EDIT) {
                    selection_start = cursor;
                }     
                update_state(SELECT);
            }
            break;
        }
        case (MACOS_LEFT_COMMAND): {
            update_state(COMMAND);
            break;
        }
        case (MACOS_RIGHT_COMMAND): {
            update_state(COMMAND);
            break;
        }
        case SDLK_TAB: {
            if (select) {
              file.delete_selection(selection_start, cursor);
            }
            file.place_char('\t', cursor);
            break;
        }
        case SDLK_BACKSPACE: {
            if (!select) {
                file.remove_char(cursor);
            } else {
                file.delete_selection(selection_start, cursor);
                select = false;
            }
            break;
        }
        case SDLK_RETURN: {
            if (select) {
                file.delete_selection(selection_start, cursor);
                select = false;
            }
            file.insert_line(cursor);
            break;
        }
        case SDLK_RIGHT: {
            move_cursor(1, 0);
            if (state == SELECT) {
                select = true;;
            } else {
                select = false;
            }
            break;
        }
        case SDLK_LEFT: {
            move_cursor(-1,0);
            if (state == SELECT) {
                select = true;;
            } else {
                select = false;
            }
            break;
        }
        case SDLK_UP: {
            move_cursor(0,-1);
            if (state == SELECT) {
                select = true;;
            } else {
                select = false;
            }
            break;
        }
        case SDLK_DOWN: {
            move_cursor(0,1);
            if (state == SELECT) {
                select = true;;
            } else {
                select = false;
            }
            break;
        }
        default:
            break;
    }
}

void Editor::process_key_up_event(SDL_Keycode kc) {
    switch (kc) {
        case SDLK_LSHIFT: {;
            reset_state(EDIT);
            break;
        }
        case SDLK_RSHIFT: {
            reset_state(EDIT);
            break;
        }
        case MACOS_LEFT_COMMAND: {
            reset_state(prev_state);
            break;
        }
        case MACOS_RIGHT_COMMAND: {
            reset_state(prev_state);
            break;
        }
        default:
            break;
    }
}

/*
 * takes the offset of the cursor
 * i.ie x = 1, y = 0 moves the cursir 1 to the right
 */
void Editor::move_cursor(int x, int y) {
    Vec2 temp(cursor.x + x, cursor.y + y);
    if (!file.pos_in_bounds(temp)) {
        if (y > 0 && temp.y < file.lines.size()) { // on moving down
            cursor.x = file.lines[temp.y].cs.size(); // move to end of next line
            ++cursor.y;
            return;
        } else if (y < 0 && temp.y >= 0) { // on moving up
            cursor.x = file.lines[temp.y].cs.size(); // move to end of prev line
            --cursor.y;
            return;
        } else if (x > 0 && temp.y + 1 < file.lines.size()) { // on moving right past line
            cursor.x = 0; // move to start of next line
            ++cursor.y;
            return;
        } else if (x < 0 && temp.y - 1 >= 0) { // on moving left past 0
            cursor.x = file.lines[temp.y - 1].cs.size();  // move to end of prev line
            --cursor.y;
            return;
        } else {
            return;
        }
    }
    cursor = temp;
}

Editor::~Editor() {
    std::cout << "cleaning up editor resources..." << std::endl;
    SDL_Quit();
}