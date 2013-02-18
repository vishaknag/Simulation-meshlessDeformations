/* Source: render
 * Description: Contains functions that render graphics using OpenGL.
 */

#include "render.h"
#include "lights.h"
#include "mouse.h"
#include "keyboard.h"
#include "camera.h"
#include "world.h"
#include "physics.h"
#include "input.h"
#include "ui.h"
#include "performanceCounter.h"

/* Global Variables BEGIN */
// Window settings
float ratio;
int mainWindowId = 0;

// File name
char filename[50];

// Application controls
int pause, saveScreenToFile, sprite;
GLUI *glui;
float gTStep, gKCol, gDCol, gGravity, gAlpha, gBeta, gDelta, gMass;
int gNStep, gNextModelID = 0, boxType = 3, axis = 1, stickyFloor = 0, gFRateON = 0;

// Light controls
int lighting;
int setShineLevel, setSpecLevel, setEmissLevel;
int light1, light2, light3, light4, light5, light6, light7, light8;
int materialColor;

// Mouse controls
point mousePos, pMousePos, userForce;
int leftButton, rightButton, middleButton, objectName = -1, lMouseVal = 0, iMouseModel = 0;

// Camera controls
point cameraPos, lineOfSight, viewVector;

// Camera controls
double Theta, Phi, R;
int camFol;

// Object File Data Structure
GLMmodel *objModel;
bool objCollide, renderLine;

//GLUquadricObj* quad;
GLUquadricObj* sphere;

// Deformation Controls
int gDeformMode;

// Models
struct pModel *phyzxModels;
int modelCounter = -1;
char gCrateName[30];


// Frame rate counter
PerformanceCounter pCounter;

/* Global Variables END */

/* Function: initialize
 * Description: Initializes all variables and settings for the application.
 * Input: None
 * Output: None
 */
void initialize()
{
	// Initialize all variables
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, ratio, 0.01, 1000);

	// Set Background Color
	glClearColor (0.4, 0.4, 0.4, 0.0);

	glEnable(GL_DEPTH_TEST);

	// Initialize Camera
	camInit();

	// Rand Initialization
	srand( time(NULL) );

	// Initialize Light
	lightInit();

	// Initialize Environment
	worldInit();
	inputInit();
	keyInit();
	strcpy(gCrateName, "crateFragile.ppm");

	gDeformMode = 0;

} //end initialize


void LoadImages()
{
	glEnable(GL_DEPTH_TEST);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);

	Snap* snapBK = storeBitmap("textWall.bmp");
	back_face_id = ImageToGLTexture(snapBK);
	delete snapBK;

	Snap* snapF = storeBitmap("wall.bmp");
	front_face_id = ImageToGLTexture(snapF);
	delete snapF;

	Snap* snapT = storeBitmap("topSky.bmp");
	top_face_id = ImageToGLTexture(snapT);
	delete snapT;

	Snap* snapB = storeBitmap("floor.bmp");
	bottom_face_id = ImageToGLTexture(snapB);
	delete snapB;

	Snap* snapL = storeBitmap("wall.bmp");
	left_face_id = ImageToGLTexture(snapL);
	delete snapL;

	Snap* snapR = storeBitmap("wall.bmp");
	right_face_id = ImageToGLTexture(snapR);
	delete snapR;

	glDisable(GL_COLOR_MATERIAL);

	return; 
}

/* Function: display
 * Description: Renders all specified graphics on the given window.
 * Input: None
 * Output: None
 */
