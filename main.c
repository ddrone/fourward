#include <stdio.h>

const int true = 1;

int main() {
  while (true) {
    int c = getchar();
    if (c == EOF) {
      break;
    }

    putchar(c);
  }
}
