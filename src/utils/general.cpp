#include "general.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "log.h"
#include "utils/app_info.h"

char* get_file_contents(const char* full_path) {
  FILE* file = fopen(full_path, "r");
  inu_assert(file, "could not open file to get file contents");

  int size = 100;
  char* contents_buffer = (char*)malloc(size * sizeof(char));
  int count = 0;

  while (!feof(file)) {
    if (count >= size) {
      size *= 2; 
      contents_buffer = (char*)realloc(contents_buffer, size * sizeof(char));
    }
    char c = fgetc(file);
    if (c > 0) {
        contents_buffer[count] = c;
        count++;
    }
  }
  if (count == size) {
    contents_buffer = (char*)realloc(contents_buffer, (size+1) * sizeof(char));
    contents_buffer[size] = 0;
  } else {
    memset(contents_buffer + count, 0, size - count);
  }

  fclose(file);
  return contents_buffer;
}

extern app_info_t app_info;
void get_resources_folder_path(char path_buffer[256]) {	
	static bool got_root_path = false;
	static char s_folder[256]{};
	if (!got_root_path) {
		char folder[256]{};
		if (app_info.running_in_vs) {
		  GetCurrentDirectoryA(256, folder);
		  char* slash_before_build_folder = strrchr(folder, '\\');
		  *slash_before_build_folder = 0;
      char* slash_before_out_folder = strrchr(folder, '\\');
		  *slash_before_out_folder = 0;
		} else {
		  GetCurrentDirectoryA(256, folder);
		}

		sprintf(s_folder, "%s\\resources", folder);
		got_root_path = true;
	}
	memcpy(path_buffer, s_folder, strlen(s_folder)); 
}
