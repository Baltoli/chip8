#include "interpreter.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH (64*5)
#define SCREEN_HEIGHT (32*5)

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

      bool quit = false;
      SDL_Event e;
      SDL_Renderer* gRenderer = SDL_GetRenderer(window);
      if(!gRenderer) {
        printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        return 1;
      }

      while(!quit) {
        while(SDL_PollEvent( &e ) != 0) {
          if( e.type == SDL_QUIT ) {
            quit = true;
          }
        }

        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( gRenderer );

        for(int y = 0; y < 32; ++y) {
          for(int x = 0; x < 64; ++x) {
            SDL_Rect fillRect = { x*5, y*5, 5, 5 };
            SDL_SetRenderDrawColor( gRenderer, (x+y)%2==0?0xFF:0x0, (x+y)%2==0?0xFF:0x0, (x+y)%2==0?0xFF:0x0, 0xFF );        
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
