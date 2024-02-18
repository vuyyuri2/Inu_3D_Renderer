#include <stdio.h>

#include "windowing/window.h"

extern window_t window;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  create_window(hInstance, 400, 300); 
  while (window.running) {
    poll_events();
  }
}
