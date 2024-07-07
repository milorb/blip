#include <iostream>

#include "editor.hpp"

/*#############################################################*/ /*

 Hi, this is a small text editor for macOS
 it utilizes sdl2 for rendering

 **fonts are not provided**, font can be changed in renderer.cpp;
   Renderer::Renderer() {
     ...
     ...
     if (TTF_Init() != 0) Utilities::sdl_error();
     f = TTF_OpenFont("../resources/Rubik-Regular.TTF", 18);
     if (!f) Utilities::sdl_error();
     ...
     ...
 place them in a resources/ folder in the top of the directory

 */
/*##############################################################*/

int main(void) {
  Editor e;
  return 0;
}