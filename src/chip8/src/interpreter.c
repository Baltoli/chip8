#include "interpreter.h"

#include <stddef.h>
#include <stdio.h>

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

  if(opcode(i) == 0x3) {
    se_direct(interp, x(i), byte(i));
    return;
  }

  if(opcode(i) == 0x4) {
    sne_direct(interp, x(i), byte(i));
    return;
  }

  if(opcode(i) == 0x5 && nibble(i) == 0) {
    se_indirect(interp, x(i), y(i));
    return;
  }

  if(opcode(i) == 0x6) {
    load(interp, x(i), byte(i));
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

void se_direct(struct c8_interpreter *interp, uint8_t reg, uint8_t byte)
{
  if(interp->cpu.registers[reg] == byte) {
    interp->cpu.pc += 2;
  }
}

void sne_direct(struct c8_interpreter *interp, uint8_t reg, uint8_t byte)
{
  if(interp->cpu.registers[reg] != byte) {
    interp->cpu.pc += 2;
  }
}

void se_indirect(struct c8_interpreter *interp, uint8_t r1, uint8_t r2)
{
  if(interp->cpu.registers[r1] == interp->cpu.registers[r2]) {
    interp->cpu.pc += 2;
  }
}

void load(struct c8_interpreter *interp, uint8_t reg, uint8_t byte)
{
  interp->cpu.registers[reg] = byte;
}

void dump_state(struct c8_interpreter *interp)
{
  printf("Interpreter state:\n");
  printf("\tRunning: %s\n", interp->running ? "yes" : "no");
  printf("\tRegisters:\n");
  for(int i = 0; i < 16; ++i) {
    if(i % 4 == 0) { printf("\t\t"); }
    printf("V%1X: 0x%02X\t", i, interp->cpu.registers[i]);
    if(i % 4 == 3) { printf("\n"); }
  }
  printf("\t\tVI: 0x%04X\n", interp->cpu.vi);
  printf("\t\tPC: 0x%04X\n", interp->cpu.pc);
  printf("\t\tSP: %d\n", interp->cpu.sp);
}
