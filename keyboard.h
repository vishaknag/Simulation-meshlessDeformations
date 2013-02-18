/* Header: keyboard
 * Description: Header file for keyboard controls
 */

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

// Keyboard Controls
void keyInit();
void keyboardKeys(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);

#endif