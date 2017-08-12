#include "interpreter.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH (64*5)
#define SCREEN_HEIGHT (32*5)

int main(int argc, char **argv)
{ 
  struct c8_interpreter *in = new_interpreter();

  in->cpu.memory[0x200] = 0x12;
  in->cpu.memory[0x201] = 0x00;

  in->display.buffer[0*64 + 0] = 1;
  in->display.buffer[1*64 + 1] = 1;
  in->display.buffer[2*64 + 2] = 1;

  in->running = true;

  SDL_Window* window = NULL;

  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
  } else {
    window = SDL_CreateWindow(
      "SDL Tutorial", 
      SDL_WINDOWPOS_UNDEFINED, 
      SDL_WINDOWPOS_UNDEFINED, 
      SCREEN_WIDTH, 
      SCREEN_HEIGHT, 
      SDL_WINDOW_SHOWN
    );

    if(window == NULL) {
      printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
    } else {
      bool quit = false;
      SDL_Event e;
      SDL_Renderer* gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
      if(!gRenderer) {
        printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        return 1;
      }

      while(!quit && in->running) {
        step(in);

        while(SDL_PollEvent( &e ) != 0) {
          if( e.type == SDL_QUIT ) {
            quit = true;
          }
        }

        for(int y = 0; y < 32; ++y) {
          for(int x = 0; x < 64; ++x) {
            int colour = in->display.buffer[y*64 + x] ? 0xFF : 0x00;

            SDL_Rect fillRect = { x*5, y*5, 5, 5 };
            SDL_SetRenderDrawColor( gRenderer, colour, colour, colour, 0xFF );
            SDL_RenderFillRect( gRenderer, &fillRect );
          }
        }

        SDL_RenderPresent( gRenderer );
      }
    }
  }
  
  SDL_DestroyWindow( window );
  SDL_Quit();
  
  return 0;
}
