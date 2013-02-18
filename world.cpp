/* Source: world
 * Description: Contains functions that renders background and/or environment objects.
 */

#include "render.h"
#include "world.h"
#include "physics.h"

/* Function: worldInit
 * Description: Initializes all background and/or environment object variables and settings.
 * Input: None
 * Output: None
 */
void worldInit()
{
	//Initialize variables
	gTStep = 0.002;
	gNStep = 4;
	gKCol = 70.0;
	gDCol = 0.2;
	gGravity = -0.7;
	gAlpha = 0.1;
	gBeta = 0.15;
	gDelta = 0.01;
	gMass = 0.02;
	userForce = vMake(0.0);
} //end worldInit

/* Function: renderAxis
 * Description: Renders the xyz Axis in world space.
 * Input: None
 * Output: None
 */
void renderAxis()
{
	glLineWidth(2);
	glDisable(GL_LIGHTING);
	glPushMatrix();
		glBegin(GL_LINES);
			// Render X Axis
			glColor4f(1.0, 0.0, 0.0, 0.0);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(1.0, 0.0, 0.0);

			// Render Y Axis
			glColor4f(0.0, 1.0, 0.0, 0.0);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(0.0, 1.0, 0.0);

			// Render Z Axis
			glColor4f(0.0, 0.0, 1.0, 0.0);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(0.0, 0.0, 1.0);
		glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);
} //end renderAxis

/* Function: setBoundingBox
 * Description: Setup the normals of each face in the bounding box and its distance to world origin.
 * Input: box - Data structure of the bounding box
 * Output: Nothing
 */
void setBoundingBox(struct boundBox * box)
{
	// Set Left Face Information
	box->norm[0].x = 1.0;
	box->norm[0].y = 0.0;
	box->norm[0].z = 0.0;
	box->d[0] = 2.0;

	// Set Right Face Information
	box->norm[1].x = -1.0;
	box->norm[1].y = 0.0;
	box->norm[1].z = 0.0;
	box->d[1] = 2.0;

	// Set Front Face Information
	box->norm[2].x = 0.0;
	box->norm[2].y = 1.0;
	box->norm[2].z = 0.0;
	box->d[2] = 2.0;

	// Set Back Face Information
	box->norm[3].x = 0.0;
	box->norm[3].y = -1.0;
	box->norm[3].z = 0.0;
	box->d[3] = 2.0;

	// Set Bottom Face Information
	box->norm[4].x = 0.0;
	box->norm[4].y = 0.0;
	box->norm[4].z = 1.0;
	box->d[4] = 2.0;

	// Set Top Face Information
	box->norm[5].x = 0.0;
	box->norm[5].y = 0.0;
	box->norm[5].z = -1.0;
	box->d[5] = 2.0;
} //end boundingBox

/* Function: renderWireBox
 * Description: Renders the wireframe box about the world origin.
 * Input: None
 * Output: None
 */
void renderWireBox()
{
	int i,j;

	// Render wall in wireframe
	glDisable(GL_LIGHTING);
	glColor4f(1.0, 1.0, 1.0, 0.0);
	glBegin(GL_LINES);

		// front face
		for(i=-2; i<=2; i++)
		{
			glVertex3f(i, -2, 2);
			glVertex3f(i, 2, 2);
		}
		for(j=-2; j<=2; j++)
		{
			glVertex3f(-2, j, 2);
			glVertex3f(2, j, 2);
		}

		// back face
		for(i=-2; i<=2; i++)
		{
			glVertex3f(i, -2, -2);
			glVertex3f(i, 2, -2);
		}
		for(j=-2; j<=2; j++)
		{
			glVertex3f(-2, j, -2);
			glVertex3f(2, j, -2);
		}

		// left face
		for(i=-2; i<=2; i++)
		{
			glVertex3f(-2, i, -2);
			glVertex3f(-2, i, 2);
		}
		for(j=-2; j<=2; j++)
		{
			glVertex3f(-2, -2, j);
			glVertex3f(-2, 2, j);
		}

		// right face
		for(i=-2; i<=2; i++)
		{
			glVertex3f(2, i, -2);
			glVertex3f(2, i, 2);
		}
		for(j=-2; j<=2; j++)
		{
			glVertex3f(2, -2, j);
			glVertex3f(2, 2, j);
		}
	glEnd();
	glEnable(GL_LIGHTING);

	// Render solid floor
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	GLfloat materialColor[] = { 0.0, 0.0, 0.7, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor);
	
	glBegin(GL_POLYGON);
		glVertex3f(2.0, -2.0, 2.0);
		glVertex3f(2.0, -2.0, -2.0);
		glVertex3f(-2.0, -2.0, -2.0);
		glVertex3f(-2.0, -2.0, 2.0);
	glEnd();
	glDisable(GL_CULL_FACE);
} //end renderWireBox

