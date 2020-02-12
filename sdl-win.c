/*
 * MIT License
 *
 * Copyright (c) 2020 vzvca
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WIDTH 720
#define HEIGHT 576
#define IMG_PATH "deco-salon.png"

int SDLCALL
ThreadFunc(void *data)
{

  // variable declarations
  SDL_Window *win = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Texture *img = NULL;
  int w, h; // texture width & height
	
	
  // create the window and renderer
  // note that the renderer is accelerated
  puts("create window");
  win = SDL_CreateWindow("video capture", 100, 100, WIDTH, HEIGHT, 0);

  puts("create renderer");
  renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	
  // load our image
  //  img = IMG_LoadTexture(renderer, IMG_PATH);
  //  SDL_QueryTexture(img, NULL, NULL, &w, &h); // get the width and height of the texture
  // put the location where we want the texture to be drawn into a rectangle
  // I'm also scaling the texture 2x simply by setting the width and height
  //  SDL_Rect texr; texr.x = WIDTH/2; texr.y = HEIGHT/2; texr.w = w*2; texr.h = h*2; 

  puts("entering main loop");
  
  
  // main loop
  while (1) {
    puts("looping");
		
    // event handling
    SDL_Event e;
    if ( SDL_WaitEventTimeout(&e,1000) ) {
      if (e.type == SDL_QUIT)
	break;
      else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
	break;
    } 
		
    // clear the screen
    SDL_RenderClear(renderer);
    // copy the texture to the rendering context
    //    SDL_RenderCopy(renderer, img, NULL, &texr);
    // flip the backbuffer
    // this means that everything that we prepared behind the screens is actually shown
    SDL_RenderPresent(renderer);
    SDL_ShowWindow(win);
		
  }
	
  //SDL_DestroyTexture(img);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(win);
	
  return 0;
}

int main (int argc, char *argv[])
{
  SDL_Thread *thread;

  puts("init sdl");
  
  // Initialize SDL.
  //  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
  if (SDL_Init(0) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Init failed: %s\n", SDL_GetError());
    return 1;
  }
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Init failed: %s\n", SDL_GetError());
    return 1;
  }

  puts("init done");
  
  thread = SDL_CreateThread(ThreadFunc, "One", "#1");
  if (thread == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create thread: %s\n", SDL_GetError());
    exit(1);
  }
  SDL_Log("Waiting for thread #1\n");
  SDL_WaitThread(thread, NULL);
}
