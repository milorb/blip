#ifndef STATES_HPP
#define STATES_HPP

#include <string>
#include <unordered_map>

/**
 * Possible additional states:
 * Edit mode
 * Select mode
 * Command mode
 */

enum C_type { BLANK, CHAR, SPACE, TAB };  // char wrapper state

enum E_State { COMMAND, EDIT, EXIT, RESTING, SELECT };  // editor state

enum F_State { SAVED, UNSAVED };  // file state

static inline std::unordered_map<E_State, std::string> e_sstrings = {
    {COMMAND, "command"},
    {EDIT, "edit"},
    {EXIT, "exit"},
    {RESTING, "resting"},
    {SELECT, "select"}};

#endif