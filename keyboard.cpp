/* Source: keyboard
 * Description: Contains functions that read keyboard input.
 */

#include "render.h"
#include "keyboard.h"
#include "camera.h"
#include "physics.h"

/* Function: keyInit
 * Description: Initialize keyboard input from user.
 * Input: None
 * Output: None
 */
void keyInit()
{
	pause = 1;
	saveScreenToFile = 0;
} //end keyInit()

/* Function: keyboardKeys
 * Description: Reads keyboard input from user.
 * Input: unsigned char key - ASCII value of the key pressed.
 *		  int x - X-position
 *		  int y - Y-position
 * Output: None
 */
void keyboardKeys(unsigned char key, int x, int y) 
{
	switch(tolower(key))
	{
		case 27:
			exit(0);
			break;
		case 'z':
			zoom(-1, camFol);
			break;
		case 'x':
			zoom(1, camFol);
			break;
		case ' ':
			saveScreenToFile = 1 - saveScreenToFile;
		case 'w':
			if (camFol == 0)
				cameraFreeMove(0);
			break;
		case 's':
			if (camFol == 0)
				cameraFreeMove(1);
			break;
		case 'q':
			if (camFol == 0)
				cameraFreeMove(2);
			break;
		case 'e':
			if (camFol == 0)
				cameraFreeMove(3);
			break;
		case 'd':
			if (camFol == 0)
				cameraFreeMove(4);
			break;
		case 'a':
			if (camFol == 0)
				cameraFreeMove(5);
			break;
		case 'f':
			camFol = 1 - camFol;
			break;
		case 'p':
			saveScreenToFile = 1 - saveScreenToFile;
				break;
	} //end switch
} //end keyboardKeys

/* Function: specialKeys
 * Description: Reads special GLUT keyboard input from user.
 * Input: key - ASCII value of the key pressed.
 *		  int x - X-position
 *		  int y - Y-position
 * Output: None
 */
void specialKeys(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_RIGHT:
			break;
		case GLUT_KEY_LEFT:
			break;
		case GLUT_KEY_UP:
			break;
		case GLUT_KEY_DOWN:
			break;
	} //end switch
} //end specialKeys