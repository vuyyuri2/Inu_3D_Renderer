
// actual Win32 include
#include <windows.h>

#include "utils/vectors.h"

struct input_t {
  ivec2 mouse_pos; 
};

struct window_t {
  input_t input; 
  ivec2 window_dim;

  HWND win32_wnd = NULL;
  bool running = true;
};

void create_window(HINSTANCE h_instance, int width, int height);
void poll_events();
