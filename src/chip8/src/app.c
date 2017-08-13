#include "interpreter.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define SCREEN_SCALE 5
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

static SDL_Window *window;

void init_graphics(void)
{
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    exit(1);
  } else {
    window = SDL_CreateWindow(
      "Chip8", 
      SDL_WINDOWPOS_UNDEFINED, 
      SDL_WINDOWPOS_UNDEFINED, 
      SCREEN_WIDTH * SCREEN_SCALE,
      SCREEN_HEIGHT * SCREEN_SCALE, 
      SDL_WINDOW_SHOWN
    );

    if(window == NULL) {
      fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
      exit(1);
    }
  }
}

void emulator_loop(struct c8_interpreter *in)
{
  bool quit = false;
  SDL_Event e;
  SDL_Renderer* gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if(!gRenderer) {
    printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
    exit(1);
  }

  uint32_t last_draw = SDL_GetTicks();

  while(!quit && in->running) {
    step(in);

    while(SDL_PollEvent( &e ) != 0) {
      if( e.type == SDL_QUIT ) {
        quit = true;
      }
    }

    if(SDL_GetTicks() - last_draw >= 16) {
      for(int y = 0; y < SCREEN_HEIGHT; ++y) {
        for(int x = 0; x < SCREEN_WIDTH; ++x) {
          int colour = in->display.buffer[y*SCREEN_WIDTH + x] ? 0xFF : 0x00;

          SDL_Rect fillRect = { x*SCREEN_SCALE, y*SCREEN_SCALE, SCREEN_SCALE, SCREEN_SCALE };
          SDL_SetRenderDrawColor( gRenderer, colour, colour, colour, 0xFF );
          SDL_RenderFillRect( gRenderer, &fillRect );
        }
      }
      last_draw = SDL_GetTicks();
    }

    SDL_RenderPresent( gRenderer );
  }
}

uint8_t *rom_data(int argc, char **argv)
{
  if(argc != 2) {
    return NULL;
  }

  FILE *file = fopen(argv[1], "r");
  if(!file) {
    return NULL;
  }

  struct stat st;
  stat(argv[1], &st);
  if(st.st_size != 4096) {
    return NULL;
  }

  uint8_t *data = malloc(4096);
  if(!data) {
    return NULL;
  }

  fread(data, 1, 4096, file);
  return data;
}

int main(int argc, char **argv)
{ 
  uint8_t *rom = rom_data(argc, argv);
  if(!rom) {
    fprintf(stderr, "Couldn't load ROM!\n");
    return 1;
  }

  init_graphics();
  struct c8_interpreter *in = new_interpreter();

  memcpy(in->cpu.memory, rom, 4096);

  in->running = true;
  
  emulator_loop(in);
  dump_state(in);

  SDL_DestroyWindow( window );
  SDL_Quit();
  
  return 0;
}
