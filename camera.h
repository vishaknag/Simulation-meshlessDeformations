/* Author: Samuel Yung
 * Header: camera
 * Description: Header file for camera controls
 */

#ifndef _CAMERA_H_
#define _CAMERA_H_

#define ZOOM 0.1
#define CAMSPEED 0.1
#define TURNSPEED 0.02
#define CAMROT 0.1

/*** Functions ***/
// Camera controls
void camInit();
void setCamera();

// Orbital camera
void zoom(int dir, int follow);
void cameraOrbit(int x, int y);
void cameraFreeMove(int dir);
void cameraFollow(GLMmodel *model);

#endif
