/* Header: UI
 * Description: Header file for GLUI
 */

#ifndef _UI_H_
#define _UI_H_

#include "physics.h"

#define MODE 1
#define RESET 2
#define PAUSE 3
#define CAMERA 4
#define GVITY 5
#define TSTEP 6
#define NSTEP 7
#define KCOL 8
#define KDAMP 9
#define ALPHAFACTOR 10
#define BETAFACTOR 11
#define DELTAFACTOR 12
#define ADDMODEL 13
#define NEXTMODEL 14
#define TESTCASE 15
#define FRAMERATE 16

void control_cb(int control);
void setGlobal(phyzx *phyzxObj);
void dispPhysics(phyzx *phyzxObj);
void createGLUI();

#endif