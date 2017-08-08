#include "interpreter.h"

#include <stdio.h>

int main(int argc, char **argv)
{ 
  struct c8_interpreter *in = new_interpreter();
  in->cpu.memory[0x200] = 0x2300;
  in->cpu.memory[0x201] = 0x1201;

  in->cpu.memory[0x300] = 0x60FF;
  in->cpu.memory[0x301] = 0x61FE;
  in->cpu.memory[0x302] = 0x62FD;
  in->cpu.memory[0x303] = 0x2300;
  in->cpu.memory[0x304] = 0x00EE;

  run(in);
  dump_state(in);
  return 0;
}
