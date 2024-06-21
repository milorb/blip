#include "file.hpp"

File::File() {
}

void File::new_file() {
    op = true;
    contents.clear();
    contents.push_back("");
}

void File::open_file(const std::string path) {
    std::string line;
    std::ifstream os;
    os.open(path);

    file_name = path;
    if (!os.is_open()) {
        std::cout << "could not open file " << path << "creating new file with name" << std::endl;
        new_file();
        return;
    }
    op = true;
    while (getline(os, line)) {
        contents.emplace_back(line);
    }
    os.close();
}

void File::save_file() {
    std::cout << "saving file... " << file_name << std::endl;
    std::string line;
    std::ofstream os;
    os.open(file_name, std::ofstream::out | std::ofstream::trunc);
    if (!os.is_open()) {
        std::cout << "could not locate file " << file_name 
            << " while saving, file may have been moved or deleted" << std::endl;
        return;
    }

    for (std::string s : contents) {
        os << s << "\n";
    }
    os.close();
}

bool File::pos_in_bounds(Vec2& pos) {
    if (pos.x < 0 || pos.y >= contents.size() || pos.x < 0 || pos.x > contents[pos.y].length()) {
        return false;
    }

    return true;
}

void File::remove_char(Vec2& pos) {
    // case where the cursor is at the beginning of a line
    if (pos.x == 0) {
        if (pos.y == 0) {
            return;
        } else {
            pos.x = contents[pos.y - 1].length();
            contents[pos.y - 1].append(contents[pos.y]);
            contents.erase(contents.begin() + pos.y);
            --pos.y;
        }
    } else if (pos.x >= contents[pos.y].length()) {
        contents[pos.y].pop_back();
        pos.x = pos.x - 1;
    } else {
        contents[pos.y].erase(pos.x - 1, 1);
        pos.x = pos.x - 1;
    }
}

void File::place_char(char c, Vec2& pos) {
    if (pos.x >= contents[pos.y].length()) {
       contents[pos.y] += c;
    } else {
        insert_char(c, pos);
    }
    ++pos.x;
}

void File::insert_char(char c, Vec2& pos) {
    contents[pos.y].insert(contents[pos.y].begin() + pos.x, 1, c);
}

void File::insert_line(Vec2& pos) {
    if (pos.x == contents[pos.y].length()) {
        if (pos.y >= contents.size() - 1) {
            contents.push_back("");
            pos.y = contents.size() - 1;
        } else {
            contents.insert(contents.begin() + pos.y + 1, "");
            ++pos.y;
        } 
        pos.x = 0;
    } else {
        std::string ins(contents[pos.y].begin() + pos.x, contents[pos.y].end());
        contents[pos.y].erase(pos.x, contents[pos.y].length() - pos.x);
        if (pos.y >= contents.size() - 1) {
            contents.push_back(ins);
            pos.y = contents.size() - 1;
        } else {
            contents.insert(contents.begin() + pos.y + 1, ins);
            ++pos.y;            
        }
        pos.x = 0;
    }
}