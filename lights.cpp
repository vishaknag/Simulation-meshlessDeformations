/* Source: lights
 * Description: Contains functions that controls the light settings.
 */

#include "render.h"
#include "lights.h"

/* Function: lightInit
 * Description: Initializes all light variables and settings for the application.
 * Input: None
 * Output: None
 */
void lightInit()
{
	// LIGHTING SETTING CONTROLS
	lighting = 0;

	materialColor = 2;
	setShineLevel = 1; //Whether the shininess parameter is turned on
	setSpecLevel = 0; //Whether the specularity parameter is turned on
	setEmissLevel = 0; //Whether the emission parameter is turned on

	// Lighting controllers
	light1 = 1;
	light2 = 0;
	light3 = 1;
	light4 = 0;
	light5 = 0;
	light6 = 0;
	light7 = 0;
	light8 = 0;

	setLights();

	glEnable(GL_LIGHTING);
	
	// Control the lights 
	enableLights();

} //end lightInit()

/* Function: setLights()
 * Description: Setup the lighting variables.
 * Input: None
 * Output: None
 */
void setLights()
{
	float shineLevel;

	if (setShineLevel) {
		shineLevel = 25;
	}
	else {
		shineLevel = 12;
	}

	float specLevel;
	if (setSpecLevel) {
		specLevel = 1;
	}
	else {
		specLevel = 0.3f;
	}

	float emissLevel;
	if (setEmissLevel) {
		emissLevel = 0.2f;
	}
	else {
		emissLevel = 0;
	}

//------------------------------
//		SCENE LIGHTING
//------------------------------
	// ---------------------
	//	LIGHT 1
	// ---------------------
	GLfloat lightColor1[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos1[] = {1.5f * 0.9, 2 * 0.9, 1.5 * 0.9, 0.0f};

	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightColor1);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);

	// ---------------------
	//	LIGHT 2
	// ---------------------
	GLfloat lightColor2[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos2[] = { -1.9, -1.9, -1.9, 0.0};

	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor2);
	glLightfv(GL_LIGHT2, GL_SPECULAR, lightColor2);
	glLightfv(GL_LIGHT2, GL_POSITION, lightPos2);
    
	// ---------------------
	//	LIGHT 3
	// ---------------------
	GLfloat lightColor3[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos3[] = { 1.9, -1.9, -1.9, 0.0};

	glLightfv(GL_LIGHT3, GL_DIFFUSE, lightColor3);
	glLightfv(GL_LIGHT3, GL_SPECULAR, lightColor3);
	glLightfv(GL_LIGHT3, GL_POSITION, lightPos3);

	// ---------------------
	//	LIGHT 4
	// ---------------------
	GLfloat lightColor4[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos4[] = { 1.9, 1.9, -1.9, 0.0 };

	glLightfv(GL_LIGHT4, GL_DIFFUSE, lightColor3);
	glLightfv(GL_LIGHT4, GL_SPECULAR, lightColor3);
	glLightfv(GL_LIGHT4, GL_POSITION, lightPos3);

	// ---------------------
	//	LIGHT 5
	// ---------------------
	GLfloat lightColor5[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos5[] = {-1.9, 1.9, -1.9, 0.0 };

	glLightfv(GL_LIGHT3, GL_DIFFUSE, lightColor3);
	glLightfv(GL_LIGHT3, GL_SPECULAR, lightColor3);
	glLightfv(GL_LIGHT3, GL_POSITION, lightPos3);

	// ---------------------
	//	LIGHT 6
	// ---------------------
	GLfloat lightColor6[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos6[] = {-1.9, -1.9, 1.9, 0.0};

	glLightfv(GL_LIGHT6, GL_DIFFUSE, lightColor6);
	glLightfv(GL_LIGHT6, GL_SPECULAR, lightColor6);
	glLightfv(GL_LIGHT6, GL_POSITION, lightPos6);

	// ---------------------
	//	LIGHT 7
	// ---------------------
	GLfloat lightColor7[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos7[] = {1.9, 1.9, 1.9, 0.0 };

	glLightfv(GL_LIGHT7, GL_DIFFUSE, lightColor7);
	glLightfv(GL_LIGHT7, GL_SPECULAR, lightColor7);
	glLightfv(GL_LIGHT7, GL_POSITION, lightPos7);

	// ---------------------
	//	LIGHT 8
	// ---------------------
	GLfloat lightColor8[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos8[] = {-1.9, 1.9, 1.9, 0.0};

	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor8);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor8);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos8);

	GLfloat materialColor2[4];
	if(materialColor == GOLD)
	{
		materialColor2[0] = 1;
		materialColor2[1] = 1;
		materialColor2[2] = 0;
		materialColor2[3] = 1;
	}
	if(materialColor == MAROON)
	{
		materialColor2[0] = 0.556863;
		materialColor2[1] = 0.137255;
		materialColor2[2] = 0.419608;
		materialColor2[3] = 1;
	}
	if(materialColor == FLESH)
	{
		materialColor2[0] = 0.96;
		materialColor2[1] = 0.80;
		materialColor2[2] = 0.69;
		materialColor2[3] = 1;
	}

	//The specular (shiny) component of the materialf
	GLfloat materialSpecular2[] = {specLevel, specLevel, specLevel, 1.0f};

	//The color emitted by the material
	GLfloat materialEmissLevel2[] = {emissLevel, emissLevel, emissLevel, 1.0f};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor2);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular2);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmissLevel2);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shineLevel); //The shininess parameter
} //end setLights

/* Function: lightInit
 * Description: Enables the light that is selected.
 * Input: None
 * Output: None
 */
void enableLights()
{
	if(light1)
		glEnable(GL_LIGHT0);
	else
		glDisable(GL_LIGHT0);

	if(light2)
		glEnable(GL_LIGHT1);
	else
		glDisable(GL_LIGHT1);

	if(light3)
		glEnable(GL_LIGHT2);
	else
		glDisable(GL_LIGHT2);

	if(light4)
		glEnable(GL_LIGHT3);
	else
		glDisable(GL_LIGHT3);

	if(light5)
		glEnable(GL_LIGHT4);
	else
		glDisable(GL_LIGHT4);

	if(light6)
		glEnable(GL_LIGHT5);
	else
		glDisable(GL_LIGHT5);

	if(light7)
		glEnable(GL_LIGHT6);
	else
		glDisable(GL_LIGHT6);

	if(light8)
		glEnable(GL_LIGHT7);
	else
		glDisable(GL_LIGHT7);
} //end enableLights