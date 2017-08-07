#include "interpreter.h"

#include <stdio.h>

int main(int argc, char **argv)
{ 
  struct c8_instruction i = { .op = 0x1234 };
  printf("0x%02x\n", byte(i));
  printf("0x%01x\n", nibble(i));
  printf("0x%01x\n", x(i));
  printf("0x%01x\n", y(i));
  printf("0x%03x\n", addr(i));
  return 0;
}
