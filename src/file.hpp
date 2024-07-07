#ifndef FILE_HPP
#define FILE_HPP

#include <cstdio>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "states.hpp"
#include "utilities.hpp"

struct Character {
    Character() : c('\0'), t(BLANK), start_pixel(Vec2()), end_pixel(Vec2()){}

    Character(char inc) {
        if (inc == ' ') {
             t = SPACE;
        } else if (inc == '\t') {
            t = TAB;
        } else {
            t = CHAR;
        }
        c = inc;
    }

    char c;
    C_type t;
    Vec2 start_pixel;
    Vec2 end_pixel;
};

struct Line {
    Line();
    Line(std::string& in) {
        for (char c : in) cs.push_back(Character(c));
    }

    Line get_sub_line(int a, int b);
    std::string get_sub_line_str(int a, int b);
    Vec2 get_subline_pxs(int a, int b);
    void remove_sub_line(int a, int b);
    void append_line(Line& added);

    void insert_char(char c, int idx);
    std::vector<Character> cs;
};

class File {
public:
    File();
    void open_file(const std::string path);
    void new_file();
    void save_file();
    void place_char(char c, Vec2& pos);
    void remove_char(Vec2& pos);
    void insert_line(Vec2 & pos);

    void delete_selection(Vec2 &selection_start, Vec2 &cursor);
    void cut(Vec2 &selection_start, Vec2 &cursor);
    void copy(Vec2 selection_start, Vec2 cursor);
    void paste(Vec2 &pos);

    bool pos_in_bounds(Vec2& pos);
    bool is_open() {return op;}

    std::vector<Line> lines;
    std::string file_name;
    
private:
    bool op = false;
    void append_char(char c, int line);
    void insert_char(char c, Vec2 &pos);
    
    F_State state;
};

#endif