/* Function: renderCornellBox
 * Description: Renders a Cornell Box encompassing the world origin.
 * Input: None
 * Output: None
 */
void renderCornellBox()
{
	// Renders a Cornell Box
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	GLfloat materialColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor);	
	glBegin(GL_POLYGON);
		glVertex3f(CORNELLSIZE, CORNELLSIZE, -CORNELLSIZE);
		glVertex3f(CORNELLSIZE, -CORNELLSIZE, -CORNELLSIZE);
		glVertex3f(-CORNELLSIZE, -CORNELLSIZE, -CORNELLSIZE);
		glVertex3f(-CORNELLSIZE, CORNELLSIZE, -CORNELLSIZE);
	glEnd();

	GLfloat materialColor1[] = { 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor1);	
	glBegin(GL_POLYGON);
		glVertex3f(-CORNELLSIZE, -CORNELLSIZE, -CORNELLSIZE);
		glVertex3f(CORNELLSIZE, -CORNELLSIZE, -CORNELLSIZE);
		glVertex3f(CORNELLSIZE, -CORNELLSIZE, CORNELLSIZE);
		glVertex3f(-CORNELLSIZE, -CORNELLSIZE, CORNELLSIZE);
	glEnd();

	// Right Wall
	GLfloat materialColor2[] = { 0.0, 1.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor2);	
	glBegin(GL_POLYGON);
		glVertex3f(CORNELLSIZE, -CORNELLSIZE, CORNELLSIZE);
		glVertex3f(CORNELLSIZE, -CORNELLSIZE, -CORNELLSIZE);
		glVertex3f(CORNELLSIZE, CORNELLSIZE, -CORNELLSIZE);
		glVertex3f(CORNELLSIZE, CORNELLSIZE, CORNELLSIZE);
	glEnd();

	GLfloat materialColor3[] = { 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor3);	
	glBegin(GL_POLYGON);
		glVertex3f(CORNELLSIZE, CORNELLSIZE, CORNELLSIZE);
		glVertex3f(-CORNELLSIZE, CORNELLSIZE, CORNELLSIZE);
		glVertex3f(-CORNELLSIZE, -CORNELLSIZE, CORNELLSIZE);
		glVertex3f(CORNELLSIZE, -CORNELLSIZE, CORNELLSIZE);
	glEnd();

	// Left Wall
	GLfloat materialColor4[] = { 1.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor4);	
	glBegin(GL_POLYGON);
		glVertex3f(-CORNELLSIZE, -CORNELLSIZE, -CORNELLSIZE);
		glVertex3f(-CORNELLSIZE, -CORNELLSIZE, CORNELLSIZE);
		glVertex3f(-CORNELLSIZE, CORNELLSIZE, CORNELLSIZE);
		glVertex3f(-CORNELLSIZE, CORNELLSIZE, -CORNELLSIZE);
	glEnd();
	glDisable(GL_CULL_FACE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	GLfloat materialColor5[] = { 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor5);	
	glBegin(GL_POLYGON);
		glVertex3f(CORNELLSIZE, CORNELLSIZE, CORNELLSIZE);
		glVertex3f(-CORNELLSIZE, CORNELLSIZE, CORNELLSIZE);
		glVertex3f(-CORNELLSIZE, CORNELLSIZE, -CORNELLSIZE);
		glVertex3f(CORNELLSIZE, CORNELLSIZE, -CORNELLSIZE);
	glEnd();
	glDisable(GL_CULL_FACE);
} //end renderCornellBox

/* Function: changeBox
 * Description: Changes the bounding box type.
 * Input: index - Index of the bounding box
 * Output: None
 */
void changeBox(int index)
{
	// Changes the type ofbounding box
	switch(index)
	{
		case 0:
			return; // No Box
			break;
		case 1:
			renderWireBox();
			break;
		case 2:
			renderCornellBox();
			break;
		case 3:
			glDisable(GL_LIGHTING);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glEnable(GL_LIGHTING);

			// Render the box
			renderSkyBox(front_face_id, back_face_id, right_face_id, left_face_id, top_face_id, bottom_face_id);

			glDisable(GL_CULL_FACE);

			break;
	} //end switch

	return;
} //end changeBox