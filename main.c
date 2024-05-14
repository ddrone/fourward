#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum bytecode_kind {
  LIT,
  PRIM_CALL
} bytecode_kind;

typedef struct bytecode {
  bytecode_kind kind;
  int value;
} bytecode;

typedef struct calculator {
  int stack[1024];
  int* stack_top;

  bytecode bytecode[1024];
  bytecode* here;

  bool interpreting;
  bytecode* ip;
} calculator;

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


void write_lit(calculator* calc, int lit) {
  calc->here->kind = LIT;
  calc->here->value = lit;
  calc->here++;
}

void write_prim_call(calculator* calc, int index) {
  calc->here->kind = PRIM_CALL;
  calc->here->value = index;
  calc->here++;
}

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

bool step(calculator* calc) {
  if (calc->ip >= calc->here) {
    return false;
  }

  switch (calc->ip->kind) {
    case LIT:
      push_value(calc, calc->ip->value);
      break;
    case PRIM_CALL: {
      int index = calc->ip->value;
      if (index >= 0 && index < num_primitives) {
        primitives[index].fn(calc);
      }
      else {
        printf("Unexpected call to primitive with index %d!\n", index);
        exit(3);
      }
      break;
    default:
      printf("Unexpected bytecode kind %d!\n", calc->ip->kind);
      exit(4);
    }
  }
  calc->ip++;
}


void process_token(calculator* calc, char* buffer, char* next) {
  if (buffer == next) {
    return;
  }

  *next = '\0';
  int number;
  if (read_int(buffer, &number)) {
    if (calc->interpreting) {
      push_value(calc, number);
    }
    else {
      write_lit(calc, number);
    }
  }
  else {
    for (int i = 0; i < num_primitives; i++) {
      if (strcmp(buffer, primitives[i].name) == 0) {
        if (calc->interpreting) {
          primitives[i].fn(calc);
        }
        else {
          write_prim_call(calc, i);
        }
        return;
      }
    }
    fputs("Unknown word:\n    ", stdout);
    puts(buffer);
    exit(2);
  }
}

void init_calculator(calculator* calc) {
  calc->stack_top = &calc->stack[0];
  calc->here = &calc->bytecode[0];
  calc->ip = &calc->bytecode[0];
  calc->interpreting = false;
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

  calc.interpreting = true;
  while (step(&calc)) {
    // Do nothing
  }
}
