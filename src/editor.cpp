#include "editor.hpp"

Editor::Editor() {
    frame_count = 0;

    run();
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


void Editor::play_intro() {
    intro = true;
    reset_state(RESTING);
    while (intro) {
        input();
        render_intro();
        if (state == EXIT || state == EDIT) {
            intro = false;
            SDL_StopTextInput();
            std::cout << "exiting intro..." << std::endl;
        }
    }
}

void Editor::run() {
    play_intro();
    if (state == EXIT) {
        return;
    }

    reset_state(EDIT);
    SDL_StartTextInput();
    while (state != EXIT) {
        input();
        render();
        if (state == EXIT) {
            SDL_StopTextInput();
            play_intro();
            SDL_StartTextInput();
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
            case SDL_TEXTINPUT: {
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
    if (file.is_open()) r.render_file_contents(file);
    if (frame_count % 200 > 100) {
        r.render_cursor(cursor, file);
    }
    r.render_present();
}

void Editor::render_intro() {
    std::vector<std::string> intro_mssg = {
        "blip", 
        "",
        "new file : command + n",
        "open file : command + o",
        "save file : command + s",
        "quit : command + q"
    };

    if (file.is_open()) {
        intro_mssg.push_back("");
        intro_mssg.push_back("resume : command + r");
    }

    r.render_clear();

    for (int i = 0; i < intro_mssg.size(); ++i) {
        int w,h;
        TTF_SizeText(r.f, intro_mssg[i].c_str(), &w,& h);
        if (intro_mssg[i] != "") {
            r.render_text(intro_mssg[i], (WINDOW_WIDTH / 2) - (w / 2), 60 + i * 20);
        }
    }

    r.render_present();
}

void Editor::process_command(char command_char) {
    switch(command_char) {
        case 'r': {
            if (intro && file.is_open()) {
                reset_state(EDIT);
            }
            break;
        }
        case 's': {
            if (file.is_open()) file.save_file();
            reset_state(prev_state);
            break;
        } 
        case 'o': {
            break;
        }
        case 'n': {
            std::cout << "creating new file..." << std::endl;
            if (file.is_open()) file.save_file();
            cursor = Vec2(0,0);
            file.new_file();
            reset_state(EDIT);
            break;
        }
        case 'q': {
            update_state(EXIT);
            break;
        }
        default:
            break;
    }
}

void Editor::process_key_down_event(SDL_Keycode kc) {
    switch (kc) {
        case (MACOS_LEFT_COMMAND): {
            if (state != COMMAND)
                update_state(COMMAND);
            break;
        }
        case (MACOS_RIGHT_COMMAND): {
            if (state != COMMAND)
                update_state(COMMAND);
            break;
        }
        case SDLK_BACKSPACE: {
            file.remove_char(cursor);
            break;
        }
        case SDLK_RETURN: {
            file.insert_line(cursor);
            break;
        }
        case SDLK_RIGHT: {
            move_cursor(1, 0);
            break;
        }
        case SDLK_LEFT: {
            move_cursor(-1,0);
            break;
        }
        case SDLK_UP: {
            move_cursor(0,-1);
            break;
        }
        case SDLK_DOWN: {
            move_cursor(0,1);
            break;
        }
        default:
            break;
    }
}

void Editor::process_key_up_event(SDL_Keycode kc) {
    switch (kc) {
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

    if (!file.pos_in_bounds(temp)) {
        if (y > 0 && temp.y < file.contents.size()) {
            cursor.x = file.contents[temp.y].length();
            ++cursor.y;
            return;
        } else if (y < 0 && temp.y >= 0) {
            cursor.x = file.contents[temp.y].length();
            --cursor.y;
            return;
        } else if (x > 0 && temp.y + 1 < file.contents.size()) {
            cursor.x = 0;
            ++cursor.y;
            return;
        } else if (x < 0 && temp.y - 1 >= 0) {
             cursor.x = file.contents[temp.y - 1].length();
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