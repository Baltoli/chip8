#include "interpreter.h"

#include <stdio.h>

int main(void)
{ 
  struct c8_interpreter *in = new_interpreter();
  in->cpu.memory[0x200] = 0x6081;
  in->cpu.memory[0x201] = 0x800E;

  run(in);
  dump_state(in);
  return 0;
}
