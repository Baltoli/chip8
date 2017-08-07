#include "interpreter.h"

#include <stddef.h>

inline uint8_t byte(struct c8_instruction i)
{
  return i.op & 0x00FF;
}

inline uint8_t nibble(struct c8_instruction i)
{
  return i.op & 0x000F;
}

inline uint16_t addr(struct c8_instruction i)
{
  return i.op & 0x0FFF;
}

inline uint8_t x(struct c8_instruction i)
{
  return (i.op & 0x0F00) >> 8;
}

inline uint8_t y(struct c8_instruction i)
{
  return (i.op & 0x00F0) >> 4;
}

inline uint8_t opcode(struct c8_instruction i)
{
  return (i.op & 0xF000) >> 12;
}

void dispatch(struct c8_interpreter *interp, struct c8_instruction i)
{
  if(opcode(i) == 0x0) {
    if(y(i) == 0xE && nibble(i) == 0x0) {
      cls(interp);
    } else if(y(i) == 0xE && nibble(i) == 0xE) {
      ret(interp);
    }

    return;
  }

  if(opcode(i) == 0x1) {
    jump(interp, addr(i));
    return;
  }

  if(opcode(i) == 0x2) {
    call(interp, addr(i));
    return;
  }
}

void cls(struct c8_interpreter *interp)
{
  for(size_t i = 0; i < 64*32; ++i) {
    interp->display.buffer[i] = false;
  }
}

void ret(struct c8_interpreter *interp)
{
  interp->cpu.pc = interp->cpu.stack[interp->cpu.sp];
  interp->cpu.sp--;
}

void jump(struct c8_interpreter *interp, uint16_t addr)
{
  interp->cpu.pc = addr;
}

void call(struct c8_interpreter *interp, uint16_t addr)
{
  interp->cpu.sp++;
  interp->cpu.stack[interp->cpu.sp] = addr;
}
