#include <stdio.h>

const int true = 1;

void flush_token(char* buffer, char* next) {
  if (buffer == next) {
    return;
  }

  *next = '\0';
  puts(buffer);
}

int main() {
  char buffer[1024];
  char* next = buffer;

  while (true) {
    int c = getchar();
    if (c == EOF) {
      break;
    }

    switch (c) {
      case ' ':
      case '\n':
      case '\t':
      case '\r':
        flush_token(buffer, next);
        next = buffer;
        break;
      default:
        *next++ = c;
    }
  }

  flush_token(buffer, next);
}
