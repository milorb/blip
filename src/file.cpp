#include "file.hpp"

File::File() {}

Line::Line() {}

void File::new_file() {
  op = true;
  lines.clear();
  // editor needs at least one line to make calls
  lines.push_back(Line());
}

void File::open_file(const std::string path) {
  std::string line;
  std::ifstream ifs;
  lines.clear();

  ifs.open(path);
  file_name = path;
  if (!ifs.is_open()) {
    std::cout << "could not open file " << path << "creating new file with name"
              << std::endl;
    new_file();
    return;
  }
  op = true;
  std::string s;
  lines.push_back(Line());
  while (getline(ifs, s)) {
    for (char c : s) {
      lines[lines.size() - 1].cs.push_back(Character(c));
    }
    lines.push_back(Line());
  }
  lines.pop_back();
  ifs.close();
}

void File::save_file() {
  std::string line;
  std::ofstream os;
  os.open(file_name, std::ofstream::out | std::ofstream::trunc);
  if (!os.is_open()) {
    std::cout << "could not locate file " << file_name
              << " while saving, file may have been moved or deleted"
              << std::endl;
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

/*
 * whether the cursor lies within the current file space
 */
bool File::pos_in_bounds(Vec2 &pos) {
  bool ret;
  (pos.x < 0 || pos.y >= lines.size() || pos.x < 0 ||
   pos.x > lines[pos.y].cs.size())
      ? ret = false
      : ret = true;
  return ret;
}

void File::remove_char(Vec2 &pos) {
  // case where the cursor is at the beginning of a line
  if (pos.x == 0) {
    if (pos.y == 0) {
      return;
    } else {
      pos.x = lines[pos.y - 1].cs.size();
      lines[pos.y - 1].append_line(lines[pos.y]);
      lines.erase(lines.begin() + pos.y);
      --pos.y;
    }
    // cursor past end of line
  } else if (pos.x >= lines[pos.y].cs.size()) {
    lines[pos.y].cs.pop_back();
    --pos.x;
  } else {
    lines[pos.y].remove_sub_line(pos.x - 1, pos.x);
    --pos.x;
  }
}

void File::place_char(char c, Vec2 &pos) {
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
    std::cout << "error: attempted to index sub_line with invalid range"
              << std::endl;
    std::exit(1);
  }

  Line sub;
  for (int i = std::min(a, b); i < std::max(a, b); ++i) {
    sub.cs.push_back(cs[i]);
  }

  return sub;
}

std::string Line::get_sub_line_str(int a, int b) {
  if (a < 0 || b < 0 || b > cs.size() || a > cs.size()) {
    std::cout << "error: attempted to index sub_line with invalid range"
              << std::endl;
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

void Line::append_line(Line &added) {
  for (Character c : added.cs) {
    cs.push_back(c);
  }
}

void Line::remove_sub_line(int a, int b) {
  if (a < 0 || b < 0 || a > cs.size() || b > cs.size()) {
    std::cout << "error: attempted to index remove_sub_line with invalid range"
              << std::endl;
    std::exit(1);
  }

  if (std::max(a, b) - std::min(a, b) == cs.size()) {
    cs.clear();
    return;
  }

  cs.erase(cs.begin() + std::min(a, b), cs.begin() + std::max(a, b));
}

/*
 * get the width in pixels over the following range of characters
 * i.e b.max - a.min
 */
Vec2 Line::get_subline_pxs(int a, int b) {
  if (a < 0 || b < 0 || a > cs.size() || b > cs.size()) {
    std::cout
        << "error: attempted to get pixel length of sub-line with invalid range"
        << std::endl;
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

void File::insert_line(Vec2 &cursor) {
  Line ins;
  Line &cur = lines[cursor.y];

  if (cursor.y == lines.size() - 1 && cursor.x == lines[cursor.y].cs.size()) {
    lines.push_back(Line());
    cursor.y++;
    cursor.x = 0;
    return;
  }
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

/* modifies cursor and selection locations
 * this function, copy, and Renderer::render_text_selection()
 *  follow a similar pattern, only delete moves in the opposite direction
 */
void File::delete_selection(Vec2 &selection_start, Vec2 &cursor) {
  Vec2 final;
  Vec2 upper;
  Vec2 lower;
  if (selection_start.y <= cursor.y) {
    final = selection_start;
    upper = selection_start;
    lower = cursor;
  } else {
    final = cursor;
    upper = cursor;
    lower = selection_start;
  }

  // need to go from bottom up to prevent shifting indeces
  // bottom line of selection (or just selection when only one line)
  Line &ll = lines[lower.y];
  if (!ll.cs.empty()) {
    if (upper.y == lower.y) {
      ll.remove_sub_line(upper.x, lower.x);
    } else {
      ll.remove_sub_line(0, lower.x);
    }
  }

  // don't remove the line if it's the only line in the selection
  // editor breaks if file has zero lines
  if (ll.cs.empty() && upper.y != lower.y) {
    if (lower.y == lines.size()) {
      lines.pop_back();
    } else {
      lines.erase(lines.begin() + lower.y);
    }
  }

  // erase everything between the start and end sections of the selection
  for (int i = lower.y - 1; i > upper.y; --i) {
    lines.erase(lines.begin() + i);
  }

  // if there is a distinc upper line, delete it
  if (upper.y != lower.y) {
    Line &l = lines[upper.y];
    if (!l.cs.empty()) {
      l.remove_sub_line(upper.x, l.cs.size());
    }
    if (upper.y + 1 < lines.size()) {
      l.append_line(lines[upper.y + 1]);
      if (upper.y + 1 == lines.size() - 1) {
        lines.pop_back();
      } else {
        lines.erase(lines.begin() + upper.y + 1);
      }
    }
  }

  cursor.x = std::min(upper.x, (int)lines[upper.y].cs.size());
  cursor.y = upper.y;

  selection_start.x = cursor.x;
  selection_start.y = cursor.y;
}

void File::cut(Vec2 &selection_start, Vec2 &cursor) {
  copy(selection_start, cursor);
  delete_selection(selection_start, cursor);
}

/*
 * copy and paste both open a pipe to the clipboard 
 */

void File::copy(Vec2 selection_start, Vec2 cursor) {
  std::string s = "";

  Vec2 upper;
  Vec2 lower;
  if (selection_start.y <= cursor.y) {
    upper = selection_start;
    lower = cursor;
  } else {
    upper = cursor;
    lower = selection_start;
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
  if (upper.y != lower.y) {
    s += "\n";
  }
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

  FILE *pipe = popen("pbcopy", "w");
  if (pipe == nullptr) {
    std::cerr << "error: failed to open pipe for pbcopy." << std::endl;
    return;
  }
  fwrite(s.c_str(), sizeof(char), s.length(), pipe);
  pclose(pipe);
}

void File::paste(Vec2 &cursor) {
  FILE *pipe = popen("pbpaste", "r");
  if (pipe == nullptr) {
    std::cerr << "error: failed to open pipe for pbpaste." << std::endl;
  }
  std::vector<char> buffer(256);

  std::string copied_str;
  ;
  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    copied_str += buffer.data();
  }
  pclose(pipe);

  for (char c : copied_str) {
    if (c == '\n') {
      insert_line(cursor);
    } else {
      place_char(c, cursor);
    }
  }
}