#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdbool.h>
#include <stdint.h>

struct c8_cpu {
  uint16_t memory[4096];
  uint8_t registers[16];
  uint16_t vi;
  uint8_t delay;
  uint8_t sound;
  uint16_t pc;
  uint8_t sp;
  uint16_t stack[16];
};

struct c8_display {
  bool buffer[64*32];
};

struct c8_keyboard {
  bool keys[16];
};

struct c8_interpreter {
  struct c8_cpu cpu;
  struct c8_display display;
  struct c8_keyboard keyboard;
  bool running;
};

struct c8_instruction {
  uint16_t op;
};

struct c8_interpreter *new_interpreter();

uint8_t byte(struct c8_instruction i);
uint8_t nibble(struct c8_instruction i);
uint16_t addr(struct c8_instruction i);
uint8_t x(struct c8_instruction i);
uint8_t y(struct c8_instruction i);
uint8_t opcode(struct c8_instruction i);

void dispatch(struct c8_interpreter *interp, struct c8_instruction i);
void run(struct c8_interpreter *interp);

void cls(struct c8_interpreter *interp);
void ret(struct c8_interpreter *interp);
void jump(struct c8_interpreter *interp, uint16_t addr);
void call(struct c8_interpreter *interp, uint16_t addr);
void se_direct(struct c8_interpreter *interp, uint8_t reg, uint8_t byte);
void sne_direct(struct c8_interpreter *interp, uint8_t reg, uint8_t byte);
void sne_indirect(struct c8_interpreter *interp, uint8_t r1, uint8_t r2);
void se_indirect(struct c8_interpreter *interp, uint8_t r1, uint8_t r2);
void load(struct c8_interpreter *interp, uint8_t reg, uint8_t byte);
void add(struct c8_interpreter *interp, uint8_t reg, uint8_t byte);
void alu(struct c8_interpreter *interp, uint8_t rx, uint8_t ry, uint8_t f);

void dump_state(struct c8_interpreter *interp);

#endif
