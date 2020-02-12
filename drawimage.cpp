// drawimage.cpp
// by Glenn G. Chappell
// October 2003
//
// For CS 381
// Example of Raster Image Drawing & Zooming

#include <iostream>
using std::cerr;
using std::endl;
#include <string>
using std::string;
#include <sstream>
using std::ostringstream;
#include <stdlib.h>  // Some versions of MS-Vis C++ have broken <cstdlib>
//using std::exit;
#include <GL/glut.h> // GLUT stuff - includes OpenGL headers as well


// Global variables
// Window/viewport
const int startwinsize = 400; // Starting window width & height, in pixels

// Keyboard
const int ESCKEY = 27;        // ASCII value of escape character

// For image
const int img_width = 100;
const int img_height = 100;
GLubyte the_image[img_height][img_width][3];
                              // The image itself
                              // 3rd subscript 0 = R, 1 = G, 2 = B

double zoom = 1.0;            // Pixel zoom, for both x & y
const double maxzoom = 10.0;
const double minzoom = -10.0;


// printbitmap
// Prints the given string at the given raster position
//  using GLUT bitmap fonts.
// You probably don't want any rotations in the model/view
//  transformation when calling this function.
void printbitmap(const string msg, double x, double y)
{
   glRasterPos2d(x, y);
   for (string::const_iterator ii = msg.begin();
        ii != msg.end();
        ++ii)
   {
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *ii);
   }
}


// display
// The GLUT display function
void display()
{
   glClear(GL_COLOR_BUFFER_BIT);

   // Draw image
   glRasterPos2d(0.0, 0.0);
   glPixelZoom(zoom, zoom);
   glDrawPixels(img_width, img_height,
                GL_RGB, GL_UNSIGNED_BYTE,
                the_image);

   // Draw instructions
   glColor3d(0.0, 0.0, 0.0);
   printbitmap("<- -> to change pixel zoom.", -0.9, 0.9);
   printbitmap("SPACE resets zoom to 1.", -0.9, 0.8);
   ostringstream num;
   num << "Zoom: " << zoom;
   printbitmap(num.str(), -0.9, 0.7); 

   glutSwapBuffers();
}


// reshape
// The GLUT reshape function
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // We set up coordinate system so that aspect ratios are always correct,
    //  and the region from -1..1 in x & y always just fits in the viewport.
    if (w > h)
    {
       gluOrtho2D(-double(w)/h, double(w)/h, -1., 1.);
    }
    else
    {
       gluOrtho2D( -1., 1., -double(h)/w, double(h)/w);
    }

    glMatrixMode(GL_MODELVIEW);  // Always go back to modelview mode
}


// keyboard
// The GLUT keyboard function
void keyboard(unsigned char key, int x, int y)
{
   switch (key)
   {
   case ESCKEY:  // ESC: Quit
      exit(0);
      break;
   case ' ':     // Space: reset zoom
      zoom = 1.0;
      glutPostRedisplay();
      break;
   }
}


// special
// The GLUT special function
void special(int key, int x, int y)
{
   switch(key)
   {
   case GLUT_KEY_RIGHT: // Zoom goes up
      zoom += 0.25;
      if (zoom > maxzoom) zoom = maxzoom;
      glutPostRedisplay();
      break;
   case GLUT_KEY_LEFT:  // Zoom goes down
      zoom -= 0.25;
      if (zoom < minzoom) zoom = minzoom;
      glutPostRedisplay();
      break;
   }
}


// idle
// The GLUT idle function
void idle()
{
   // Print OpenGL errors, if there are any (for debugging)
   if (GLenum err = glGetError())
   {
      cerr << "OpenGL ERROR: " << gluErrorString(err) << endl;
   }
}


// makeimage
// Make image in the_image
void makeimage()
{
   for (int i=0; i<img_width; ++i)
   {
      double x = double(i)/(img_width-1);
      for (int j=0; j<img_height; ++j)
      {
         double y = double(j)/(img_height-1);

         the_image[j][i][0] = int(x*255)*15 % 256;
         the_image[j][i][1] = int(y*255)*15 % 256;
         the_image[j][i][2] = 0.75*255;
      }
   }
}


// init
// Initialization
// Called by main
void init()
{
   glClearColor(1.0, 1.0, 1.0, 0.0);

   // Tell OpenGL to treat image as normal array
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   // Create the image in the array
   makeimage();
}


int main(int argc, char ** argv)
{
   // Initialize OpenGL/GLUT
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

   // Make a window
   glutInitWindowSize(startwinsize, startwinsize);
   glutInitWindowPosition(50, 50);
   glutCreateWindow("CS 381 - Raster Image with Zoom");

   // Initialize GL states & register callbacks
   init();
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(special);
   glutIdleFunc(idle);

   // Do something
   glutMainLoop();

   return 0;
}
