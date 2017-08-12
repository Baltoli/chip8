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

  else if(opcode(i) == 0x7) {
    add(interp, x(i), byte(i));
  }

  else if(opcode(i) == 0x8) {
    alu(interp, x(i), y(i), nibble(i));
  }

  else if(opcode(i) == 0x9 && nibble(i) == 0) {
    sne_indirect(interp, x(i), y(i));
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
  interp->cpu.pc = (0x200 - 2);

  while(interp->running) {
    interp->cpu.pc += 2;

    uint16_t op = 0;
    op |= ((uint16_t)(interp->cpu.memory[interp->cpu.pc])) << 8;
    op |= (uint16_t)(interp->cpu.memory[interp->cpu.pc + 1]);

    struct c8_instruction i = { .op = op };
    dispatch(interp, i);
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
  interp->cpu.pc = addr - 2;
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
  interp->cpu.pc = addr - 2;
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

void sne_indirect(struct c8_interpreter *interp, uint8_t r1, uint8_t r2)
{
  if(interp->cpu.registers[r1] != interp->cpu.registers[r2]) {
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

void add(struct c8_interpreter *interp, uint8_t reg, uint8_t byte)
{
  interp->cpu.registers[reg] += byte;
}

void alu(struct c8_interpreter *interp, uint8_t rx, uint8_t ry, uint8_t f)
{
  switch(f) {
    case 0x0:
      interp->cpu.registers[rx] = interp->cpu.registers[ry];
      break;
    case 0x1:
      interp->cpu.registers[rx] |= interp->cpu.registers[ry];
      break;
    case 0x2:
      interp->cpu.registers[rx] &= interp->cpu.registers[ry];
      break;
    case 0x3:
      interp->cpu.registers[rx] ^= interp->cpu.registers[ry];
      break;
    case 0x4: {
      uint16_t x = interp->cpu.registers[rx];
      uint16_t y = interp->cpu.registers[ry];
      interp->cpu.registers[0xF] = (x + y > 255) ? 1 : 0;
      interp->cpu.registers[rx] = (uint8_t)((x + y) & 0xFF);
      break;
              }
    case 0x5: {
      uint8_t x = interp->cpu.registers[rx];
      uint8_t y = interp->cpu.registers[ry];
      interp->cpu.registers[0xF] = (x > y) ? 1 : 0;
      interp->cpu.registers[rx] = (uint8_t)(x - y);
      break;
              }
    case 0x6: {
      uint8_t x = interp->cpu.registers[rx];
      interp->cpu.registers[0xF] = (x % 2 == 1) ? 1 : 0;
      interp->cpu.registers[rx] >>= 1;
      break;
              }
    case 0x7: {
      uint8_t x = interp->cpu.registers[rx];
      uint8_t y = interp->cpu.registers[ry];
      interp->cpu.registers[0xF] = (y > x) ? 1 : 0;
      interp->cpu.registers[rx] = (uint8_t)(y - x);
      break;
              }
    case 0xE: {
      uint8_t x = interp->cpu.registers[rx];
      interp->cpu.registers[0xF] = (x & 0x80) ? 1 : 0;
      interp->cpu.registers[rx] <<= 1;
      break;
              }
    default:
      printf("Unrecognised ALU operation: 0x%1X\n", f);
      interp->running = false;
      break;
  }
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

  printf("\tMemory:\n+\t");
  for(int i = 0; i < 32; ++i) {
    printf("%02X ", i);
  }
  printf("\n");

  for(int i = 0; i < 4096; ++i) {
    if(i % 32 == 0) { printf("0x%03X\t", i); }
    printf("%02X ", interp->cpu.memory[i]);
    if(i % 32 == 31) { printf("\n"); }
  }
}