void display(void)
{
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ACCUM_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	// Setup camera position
	if (camFol == 1 && phyzxModels->mIndex != -1)
		cameraFollow(phyzxModels->pObj->model);
	
	setCamera();

	// Setup lights
	setLights();
	enableLights();

	if (axis == 1)
		renderAxis();
	
	// Display all the models in the List
	pModel *temp = phyzxModels;
	if(temp != NULL)
	{
		while(temp->next != NULL)
		{
			glPushMatrix();
	//			glTranslatef(temp->translate.x, temp->translate.y, temp->translate.z);
				if(strcmp(temp->file, "bunny.obj") == 0)
				{
					glmDraw(temp->pObj->model, GLM_SMOOTH);
				}
				else
				{
					glmDraw(temp->pObj->model, GLM_TEXTURE);
				}

			glPopMatrix();

	/*		sphere = gluNewQuadric();
			glPushMatrix();
				glTranslatef(temp->cModel.x, temp->cModel.y, temp->cModel.z);
				gluSphere(sphere, temp->radius, 30, 30);
			glPopMatrix();
	*/
			temp = temp->next;
		}
	}

	if (leftButton && lMouseVal == 2 && objectName != 1 && iMouseModel != -1 && renderLine)
	{
		point vertex;
		point mouse = getCoord(mousePos.x, mousePos.y);
		point pMouse = getCoord(pMousePos.x, pMousePos.y);

		pModel *temp = phyzxModels;
	
		while(temp->next != NULL)
		{
			if(temp->mIndex == iMouseModel)
			{
				vertex = vMake(temp->pObj->model->vertices[3*objectName], temp->pObj->model->vertices[3*objectName+1], temp->pObj->model->vertices[3*objectName+2]);
				break;
			}
			temp = temp->next;
		}

		glDisable(GL_LIGHTING);
		glPushMatrix();
			glBegin(GL_LINES);
				glColor4f(1.0, 1.0, 0.0, 1.0);
				glVertex3f(mouse.x, mouse.y, mouse.z);
				glVertex3f(vertex.x, vertex.y, vertex.z);
			glEnd();
		glPopMatrix();
		glEnable(GL_LIGHTING);
	} //end if

	/*if (test)
	{
		glmPrintGroupInfo(objModel);
		test = false;
	} //end if */

	/*if (objectName != -1)
	{
		sphere = gluNewQuadric();
		glPushMatrix();
			glColor3f(0.0, 1.0, 0.0);
			glTranslatef(objModel->vertices[3*objectName], objModel->vertices[3*objectName + 1], objModel->vertices[3*objectName + 2]);
			gluSphere(sphere,0.05, 60, 60);
		glPopMatrix();
	}*/
	
	changeBox(boxType);

	glutSwapBuffers();
} //end display

/* Function: idle
 * Description: Continues rendering all specified graphics while no inputs are given.
 * Input: None
 * Output: None
 */
void idle(void)
{
	char ssname[20]="modxxxx.obj";
	double frameRate = 0.0;

	// save screen to file
	ssname[3] = 48 + (sprite / 1000);
	ssname[4] = 48 + (sprite % 1000) / 100;
	ssname[5] = 48 + (sprite % 100 ) / 10;
	ssname[6] = 48 + sprite % 10;

	if (saveScreenToFile == 1)
	{
		//saveScreenshot(WINRESX, WINRESY, ssname);
		glmWriteOBJ(phyzxModels->pObj->model, ssname, GLM_SMOOTH, 1); 
	//	saveScreenToFile = 1; // save only once, change this if you want continuos image generation (i.e. animation)
		sprite++;
	} //end if

	if (sprite >= 9999)
	{
		sprite = 0;
	} //end if

	if (pause == 0)
	{
		// insert code which appropriately performs one step of the cube simulation:
//		for (int i = 1; i <= phyzxObj->n; i++)
//		{
			// Recompute the goal position. Timestep using modified Euler
			// Collision detection and response and the final position is updated
		for(int i = 0; i < gNStep; i++)
		{
			if(gFRateON)
			{
				pCounter.StartCounter();
				CallPerFrame();
				pCounter.StopCounter();
				frameRate = pCounter.GetElapsedTime();
				printf("Frame rate = %lf\n", 1.0 / frameRate);
			}
			else
			{
				CallPerFrame();
			}
		}
		
//		} //end for*/
		//pause = 1 - pause;
	} //end if

	/* According to the GLUT specification, the current window is 
     undefined during an idle callback.  So we need to explicitly change
     it if necessary */
    if ( glutGetWindow() != mainWindowId ) 
		glutSetWindow(mainWindowId); 
    glutPostRedisplay();
} //end idle

/* Function: reshape
 * Description: Redraws the window when the user changes the window size.
 * Input: w - width of the window
 *        h - height of the window
 * Output: None
 */
void reshape (int w, int h)
{
	int winRes = h;
	ratio = 1.0f * w / h;
	glViewport (0, 0, w, h);
	//GLUI_Master.auto_set_viewport();
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45, ratio, 0.01, 1000);
	glMatrixMode(GL_MODELVIEW);
	setCamera();
} //end reshape

/* Function: AddModel
 * Description: adds a new model from the information present in the filename into the list
 * Input: filename - name of model input file
 * Output: None
 */
