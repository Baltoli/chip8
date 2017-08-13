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
  atomic_init(&(i->cpu.delay), 0);
  atomic_init(&(i->cpu.sound), 0);

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

  else if(opcode(i) == 0xA) {
    interp->cpu.vi = addr(i);
  }

  else if(opcode(i) == 0xB) {
    // TODO: relative jump
  }

  else if(opcode(i) == 0xC) {
    // TODO: random byte
  }

  else if(opcode(i) == 0xD) {
    draw(interp, x(i), y(i), nibble(i));
  }

  else if(opcode(i) == 0xE) {
    if(byte(i) == 0x9E) {
      if(interp->keyboard.pressed(x(i))) { interp->cpu.pc += 2; }
    } else if(byte(i) == 0xA1) {
      if(!interp->keyboard.pressed(x(i))) { interp->cpu.pc += 2; }
    } else {
      goto fail;
    }
  }

  else if(opcode(i) == 0xF) {
    interact(interp, x(i), byte(i));
  }

  else {
fail:
    printf("Unhandled instruction at 0x%03X: 0x%04X\n", interp->cpu.pc, i.op);
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

void step(struct c8_interpreter *interp)
{
  uint16_t op = 0;
  op |= ((uint16_t)(interp->cpu.memory[interp->cpu.pc])) << 8;
  op |= (uint16_t)(interp->cpu.memory[interp->cpu.pc + 1]);

  struct c8_instruction i = { .op = op };
  dispatch(interp, i);
  interp->cpu.pc += 2;
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
    printf("Stack underflow at 0x%03X\n", interp->cpu.pc);
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
    printf("Stack overflow at 0x%03X\n", interp->cpu.pc);
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

void draw(struct c8_interpreter *interp, uint8_t xr, uint8_t yr, uint8_t size)
{
  uint8_t xp = interp->cpu.registers[xr];
  uint8_t yp = interp->cpu.registers[yr];

  for(int y = 0; y < size; ++y) {
    uint8_t row = interp->cpu.memory[interp->cpu.vi + y];
    for(int x = 0; x < 8; ++x) {
      interp->display.buffer[(yp+y)*64 + (xp+x)] ^= (row & (1 << (7 - x))) >> (7 - x);
    }
  }
}

void interact(struct c8_interpreter *interp, uint8_t x, uint8_t code)
{
  switch(code) {
    case 0x07:
      interp->cpu.registers[x] = interp->cpu.delay;
      break;
    case 0x0A:
      interp->cpu.registers[x] = interp->keyboard.wait();
      break;
    case 0x15:
      interp->cpu.delay = interp->cpu.registers[x];
      break;
    case 0x18:
      interp->cpu.sound = interp->cpu.registers[x];
      break;
    case 0x1E:
      interp->cpu.vi += interp->cpu.registers[x];
      break;
    case 0x29:
      // TODO: interpreter sprite data
      break;
    case 0x33:
      interp->cpu.memory[interp->cpu.vi] = interp->cpu.registers[x] / 100;
      interp->cpu.memory[interp->cpu.vi+1] = (interp->cpu.registers[x] / 10) % 10;
      interp->cpu.memory[interp->cpu.vi+2] = interp->cpu.registers[x] % 10;
      break;
    case 0x55:
      for(int i = 0; i < x; ++i) {
        interp->cpu.memory[interp->cpu.vi+i] = interp->cpu.registers[i];
      }
      break;
    case 0x65:
      for(int i = 0; i < x; ++i) {
        interp->cpu.registers[i] = interp->cpu.memory[interp->cpu.vi+i];
      }
      break;
    default:
      printf("Unregonised F-series operation: 0x%02X\n", code);
      interp->running = false;
      break;
  }
}

void dump_state(struct c8_interpreter *interp, bool mem)
{
  printf("Interpreter state:\n");
  printf("\tRunning: %s\n", interp->running ? "yes" : "no");
  printf("\tRegisters:\n");
  for(int i = 0; i < 16; ++i) {
    if(i % 4 == 0) { printf("\t\t"); }
    printf("V%1X: 0x%02X\t", i, interp->cpu.registers[i]);
    if(i % 4 == 3) { printf("\n"); }
  }
  printf("\t\tVI: 0x%03X\tDelay: 0x%02X\n", interp->cpu.vi, interp->cpu.delay);
  printf("\t\tPC: 0x%03X\tSound: 0x%02X\n", interp->cpu.pc, interp->cpu.sound);
  printf("\t\tSP: %d\n", interp->cpu.sp);

  if(mem) {
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
}
