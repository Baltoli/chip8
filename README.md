# Chip8 Implementation

## To-Do list:

* Make noise
* Implement keyboard methods on interpreter
  * Waiting for a keypress needs a better design than just blocking. Global
    field that gets set when waiting? Only execute next instruction when a
    keypress comes in?
  * Up / down checks are easier
* Pseudo-assembler to make programs easier to write
* Collision detection
* Clean up and document code
* 0nnn instructions are a no-op
