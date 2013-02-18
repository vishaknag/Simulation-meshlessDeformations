/* Header: mouse
 * Description: Header file for mouse controls
 */

#ifndef _MOUSE_H_
#define _MOUSE_H_

// Mouse Controls
point getCoord(int x, int y);
void selection(GLint hits, GLuint *buffer);
void passiveMotion(int x, int y);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);

#endif