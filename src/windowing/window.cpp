#include "window.h"

#include "utils/log.h"

#include <iostream>
#include <Windowsx.h>
#include <wingdi.h>
#include <winuser.h>

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#define SHOW_CONSOLE 1

window_t window;

LRESULT CALLBACK window_procedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void create_window(HINSTANCE h_instance, int width, int height) {

#if SHOW_CONSOLE
  AllocConsole();
  AttachConsole(GetCurrentProcessId());
  freopen("CON", "w", stdout);
#endif

  const char* CLASS_NAME = "Inu Renderer";

  WNDCLASS win_class = {};
  win_class.lpfnWndProc = window_procedure;
  win_class.hInstance = h_instance;
  win_class.lpszClassName = TEXT(CLASS_NAME);

  RegisterClass(&win_class);

  RECT adjusted_win_size{};
  adjusted_win_size.right = width;
  adjusted_win_size.bottom = height;
  AdjustWindowRectEx(
      &adjusted_win_size, 
      WS_OVERLAPPEDWINDOW,
      FALSE,
      0
  );

  window.win32_wnd = CreateWindowEx(
    0,
    TEXT(CLASS_NAME),
    TEXT("Inu 3D Renderer"),
    WS_OVERLAPPEDWINDOW,

    CW_USEDEFAULT, 
    CW_USEDEFAULT, 

    // width and height to this function needs to include non-client and client area combined
    adjusted_win_size.right - adjusted_win_size.left, 
    adjusted_win_size.bottom - adjusted_win_size.top,

    NULL,
    NULL,
    h_instance,
    NULL
  );

  inu_assert(window.win32_wnd != NULL, "window could not be created");

  window.window_dim.x = width;
  window.window_dim.y = height;

  window.running = true;

  ShowWindow(window.win32_wnd, SW_SHOWNORMAL);

  // OPENGL INITIALIZATION
  HDC device_context = GetDC(window.win32_wnd);

  PIXELFORMATDESCRIPTOR pixel_format = { 
    sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd  
    1,                     // version number  
    PFD_DRAW_TO_WINDOW |   // support window  
    PFD_SUPPORT_OPENGL |   // support OpenGL  
    PFD_DOUBLEBUFFER,      // double buffered  
    PFD_TYPE_RGBA,         // RGBA type  
    24,                    // 24-bit color depth  
    0, 0, 0, 0, 0, 0,      // color bits ignored  
    0,                     // no alpha buffer  
    0,                     // shift bit ignored  
    0,                     // no accumulation buffer  
    0, 0, 0, 0,            // accum bits ignored  
    32,                    // 32-bit z-buffer  
    0,                     // no stencil buffer  
    0,                     // no auxiliary buffer  
    PFD_MAIN_PLANE,        // main layer  
    0,                     // reserved  
    0, 0, 0                // layer masks ignored  
  };

  int pixel_format_idx = ChoosePixelFormat(device_context, &pixel_format);
  SetPixelFormat(device_context, pixel_format_idx, &pixel_format);

  HGLRC gl_render_context = wglCreateContext(device_context);
  wglMakeCurrent(device_context, gl_render_context);

  HDC gl_device_context = wglGetCurrentDC();
  inu_assert(gl_device_context == device_context, "opengl device context not the same as the window's\n");
}

LRESULT CALLBACK window_procedure(HWND h_window, UINT u_msg, WPARAM w_param, LPARAM l_param) {
  switch (u_msg) {
    case WM_DESTROY: {
      window.running = false;
      PostQuitMessage(0);
      return 0;
    }
    case WM_LBUTTONUP: {
      printf("left button up\n");
      break;
    }
    case WM_MOUSEMOVE: {
      // bottom left should be (0,0)
      window.input.mouse_pos.x = GET_X_LPARAM(l_param);
      window.input.mouse_pos.y = window.window_dim.y - GET_Y_LPARAM(l_param);
      printf("mouse pos: (%i,%i)\n", window.input.mouse_pos.x, window.input.mouse_pos.y);
      break;
    }
  }
  return DefWindowProc(h_window, u_msg, w_param, l_param);
}

void poll_events() {
  MSG msg{};
  while (PeekMessage(&msg, window.win32_wnd, 0, 0, 0)) {
    bool quit_msg = (GetMessage(&msg, NULL, 0, 0) == 0);
    if (quit_msg) break;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}
