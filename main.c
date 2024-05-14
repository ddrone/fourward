#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int bool;
const bool true = 1;
const bool false = 0;

typedef struct calculator {
  int stack[1024];
  int* stack_top;
} calculator;

typedef void (*handler)(calculator*);

typedef struct primitive {
  char* name;
  handler fn;
} primitive;

bool read_int(char* buffer, int* result) {
  char* end;
  *result = strtol(buffer, &end, 10);
  return *end == '\0' && *buffer != '\0';
}

void push_value(calculator* calc, int value) {
  *calc->stack_top++ = value;
}

int pop_value(calculator* calc) {
  if (calc->stack_top == calc->stack) {
    puts("Stack underflow!");
    exit(1);
  }

  calc->stack_top--;
  return *calc->stack_top;
}

void prim_add(calculator* calc) {
  int x = pop_value(calc);
  int y = pop_value(calc);
  push_value(calc, x + y);
}

void prim_mul(calculator* calc) {
  int x = pop_value(calc);
  int y = pop_value(calc);
  push_value(calc, x * y);
}

void prim_print(calculator* calc) {
  int x = pop_value(calc);
  printf("%d\n", x);
}

static primitive primitives[] = {
  { .name = "+", .fn = prim_add },
  { .name = "*", .fn = prim_mul },
  { .name = "print", .fn = prim_print },
};

static const int num_primitives = sizeof(primitives) / sizeof(primitives[0]);

void process_token(calculator* calc, char* buffer, char* next) {
  if (buffer == next) {
    return;
  }

  *next = '\0';
  int number;
  if (read_int(buffer, &number)) {
    push_value(calc, number);
  }
  else {
    for (int i = 0; i < num_primitives; i++) {
      if (strcmp(buffer, primitives[i].name) == 0) {
        primitives[i].fn(calc);
        return;
      }
    }
    fputs("Unknown word:\n    ", stdout);
    puts(buffer);
    exit(2);
  }
}

void init_calculator(calculator* calc) {
  calc->stack_top = &calc->stack[0]; // Is the right-hand side valid? I have no idea
}


int main() {
  char buffer[1024];
  char* next = buffer;
  calculator calc;
  init_calculator(&calc);

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
        process_token(&calc, buffer, next);
        next = buffer;
        break;
      default:
        // TODO: check for buffer overflow here
        *next++ = c;
    }
  }

  process_token(&calc, buffer, next);
}
