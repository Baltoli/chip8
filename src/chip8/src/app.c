#include "interpreter.h"

#include <stdio.h>

int main(int argc, char **argv)
{ 
  struct c8_interpreter *in = new_interpreter();
  in->cpu.memory[0x200] = 0x6008;
  in->cpu.memory[0x201] = 0x8006;
  in->cpu.memory[0x202] = 0x8006;
  in->cpu.memory[0x203] = 0x8006;
  in->cpu.memory[0x204] = 0x8006;

  run(in);
  dump_state(in);
  return 0;
}
