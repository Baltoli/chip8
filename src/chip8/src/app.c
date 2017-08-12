#include "interpreter.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int main(int argc, char **argv)
{ 
  struct c8_interpreter *in = new_interpreter();

  in->cpu.memory[0x200] = 0x60;
  in->cpu.memory[0x201] = 0xFF;

  run(in);
  dump_state(in);

  SDL_Window* window = NULL;
  SDL_Surface* screenSurface = NULL;

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
      //Get window surface
      screenSurface = SDL_GetWindowSurface( window );

      //Fill the surface white
      SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
      
      //Update the surface
      SDL_UpdateWindowSurface( window );

      //Wait two seconds
      for(int i = 0; i < 2000; i++){
        SDL_PumpEvents();
        SDL_Delay(1);
      }
    }
  }
  
  SDL_DestroyWindow( window );
  SDL_Quit();
  
  return 0;
}
