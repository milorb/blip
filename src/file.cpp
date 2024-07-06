#include "file.hpp"

File::File() {
}

Line::Line() {}

void File::new_file() {
    op = true;
    lines.clear();
    lines.push_back(Line());
}

void File::open_file(const std::string path) {
    std::string line;
    std::ifstream ifs;
    lines.clear();

    ifs.open(path);

    file_name = path;
    if (!ifs.is_open()) {
        std::cout << "could not open file " << path << "creating new file with name" << std::endl;
        new_file();
        return;
    }
    op = true;
    std::string s;
    lines.push_back(Line());
    while (getline(ifs, s)) {
        for (char c : s) {
            lines[lines.size()-1].cs.push_back(Character(c));
        }
        lines.push_back(Line());
    }
    lines.pop_back();
    ifs.close();
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

    for (Line &l : lines) {
        for (Character &c : l.cs) {
            os << c.c;
        }
        os << "\n";
    }
    os.close();
}

bool File::pos_in_bounds(Vec2& pos) {
    bool ret;
    (pos.x < 0 || 
    pos.y >= lines.size() || 
    pos.x < 0 || pos.x > lines[pos.y].cs.size()) ?
    ret = false : ret = true;
    return ret;
}

void File::remove_char(Vec2& pos) {
    // case where the cursor is at the beginning of a line
    if (pos.x == 0) {
        if (pos.y == 0) {
            return;
        } else {
            pos.x = lines[pos.y - 1].cs.size();
            lines[pos.y - 1].append_line(lines[pos.y]);
            lines.erase(lines.begin() + pos.y);
            //lines[pos.y].remove_sub_line(0, lines[pos.y].cs.size());
            --pos.y;
        }
    } else if (pos.x >= lines[pos.y].cs.size()) {
        lines[pos.y].cs.pop_back();
        --pos.x;
    } else {
        lines[pos.y].remove_sub_line(pos.x - 1, pos.x);
        --pos.x;
    }
}

void File::place_char(char c, Vec2& pos) {
    lines[pos.y].insert_char(c, pos.x);
    ++pos.x;
}


void Line::insert_char(char c, int idx) {
    if (idx >= cs.size()) {
       cs.push_back(Character(c));
    } else {
      cs.insert(cs.begin() + idx, 1, Character(c));
    }
}

// get range of characters from [a, b)
Line Line::get_sub_line(int a, int b) {
    if (a < 0 || b < 0 || b > cs.size() || a > cs.size()) {
        std::cout << "error: attempted to index sub_line with invalid range" << std::endl;
        std::exit(1);
    }

    Line sub;
    for (int i = std::min(a, b); i < std::max(a, b); ++i) {
        sub.cs.push_back(cs[i]);
    }

    return sub;
}

std::string Line::get_sub_line_str(int a, int b) {
    if (a < 0 || b < 0 || b > cs.size() || a > cs.size() ) {
        std::cout << "error: attempted to index sub_line with invalid range" << std::endl;
        std::exit(1);
    }

    std::string sub = "";
    for (int i = std::min(a, b); i < std::max(a, b); ++i) {
        if (i != cs.size()) {
            sub += cs[i].c;
        }
    }

    return sub;
}

void Line::append_line(Line& added) {
    for (Character c : added.cs) {
        cs.push_back(c);
    }
}

void Line::remove_sub_line(int a, int b) {
    if (a < 0 || b < 0 || a > cs.size() || b > cs.size()) {
        std::cout << "error: attempted to index remove_sub_line with invalid range" << std::endl;
        std::exit(1);
    }

    if (std::max(a, b) - std::min(a, b) == cs.size()) {
        cs.clear();
        return;
    }

    cs.erase(cs.begin() + std::min(a,b), 
        cs.begin() + std::max(a, b));
}

Vec2 Line::get_subline_pxs(int a, int b) {
    if (a < 0 || b < 0 ||  a > cs.size() || b > cs.size()) {
        std::cout << "error: attempted to get pixel length of sub-line with invalid range" << std::endl;
        std::exit(1);
    }

    if (a == b) {
        return Vec2(0, 0);
    } else if (a == cs.size()) {
       return Vec2(cs[b].start_pixel.x, cs[a - 1].end_pixel.x);
    } else if (b == cs.size()) {
        return Vec2(cs[a].start_pixel.x, cs[b - 1].end_pixel.x);
    } else if (a < b) {
        return Vec2(cs[a].start_pixel.x, cs[b].start_pixel.x);
    } else {
        return Vec2(cs[b].start_pixel.x, cs[a].start_pixel.x);
    }
}