void AddModel(char *filename, int position)
{
	double random = 0;

	pModel *node;
	node = (pModel*)malloc(sizeof(pModel));
	node->next = phyzxModels;
	phyzxModels = node;

	phyzxModels->mIndex = ++modelCounter;

	phyzxModels->pObj = new phyzx();

	strcpy(phyzxModels->file, filename);
	phyzxModels->pObj->model = glmReadOBJ(phyzxModels->file);

	if(gNextModelID != 4)
	{
		strcpy(phyzxModels->pObj->model->materials[1].textureFile, gCrateName);
		glmSetUpTextures(phyzxModels->pObj->model, GL_MODULATE);
	}

	// Initialize the Physics module
	phyzxInit(phyzxModels->pObj);
	
	if(position == RANDOMPOS) 
	{
		random = (double)(rand() % (200 + 1));
		random -= 100;
		random /= 100;
		phyzxModels->translate.x = random;

		phyzxModels->translate.y = 0.0f;

		random = (double)(rand() % (200 + 1));
		random -= 100;
		random /= 100;
		phyzxModels->translate.z = random;

		phyzxModels->pObj->deformMode = gDeformMode;
	}
	else if(position == TESTCASE1POS)
	{
		phyzxModels->translate.x = -0.5;
		phyzxModels->translate.y = 0.0f;
		phyzxModels->translate.z = 0.0;

		phyzxModels->pObj->deformMode = 0;
	}
	else if(position == TESTCASE2POS)
	{
		phyzxModels->translate.x = 0.5;
		phyzxModels->translate.y = 0.0f;
		phyzxModels->translate.z = 0.0;

		phyzxModels->pObj->deformMode = 1;
	}
	
	for (unsigned int index = STARTFROM; index <= phyzxModels->pObj->model->numvertices; index++)
	{
		phyzxModels->pObj->model->vertices[3*index] += phyzxModels->translate.x;
		phyzxModels->pObj->model->vertices[3*index + 1] += phyzxModels->translate.y;
		phyzxModels->pObj->model->vertices[3*index + 2] += phyzxModels->translate.z;

		phyzxModels->pObj->model->verticesRest[3*index] += phyzxModels->translate.x;
		phyzxModels->pObj->model->verticesRest[3*index + 1] += phyzxModels->translate.y;
		phyzxModels->pObj->model->verticesRest[3*index + 2] += phyzxModels->translate.z;
	}

	// Compute the center of the model with  the radius of the bounding sphere
	glmMeshGeometricParameters(phyzxModels->pObj->model, &phyzxModels->cModel.x, &phyzxModels->cModel.y, &phyzxModels->cModel.z, &phyzxModels->radius);

	// Compute the radius of the best bounding sphere around the model
	glmMeshRadius(phyzxModels->pObj->model,  phyzxModels->cModel.x, phyzxModels->cModel.y, phyzxModels->cModel.z, &phyzxModels->radius);

	// Display the live variables of the first model in GLUI
	dispPhysics(phyzxModels->pObj);
}


void RunTestCase()
{
	AddModel(filename, TESTCASE1POS);
	AddModel(filename, TESTCASE2POS);
}


/* Function: DeleteModels
 * Description: clears all the models in the list
 * Input: None
 * Output: None
 */
/*
void DeleteModels()
{
	pModel *next, *cur;
	
	next = phyzxModels;
	while(next->next->next != NULL)
	{
		cur = next;
		next = next->next;	
		free(cur);
	}
	phyzxModels = next;
	phyzxModels->mIndex = 0;

	modelCounter = 1;
}
*/
void DeleteModels()
{
	pModel *next, *cur;
	
	next = phyzxModels;
	while(next->next != NULL)
	{
		cur = next;
		next = next->next;	
		free(cur);
	}
	phyzxModels = next;
	phyzxModels->mIndex = -1;

	modelCounter = -1;
}

/* Main Loop */
int main(int argc, char** argv)
{
	/*if (argc < 2)
	{  
		printf ("Usage: %s [object file]\n", argv[0]);
		exit(0);
	}*/
	strcpy(filename, "crate.obj");

	// GLUT Initialization
	glutInit(&argc, argv);

	// Create window
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WINRESX, WINRESY);
	glutInitWindowPosition(0,0);
	mainWindowId = glutCreateWindow("Meshless Deformation Using Shape Matching");

	// Initialize Application
	initialize();
	LoadImages();

	phyzxModels = (pModel*)malloc(sizeof(pModel));
	
	phyzxModels->next = NULL;

//	AddModel(filename, RANDOMPOS);
	
	// Redraws window if window is resized
	glutReshapeFunc(reshape);

	// Create GLUI window
	createGLUI();

	// Display Graphics
	glutDisplayFunc(display);
	GLUI_Master.set_glutIdleFunc(idle);

	// Read User Mouse Input
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passiveMotion);

	// Read User Keyboard Input
	glutKeyboardFunc(keyboardKeys);	
	glutSpecialFunc(specialKeys);

	glutMainLoop();
	return 0; 
}