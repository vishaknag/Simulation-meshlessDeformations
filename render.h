/* Header: render
 * Description: Header file for rendering graphics in OpenGL
 */

#ifndef _RENDER_H_
#define _RENDER_H_

// Header Files
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "openGL-headers.h"
#include <gsl/gsl_linalg.h>
#include "glme.h"
#include "glmeEncode.h"
#include "vector.h"
#include "matrix.h"
#include "texture.h"
#include "pic.h"

// Mathematics Definitions
#define PI 3.141592653589793238462643383279
#define BUFSIZE 512

// Window size
#define WINRESX 700
#define WINRESY 700

// Colors
#define GOLD 0
#define MAROON 1
#define FLESH 2

#define RANDOMPOS 1
#define TESTCASE1POS 2
#define TESTCASE2POS 3

// Window settings
extern float ratio;

// File name
extern char filename[50];

// Windows Settings
extern int mainWindowId;

// Application controls
extern int pause, saveScreenToFile, sprite;
extern GLUI *glui;
extern float gTStep, gKCol, gDCol, gGravity, gAlpha, gBeta, gDelta, gMass;
extern int gNStep, gNextModelID, boxType, axis, stickyFloor, gFRateON;

// Light Settings
extern int lighting;
extern int setShineLevel, setSpecLevel, setEmissLevel;
extern int light1, light2, light3, light4, light5, light6, light7, light8; 
extern int materialColor;

// Mouse controls
extern point mousePos, pMousePos, userForce;
extern int leftButton, rightButton, middleButton, objectName, lMouseVal, iMouseModel;

// Camera controls
extern point cameraPos, lineOfSight, viewVector;
extern double Theta, Phi, R;
extern int camFol;

// Crate name
extern char gCrateName[30];

// Bounding Box settings
struct boundBox
{
	struct point norm[6]; // plane normal for each face of the bounding box
	double d[6]; // Distance from the plane to the world origin point
};

//extern struct boundBox box;

// Models
extern struct pModel *phyzxModels;

// Object File Data Structure
extern GLMmodel *objModel;
extern bool objCollide, renderLine;

// Deformation Controls
extern int gDeformMode;

// Run the test case
void RunTestCase();

// Adds a new model to the simulation
void AddModel(char *filename, int position);

// deletes all the models from the simulation
void DeleteModels();

// Loads the BMP images for texture mapping
void LoadImages();

#endif