#include "editor.hpp"

Editor::Editor() {
    frame_count = 0;
    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);
    time = std::localtime(&now_t);



    file.new_file();
    SDL_StartTextInput();
    run();
    SDL_StopTextInput();
}

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

void Editor::run() {
    reset_state(EDIT);

    while (state != EXIT) {
        input();
        render();

        if (state == EXIT) {
           exit_query();
        }
        ++frame_count;
    }
}

void Editor::exit_query() {
    bool selected = false;
    SDL_Event e;
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
            }
            case SDL_TEXTINPUT: {
                if (select) {
                    select = false;
                    file.delete_selection(selection_start, cursor);
                    reset_state(EDIT);
                }
                if (state == EDIT) {
                    file.place_char(e.text.text[0], cursor);
                }
                break;
            }
            case SDL_KEYDOWN: { 
                if (state == COMMAND) {
                    process_command(e.key.keysym.sym);
                }
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

void Editor::render() {
    r.render_clear();
    if (file.is_open()) {
        if (select) {
            r.render_text_selection(selection_start, cursor, file);
        } else {
            r.render_line_select(cursor.y);
        }
        r.render_file_contents(file);
    }
    if (frame_count % 180 >= 90) {
        r.render_cursor(cursor, file);
    }
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
            std::cout << "creating new file..." << std::endl;
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
            if (state == EDIT || state == SELECT) {
                if (!select && state == EDIT) {
                    selection_start = cursor;
                    select = true;
                }
                update_state(SELECT);
            }
            break;
        }
        case SDLK_RSHIFT: {
            if (state == EDIT || state == SELECT) {
                if (!select && state == EDIT) {
                    selection_start = cursor;
                    select = true; 
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

void Editor::move_cursor(int x, int y) {
    Vec2 temp(cursor.x + x, cursor.y + y);
    //std::cout << "moving cursor from " << cursor.x << " " << cursor.y 
            //<< " to " << temp.x << " " << temp.y << std::endl;;
    if (!file.pos_in_bounds(temp)) {
        if (y > 0 && temp.y < file.lines.size()) {
            //std::cout << "down case" << std::endl;
            cursor.x = file.lines[temp.y].cs.size();
            ++cursor.y;
            //std::cout << "cursor now at " << cursor.x << " " << cursor.y << std::endl;
            return;
        } else if (y < 0 && temp.y >= 0) {
            //std::cout << "up case" << std::endl;
            cursor.x = file.lines[temp.y].cs.size();
            --cursor.y;
            return;
        } else if (x > 0 && temp.y + 1 < file.lines.size()) {
            //std::cout << "right case" << std::endl;
            cursor.x = 0;
            ++cursor.y;
            return;
        } else if (x < 0 && temp.y - 1 >= 0) {
            //std::cout << "left case" << std::endl;
            cursor.x = file.lines[temp.y - 1].cs.size();
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