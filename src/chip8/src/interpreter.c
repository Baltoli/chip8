#include "interpreter.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

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

struct c8_interpreter *new_interpreter()
{
  struct c8_interpreter *i = malloc(sizeof(*i));
  bzero(i, sizeof(*i));

  i->cpu.pc = 0x200;

  return i;
}

void dispatch(struct c8_interpreter *interp, struct c8_instruction i)
{
  if(opcode(i) == 0x0) {
    if(y(i) == 0xE && nibble(i) == 0x0) {
      cls(interp);
    } else if(y(i) == 0xE && nibble(i) == 0xE) {
      ret(interp);
    } else {
      goto fail;
    }
  }

  else if(opcode(i) == 0x1) {
    jump(interp, addr(i));
  }

  else if(opcode(i) == 0x2) {
    call(interp, addr(i));
  }

  else if(opcode(i) == 0x3) {
    se_direct(interp, x(i), byte(i));
  }

  else if(opcode(i) == 0x4) {
    sne_direct(interp, x(i), byte(i));
  }

  else if(opcode(i) == 0x5 && nibble(i) == 0) {
    se_indirect(interp, x(i), y(i));
  }

  else if(opcode(i) == 0x6) {
    load(interp, x(i), byte(i));
  }

  else {
fail:
    printf("Unhandled instruction at 0x%04X: 0x%04X\n", interp->cpu.pc, i.op);
    interp->running = false;
  }
}

void run(struct c8_interpreter *interp)
{
  interp->running = true;

  while(interp->running) {
    struct c8_instruction i = { .op = interp->cpu.memory[interp->cpu.pc] };
    dispatch(interp, i);
    interp->cpu.pc++;
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
  if(interp->cpu.sp == 0x0) {
    printf("Stack underflow at 0x%04X\n", interp->cpu.pc);
    interp->running = false;
    return;
  }

  interp->cpu.pc = interp->cpu.stack[interp->cpu.sp];
  interp->cpu.sp--;
}

void jump(struct c8_interpreter *interp, uint16_t addr)
{
  interp->cpu.pc = addr - 1;
}

void call(struct c8_interpreter *interp, uint16_t addr)
{
  if(interp->cpu.sp == 0xF) {
    printf("Stack overflow at 0x%04X\n", interp->cpu.pc);
    interp->running = false;
    return;
  }

  interp->cpu.sp++;
  interp->cpu.stack[interp->cpu.sp] = interp->cpu.pc;
  interp->cpu.pc = addr - 1;
}

void se_direct(struct c8_interpreter *interp, uint8_t reg, uint8_t byte)
{
  if(interp->cpu.registers[reg] == byte) {
    interp->cpu.pc += 1;
  }
}

void sne_direct(struct c8_interpreter *interp, uint8_t reg, uint8_t byte)
{
  if(interp->cpu.registers[reg] != byte) {
    interp->cpu.pc += 1;
  }
}

void se_indirect(struct c8_interpreter *interp, uint8_t r1, uint8_t r2)
{
  if(interp->cpu.registers[r1] == interp->cpu.registers[r2]) {
    interp->cpu.pc += 1;
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
