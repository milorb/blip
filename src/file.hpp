#ifndef FILE_HPP
#define FILE_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "utilities.hpp"

enum F_State {SAVED, UNSAVED};

class File {
public:
    File();
    void open_file(const std::string path);
    void new_file();
    // void save()
    // void save_as_file(std::string path)
    void save_file();
    void place_char(char c, Vec2& pos);
    void remove_char(Vec2& pos);
    bool pos_in_bounds(Vec2& pos);
    void insert_line(Vec2 & pos);

    bool is_open() {return op;}

    std::vector<std::string> contents;
    std::vector<std::string> wrapped_contents;

private:
    bool op = false;
    std::string file_name;
    void append_char(char c, int line);
    void insert_char(char c, Vec2 &pos);
    
    F_State state;
};

#endif