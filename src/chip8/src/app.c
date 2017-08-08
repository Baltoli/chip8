#include "interpreter.h"

#include <stdio.h>

int main(int argc, char **argv)
{ 
  struct c8_interpreter in = { 0 };
  in.cpu.pc = 0x600;
  dump_state(&in);
  struct c8_instruction o = { .op = 0x6045 };
  dispatch(&in, o);
  dump_state(&in);
  return 0;
}
