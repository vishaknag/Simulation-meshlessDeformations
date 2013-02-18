/* Author: Samuel Yung
 * Source: input
 * Description: Contains functions that read program inputs.
 * Source: CS 599, Physically Based Modeling for Interactive Simulation and Games
 */

#include "render.h"
#include "input.h"

/* Function: inputInit
 * Description: Initializes input variables and settings.
 * Input: None
 * Output: None
 */
void inputInit()
{
	sprite = 0;
} //end inputInit

/* Write a screenshot, in the PPM format, to the specified filename, in PPM format */
void saveScreenshot(int windowWidth, int windowHeight, char *filename)
{
  if (filename == NULL)
    return;

  // Allocate a picture buffer 
  Pic * in = pic_alloc(windowWidth, windowHeight, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (int i=windowHeight-1; i>=0; i--) 
  {
    glReadPixels(0, windowHeight-i-1, windowWidth, 1, GL_RGB, GL_UNSIGNED_BYTE,
      &in->pix[i*in->nx*in->bpp]);
  }

  if (ppm_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}