void File::insert_line(Vec2& cursor) {
    Line ins;
    Line &cur = lines[cursor.y];

    if (cursor.y == lines.size() - 1 && cursor.x == lines[cursor.y].cs.size()) {
        lines.push_back(Line());
        cursor.y++;
        cursor.x = 0;
        return;
    }

    //std::cout << "inserting line at " << cursor.x << cursor.y <<std::endl;
    if (cursor.x != lines[cursor.y].cs.size()) {
        ins = cur.get_sub_line(cursor.x, cur.cs.size());
        cur.remove_sub_line(cursor.x, cur.cs.size());
    }
    if (cursor.y >= lines.size() - 1) {
        lines.push_back(ins);
        cursor.y = lines.size() - 1;
    } else {
        lines.insert(lines.begin() + cursor.y + 1, ins);
        ++cursor.y;            
    }
    cursor.x = 0;
}

void File::delete_selection(Vec2 &start, Vec2 &end) {

    Vec2 final;
    Vec2 upper;
    Vec2 lower;
    if (start.y <= end.y) {
        final = start;
        upper = start;
        lower = end;
    } else {
        final = end;
        upper = end;
        lower = start;
    }

    Line &l = lines[upper.y];
    if (!l.cs.empty()) {
        if (upper.y == lower.y) {
            l.remove_sub_line(upper.x, lower.x);
        } else {
            l.remove_sub_line(upper.x, l.cs.size());
        }
    }

    for (int i = upper.y + 1; i < lower.y; ++i) {
        lines.erase(lines.begin() + i);
        --i;
        --lower.y;
    }

    Line &ll = lines[lower.y];
    if (!ll.cs.empty() && upper.y != lower.y) {
        ll.remove_sub_line(0, lower.x);
    } 
    if (ll.cs.empty()) {
        if (lower.y == lines.size()) {
            lines.pop_back();
        } else {
            lines.erase(lines.begin() + lower.y);
        }
    }

    if (start.y < end.y) {
        end.y = start.y;
        end.x = start.x;
    }
    start.x = end.x;
    start.y = end.y;    
}

void File::cut(Vec2 &start, Vec2 &end) {
    copy(start, end);
    delete_selection(start, end);
}

void File::copy(Vec2 &start, Vec2 &end) {
    std::string s = "";

    bool xtra_line = false;
    Vec2 upper;
    Vec2 lower;
    if (start.y <= end.y) {
        upper = start;
        lower = end;
    } else {
        upper = end;
        lower = start;
    }

    Line l = lines[upper.y];
    if (!l.cs.empty()) {
        if (upper.y == lower.y) {
            std::string line = l.get_sub_line_str(upper.x, lower.x);
            s += line;
        } else {
            std::string line = l.get_sub_line_str(upper.x, l.cs.size());
            s += line;
        }
    }

    s += "\n";
    for (int i = upper.y + 1; i < lower.y; ++i) {
        l = lines[i];
        if (!l.cs.empty()) {
            std::string line = l.get_sub_line_str(0, l.cs.size());
            s += line;
            s += "\n";
        } else {
            s += "\n";
        }
    }

    l = lines[lower.y];
    if (!l.cs.empty() && upper.y != lower.y) {
        std::string line = l.get_sub_line_str(0, lower.x);
        s += line;
    }

    FILE* pipe = popen("pbcopy", "w");
    if (pipe == nullptr) {
        std::cerr << "Failed to open pipe for pbcopy." << std::endl;
        return;
    }
    fwrite(s.c_str(), sizeof(char), s.length(), pipe);
    pclose(pipe);
}

void File::paste(Vec2 &pos) {
    FILE* pipe = popen("pbpaste", "r");
    if (pipe == nullptr) {
        std::cerr << "Failed to open pipe for pbpaste." << std::endl;
    }
    std::vector<char> buffer(256);

    std::string copied_str;;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        copied_str += buffer.data();
    }
    pclose(pipe);

    for (char c : copied_str) {
        if (c == '\n') {
            insert_line(pos);
            //lines[lines.size() - 1].cs.push_back(Character());
        } else {
            place_char(c, pos);
        }
    }
}