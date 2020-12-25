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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <SDL2/SDL.h>

#define WIDTH 720
#define HEIGHT 576
#define FPS 10
#define WINX 100
#define WINY 100

int g_x = WINX;
int g_y = WINY;
int g_width = WIDTH;
int g_height = HEIGHT;
int g_fps = FPS;
char *g_input = NULL;


int SDLCALL
ThreadFunc(void *data)
{

  // variable declarations
  SDL_Window *win = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Texture *img = NULL;
  char *pixels = NULL;
  int fbfd;
  
  // create the window and renderer
  // note that the renderer is accelerated
  puts("create window");
  win = SDL_CreateWindow("video capture", g_x, g_y, g_width, g_height, 0);

  puts("create renderer");
  renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

  puts("create texture");
  img = SDL_CreateTexture(renderer,
			  SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING,
			  g_width, g_height);
  // Display window
  SDL_ShowWindow(win);

  puts("mmap video source");
  fbfd = open( g_input, O_RDWR );
  if (fbfd == -1) {
    perror("Error: cannot open output file");
    exit(1);
  }
  puts("The output file was opened successfully.");
  pixels = (char *)mmap(0, g_width*g_height*4, PROT_READ, MAP_SHARED, fbfd, 0);
  if (*(int*)pixels == -1) {
    perror("Error: failed to map output file to memory");
    exit(1);
  }
  puts("The output file was mapped to memory successfully.\n");
  
  
  // main loop
  puts("entering main loop");
  while (1) {
    // event handling
    SDL_Event e;
    if ( SDL_WaitEventTimeout(&e,1000/g_fps) ) {
      if (e.type == SDL_QUIT)
	break;
      else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
	break;
    } 
		
    // clear the screen
    SDL_RenderClear(renderer);

    // update video frame
    SDL_UpdateTexture( img, NULL, pixels, g_width*4 );
    
    // copy the texture to the rendering context
    SDL_RenderCopy(renderer, img, NULL, NULL);
    
    // flip the backbuffer
    // this means that everything that we prepared behind the screens is actually shown
    SDL_RenderPresent(renderer);
  }
	
  SDL_DestroyTexture(img);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(win);
  munmap( pixels, g_width*g_height*4 );
  close(fbfd);
  
  return 0;
}

/*
 * --------------------------------------------------------------------------
 *   Usage
 * --------------------------------------------------------------------------
 */
void usage( int argc, char *argv[], int optind )
{
  char *what = (optind > 0) ? "error" : "usage";
  fprintf( stderr, "%s %s :\n%s [-?] -i file [-f fps] [-x x] [-y y] [-w width] [-h height]\n",
	   argv[0], what, argv[0]);
  
  fprintf( stderr, "\t-?\t\tPrints this message.\n");
  fprintf( stderr, "\t-i\t\tSets input video frame file.\n");
  fprintf( stderr, "\t-f\t\tSets the video framerate (default %d).\n", FPS);
  fprintf( stderr, "\t-w\t\tSets the width of the image (default %d).\n", WIDTH);
  fprintf( stderr, "\t-h\t\tSets the height of the image (default %d).\n", HEIGHT);
  fprintf( stderr, "\t-x\t\tSets the x position of the window (default %d).\n", WINX);
  fprintf( stderr, "\t-y\t\tSets the y position of the window (default %d).\n", WINY);
  /* exit with error only if option parsng failed */
  exit(optind > 0);
}

// --------------------------------------------------------------------------
//   Programme principal
// --------------------------------------------------------------------------
int main (int argc, char *argv[])
{
  SDL_Thread *thread;
  int opt;
  
  while ( (opt = getopt( argc, argv, "?i:f:w:h:x:y:")) != -1 ) {
    switch( opt ) {
    case '?':  usage( argc, argv, 0); break;
    case 'i':  g_input = optarg; break;
    case 'w':  g_width = atoi(optarg); break;
    case 'h':  g_height = atoi(optarg); break;
    case 'f':  g_fps = atoi(optarg); break;
    case 'x':  g_x = atoi(optarg); break;
    case 'y':  g_y = atoi(optarg); break;
    default:
      usage(argc, argv, optind);
    }
  }
  
  
  // Initialize SDL.
  puts("init sdl");
  if (SDL_Init(0) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Init failed: %s\n", SDL_GetError());
    return 1;
  }
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Init failed: %s\n", SDL_GetError());
    return 1;
  }

  thread = SDL_CreateThread(ThreadFunc, "One", "#1");
  if (thread == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create thread: %s\n", SDL_GetError());
    exit(1);
  }
  SDL_Log("Waiting for thread #1\n");
  SDL_WaitThread(thread, NULL);
}
