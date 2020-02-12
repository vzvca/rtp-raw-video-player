/*
  To test that the Linux framebuffer is set up correctly, and that the device permissions
  are correct, use the program below which opens the frame buffer and draws a gradient-
  filled red square:

  retrieved from:
  Testing the Linux Framebuffer for Qtopia Core (qt4-x11-4.2.2)

  http://cep.xor.aps.anl.gov/software/qt4-x11-4.2.2/qtopiacore-testingframebuffer.html
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <strings.h>

#include "vpl.h"

extern int g_width, g_height;

// --------------------------------------------------------------------------
//   Init du l'ecriture dans un fichier
// --------------------------------------------------------------------------
int initout( char *file, image_t *img )
{
  unsigned char block[4096];
  int i, ni, fbfd = -1;
  char *fbp = 0;
  
  // Open the file for reading and writing
  fbfd = open( file, O_RDWR);
  if (fbfd == -1) {
    perror("Error: cannot open output file");
    exit(1);
  }
  printf("The output file was opened successfully.\n");

  // fill image with 0
  bzero( block, sizeof(block));
  ni = (g_width * g_height * 4 + sizeof(block) - 1) / sizeof(block);
  for( i = 0; i < ni; ++i ) {
    if ( -1 == write( fbfd, block, sizeof(block)) ) {
      perror("Error: writing output file.");
      exit(2);
    }
  }

  // Map the device to memory
  fbp = (char *)mmap(0, ni * sizeof(block), PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
  if (*(int*)fbp == -1) {
    perror("Error: failed to map output file to memory");
    exit(4);
  }
  printf("The output file was mapped to memory successfully.\n");
  
  img->w = g_width;
  img->h = g_height;
  img->stride = g_width;
  img->pixels  = (uint32_t*) fbp;
    
  return fbfd;
}

// --------------------------------------------------------------------------
//   Init du framebuffer
// --------------------------------------------------------------------------
int initfb( char *fbdev, image_t *img )
{
  int fbfd = -1;
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  long int screensize = 0;
  char *fbp = 0;
  int x = 0, y = 0;
  long int location = 0;

  // Open the file for reading and writing
  fbfd = open( fbdev, O_RDWR);
  if (fbfd == -1) {
    perror("Error: cannot open framebuffer device");
    exit(1);
  }
  printf("The framebuffer device was opened successfully.\n");

  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
    perror("Error reading fixed information");
    exit(2);
  }

  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
    perror("Error reading variable information");
    exit(3);
  }

  printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

  // le code video ne supporte que le 32bpp
  if ( vinfo.bits_per_pixel != 32 ) {
    fprintf( stderr, "Only 32bpp framebuffers are supported. Giving up...\n");
    exit(3);
  }
    
  // Figure out the size of the screen in bytes
  screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

  // Map the device to memory
  fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
  if (*(int*)fbp == -1) {
    perror("Error: failed to map framebuffer device to memory");
    exit(4);
  }
  printf("The framebuffer device was mapped to memory successfully.\n");

  img->w = vinfo.yres;
  img->h = vinfo.xres;
  img->stride = finfo.line_length / (vinfo.bits_per_pixel/8); 
  // attention au décalage des pixels
  img->pixels  = (uint32_t*) fbp;
  img->pixels += vinfo.yoffset*finfo.line_length/(vinfo.bits_per_pixel/8) + vinfo.xoffset;
    
  return fbfd;
}
