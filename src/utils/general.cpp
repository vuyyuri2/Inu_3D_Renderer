#include "general.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

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
