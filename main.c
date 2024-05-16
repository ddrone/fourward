#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum bytecode_kind {
  LIT,
  PRIM_CALL,
  USER_CALL,
  BRANCH,
  COND_BRANCH,
} bytecode_kind;

typedef struct bytecode {
  bytecode_kind kind;
  int value;
} bytecode;

typedef struct user_function {
  char* name;
  int start;
  struct user_function* next;
  bool immediate;
} user_function;

typedef struct calculator {
  char buffer[1024];
  char* next;
  bool eof;

  int stack[1024];
  int* stack_top;

  int call_stack[1024];
  int call_stack_index;

  bytecode bytecode[1024];
  int here;

  bool interpreting;

  user_function* last;
} calculator;

bool eat_token(calculator* calc);
bool step(calculator* calc);

void run(calculator* calc) {
  while (step(calc)) {
    // Do nothing
  }
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


void write_lit(calculator* calc, int lit) {
  calc->bytecode[calc->here].kind = LIT;
  calc->bytecode[calc->here].value = lit;
  calc->here++;
}

void write_prim_call(calculator* calc, int index) {
  calc->bytecode[calc->here].kind = PRIM_CALL;
  calc->bytecode[calc->here].value = index;
  calc->here++;
}

void write_user_call(calculator* calc, int offset) {
  calc->bytecode[calc->here].kind = USER_CALL;
  calc->bytecode[calc->here].value = offset;
  calc->here++;
}

void write_branch(calculator* calc, int offset) {
  calc->bytecode[calc->here].kind = BRANCH;
  calc->bytecode[calc->here].value = offset;
  calc->here++;
}

void write_0branch(calculator* calc, int offset) {
  calc->bytecode[calc->here].kind = COND_BRANCH;
  calc->bytecode[calc->here].value = offset;
  calc->here++;
}

typedef void (*handler)(calculator*);

typedef struct primitive {
  char* name;
  handler fn;
  bool immediate;
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

void prim_sub(calculator* calc) {
  int x = pop_value(calc);
  int y = pop_value(calc);
  push_value(calc, y - x);
}

void prim_mul(calculator* calc) {
  int x = pop_value(calc);
  int y = pop_value(calc);
  push_value(calc, x * y);
}

void prim_le(calculator* calc) {
  int y = pop_value(calc);
  int x = pop_value(calc);
  push_value(calc, x < y);
}

void prim_lt(calculator* calc) {
  int y = pop_value(calc);
  int x = pop_value(calc);
  push_value(calc, x <= y);
}

void prim_print(calculator* calc) {
  int x = pop_value(calc);
  printf("%d\n", x);
}

void prim_dup(calculator* calc) {
  int x = pop_value(calc);
  push_value(calc, x);
  push_value(calc, x);
}

void prim_colon(calculator* calc) {
  calc->interpreting = false;
  if (!eat_token(calc)) {
    puts("Expected function name after ':'!");
    exit(5);
  }

  int name_length = strlen(calc->buffer);
  char* name_copy = malloc(name_length + 1);
  strcpy(name_copy, calc->buffer);

  user_function* new_definition = malloc(sizeof(user_function));
  new_definition->name = name_copy;
  new_definition->next = calc->last;
  new_definition->start = calc->here;
  new_definition->immediate = false;
  calc->last = new_definition;
  // printf("Defining function %s\n", name_copy);
}

void prim_semicolon(calculator* calc) {
  write_prim_call(calc, 0); // Write "ret" at the end of current function!
  // puts("Finished defining function");
  calc->interpreting = true;
}

void prim_ret(calculator* calc) {
  if (calc->call_stack_index <= 0) {
    puts("Call stack underflow!");
    exit(6);
  }

  calc->call_stack_index--;
}

void prim_branch(calculator* calc) {
  if (!eat_token(calc)) {
    puts("Expected offset after 'branch'!");
    exit(7);
  }

  int offset = 0;
  if (!read_int(calc->buffer, &offset)) {
    printf("Expected numeric offset after 'branch', got %s!", calc->buffer);
    exit(8);
  }

  write_branch(calc, offset);
}

void prim_0branch(calculator* calc) {
  if (!eat_token(calc)) {
    puts("Expected offset after '0branch'!");
    exit(7);
  }

  int offset = 0;
  if (!read_int(calc->buffer, &offset)) {
    printf("Expected numeric offset after '0branch', got %s!", calc->buffer);
    exit(8);
  }

  write_0branch(calc, offset);
}

void prim_immediate(calculator* calc) {
  if (calc->last == NULL) {
    puts("No user-defined functions to make immediate!");
    exit(9);
  }

  calc->last->immediate = !calc->last->immediate;
}

void prim_here(calculator* calc) {
  push_value(calc, calc->here);
}

void prim_write_branch(calculator* calc) {
  int offset = pop_value(calc);
  write_branch(calc, offset);
}

void prim_write_0branch(calculator* calc) {
  int offset = pop_value(calc);
  write_0branch(calc, offset);
}

void prim_modify_code(calculator* calc) {
  int new_value = pop_value(calc);
  int instr = pop_value(calc);
  calc->bytecode[instr].value = new_value;
}

void prim_swap(calculator* calc) {
  int x = pop_value(calc);
  int y = pop_value(calc);
  push_value(calc, x);
  push_value(calc, y);
}

const char* bytecode_kind_str(bytecode_kind bk) {
  switch (bk) {
    case LIT:
      return "LIT";
    case PRIM_CALL:
      return "PRIM_CALL";
    case USER_CALL:
      return "USER_CALL";
    case COND_BRANCH:
      return "COND_BRANCH";
    case BRANCH:
      return "BRANCH";
    default:
      return "<ERROR>";
  }
}

void dump_instr(int index, bytecode bc);

void prim_dump_bytecode(calculator* calc) {
  for (int i = 0; i < calc->here; i++) {
    dump_instr(i, calc->bytecode[i]);
  }
}

static primitive primitives[] = {
  { .name = "ret", .fn = prim_ret, .immediate = false }, // "ret" should be first!
  { .name = "dup", .fn = prim_dup, .immediate = false },
  { .name = "+", .fn = prim_add, .immediate = false },
  { .name = "-", .fn = prim_sub, .immediate = false },
  { .name = "*", .fn = prim_mul, .immediate = false },
  { .name = "<", .fn = prim_le, .immediate = false },
  { .name = "<=", .fn = prim_lt, .immediate = false },
  { .name = "print", .fn = prim_print, .immediate = false },
  { .name = ":", .fn = prim_colon, .immediate = false },
  { .name = ";", .fn = prim_semicolon, .immediate = true },
  { .name = "branch", .fn = prim_branch, .immediate = true },
  { .name = "0branch", .fn = prim_0branch, .immediate = true },
  { .name = "immediate", .fn = prim_immediate, .immediate = true },
  { .name = "here", .fn = prim_here, .immediate = false },
  { .name = "write-branch", .fn = prim_write_branch, .immediate = false },
  { .name = "write-0branch", .fn = prim_write_0branch, .immediate = false },
  { .name = "swap", .fn = prim_swap, .immediate = false },
  { .name = "modify-code", .fn = prim_modify_code, .immediate = false },
  { .name = "dump", .fn = prim_dump_bytecode, .immediate = false },
};

static const int num_primitives = sizeof(primitives) / sizeof(primitives[0]);

void dump_instr(int index, bytecode instr) {
  if (instr.kind == PRIM_CALL) {
    if (instr.value >= 0 && instr.value < num_primitives) {
      fprintf(stderr, "[%4d]: %4d %s '%s'\n", index, instr.value, bytecode_kind_str(instr.kind), primitives[instr.value].name);
    }
    else {
      fprintf(stderr, "[%4d]: %4d %s <UNKNOWN PRIMITIVE>\n", index, instr.value, bytecode_kind_str(instr.kind));
    }
  }
  else {
    fprintf(stderr, "[%4d]: %4d %s\n", index, instr.value, bytecode_kind_str(instr.kind));
  }
}

bool step(calculator* calc) {
  if (calc->call_stack_index == 0) {
    return false;
  }

  int ip = calc->call_stack[calc->call_stack_index - 1]++;
  bytecode instr = calc->bytecode[ip];
  switch (instr.kind) {
    case LIT:
      push_value(calc, instr.value);
      break;
    case PRIM_CALL: {
      int index = instr.value;
      if (index >= 0 && index < num_primitives) {
        primitives[index].fn(calc);
      }
      else {
        printf("Unexpected call to primitive with index %d!\n", index);
        exit(3);
      }
      break;
    case USER_CALL:
      calc->call_stack[calc->call_stack_index++] = instr.value;
      break;
    case BRANCH:
      calc->call_stack[calc->call_stack_index - 1] = instr.value;
      break;
    case COND_BRANCH: {
      int top = pop_value(calc);
      if (top == 0) {
        calc->call_stack[calc->call_stack_index - 1] = instr.value;
      }
      break;
    }
    default:
      printf("Unexpected bytecode kind %d!\n", instr.kind);
      exit(4);
    }
  }

  return true;
}


void process_token(calculator* calc) {
  char* buffer = calc->buffer;
  // printf("Processing token %s\n", buffer);

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
    user_function* user_fn = calc->last;

    while (user_fn != NULL) {
      if (strcmp(buffer, user_fn->name) == 0) {
        if (calc->interpreting || user_fn->immediate) {
          calc->call_stack[0] = user_fn->start;
          calc->call_stack_index = 1;
          run(calc);
        }
        else {
          write_user_call(calc, user_fn->start);
        }
        return;
      }
      user_fn = user_fn->next;
    }

    for (int i = 0; i < num_primitives; i++) {
      if (strcmp(buffer, primitives[i].name) == 0) {
        if (calc->interpreting || primitives[i].immediate) {
          // puts("Interpreting primitive");
          primitives[i].fn(calc);
        }
        else {
          // puts("Compiling primitive");
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
  calc->here = 0;
  calc->interpreting = true;
  calc->last = NULL;
  calc->eof = false;
}

bool eat_token(calculator* calc) {
  if (calc->eof) {
    return false;
  }

  calc->next = calc->buffer;
  bool scanning = true;
  while (scanning) {
    int c = getchar();
    if (c == EOF) {
      calc->eof = true;
      break;
    }

    switch (c) {
      case ' ':
      case '\n':
      case '\t':
      case '\r':
        if (calc->next != calc->buffer) { // Already have non-whitespace character in the buffer
          scanning = false;
        }
        break;
      default:
        *calc->next++ = c;
    }
  }

  *calc->next = '\0';
  return calc->next != calc->buffer;
}

int main() {
  char buffer[1024];
  char* next = buffer;
  calculator calc;
  init_calculator(&calc);

  while (!calc.eof) {
    if (eat_token(&calc)) {
      process_token(&calc);
    }
    else {
      break;
    }
  }
}
