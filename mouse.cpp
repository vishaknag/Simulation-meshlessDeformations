/* Source: mouse
 * Description: Contains functions that read mouse input.
 */

#include "render.h"
#include "mouse.h"
#include "camera.h"
#include "vector.h"
#include "physics.h"
#include "ui.h"

/* Function getCoord
 * Description: Takes the mouse coordinates in window space and determines its 3D coordinates.
 * Source: http://nehe.gamedev.net/data/articles/article.asp?article=13
 * Input: x - Mouse X-Coordinate
 *        y - Mouse Y-Coordinate
 * Output: newPos - Mouse Coordinates (X, Y, Z) 
 */
point getCoord(int x, int y)
{
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	point newPos;

	// Fetch Window Matrices
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Compute for Mouse Z-Coordinate
	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &newPos.x, &newPos.y, &newPos.z);

	return newPos;
} //end getCoord

/* Function: selection
 * Description: Determines if selection of an object on the screen occured. Then save the 
 *              selected object identification into objectName.
 * Input: hits - Number of objects selected.
 *        buffer - Information of the object selected.
 * Output: None
 */
void selection(GLint hits, GLuint *buffer)
{
	int index = 0, minObj;
	double dist, minDist = DBL_MAX;
	point mouse, vertex;

	printf("hits: %d\n", hits);
	if (hits < 1)
	{
		objectName = -1;
	} //end if
	else if (hits == 1)
	{
		index = 0;
		objectName = (int)buffer[index * 4 + 3];
	} //end else if
	else if (hits > 1)
	{
		// Convert mouse coordinates to vector data format
		mouse = getCoord(mousePos.x, mousePos.y);

		// Compute closest vertice to mouse selection coordinate
		for (index = 0; index < hits; index++)
		{
			pModel *temp = phyzxModels; 
			// Search through selection buffer
			minObj = (int)buffer[index * 4 + 3];
			
			while(temp->next != NULL)
			{
				if(lMouseVal == 1)
				{
					if(temp->mIndex == minObj)
					{
						pCPY(temp->cModel, vertex);
						break;
					}
				}
				else if(lMouseVal == 2)
				{
					if(temp->mIndex == iMouseModel)
					{
						vertex = vMake(temp->pObj->model->vertices[3*minObj], temp->pObj->model->vertices[3*minObj + 1], temp->pObj->model->vertices[3*minObj + 2]);
						break;
					}
				}
				temp = temp->next;
			}
			// Determine the distance to mouse selection coordinates
			dist = vecLeng(vertex, mouse);

			// Record closest vertex to mouse selection coordinate
			if (dist < minDist)
			{
				objectName = minObj;
				minDist = dist;
			} //end if
		} //end for
	} //end else if
} //end selection

/* Function: passiveMotion
 * Description: Function that captures and records mouse passive motions
 * Input: x - X-Coordinate of the mouse position
 *        y - Y-Coordinate of the mouse position
 * Output: None
 */
void passiveMotion(int x, int y)
{
	mousePos.x = x;
	mousePos.y = y;
	mousePos.z = 0;
} //end passiveMotion

/* Function: motion
 * Description: Function that captures and records mouse motions
 * Input: x - X-Coordinate of the mouse position
 *        y - Y-Coordinate of the mouse position
 * Output: None
 */
void motion(int x, int y)
{
	point mouse, preMouse;
	double length;

	if (rightButton && !leftButton)
		cameraOrbit(x, y);

	if(leftButton && lMouseVal == 2 && objectName != -1 && pause == 0 && !rightButton)
	{
		point vertex;
		pModel *temp = phyzxModels;

		while(temp->next != NULL)
		{
			if (temp->mIndex == iMouseModel)
			{
				vertex = vMake(temp->pObj->model->vertices[3*objectName], temp->pObj->model->vertices[3*objectName + 1], temp->pObj->model->vertices[3*objectName + 2]);
				break;
			} //end if
			temp = temp->next;
		}
		mouse = getCoord(mousePos.x, mousePos.y);
		//preMouse = getCoord(pMousePos.x, pMousePos.y);
		pDIFFERENCE(mouse, vertex, userForce);

		if(!vComp(userForce, vMake(0.0)))
		{
			pNORMALIZE(userForce);
			//pMULTIPLY(userForce, (log(length + 1.0)/log(10.0)), userForce);
			pMULTIPLY(userForce, 3.0, userForce);

/*			pModel *temp = phyzxModels;
			while(temp->next != NULL)
			{
				if(temp->mIndex == iMouseModel)
				{
					pSUM(temp->pObj->extForce[objectName], userForce, temp->pObj->extForce[objectName]);
					break;
				}
				temp = temp->next;
			}
*/
		}
	}
	mousePos.x = x;
	mousePos.y = y;
} //end motion

/* Function: mouse
 * Description: Function that captures mouse input
 * Input: button - the button pressed on the mouse
 *        state - the state of the button (up/down)
 *        x - X-Coordinate of the mouse position
 *        y - Y-Coordinate of the mouse position
 * Output: None
 */
void mouse(int button, int state, int x, int y)
{
	GLuint selectBuf[BUFSIZE];
	GLuint hits = 0;
	GLint viewport[4];

	switch (button)
	{
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN)
			{
				// Save button state
				leftButton = (state == GLUT_DOWN);
				
				if(lMouseVal != 0)
				{
					// Setup Window for Selection mode
					glSelectBuffer(BUFSIZE, selectBuf);
					glGetIntegerv(GL_VIEWPORT, viewport);
					glRenderMode(GL_SELECT);

					// Initialize name stack
					glInitNames();
					glPushName(-1);

					// Setup selection area
					glMatrixMode(GL_PROJECTION);
					glPushMatrix();
						glLoadIdentity();
						gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), 5.0, 5.0, viewport);
						gluPerspective(45, ratio, 0.01, 1000);
						glMatrixMode(GL_MODELVIEW);
						
						if(lMouseVal == 1)
						{
							pModel *temp = phyzxModels;
							while(temp->next != NULL)
							{
								glLoadName(temp->mIndex);
								glmDraw(temp->pObj->model, GLM_SMOOTH);
								temp = temp->next;
							}
						}
						else if(lMouseVal == 2)
						{
							pModel *temp = phyzxModels;
							while(temp->next != NULL)
							{
								if(temp->mIndex == iMouseModel)
								{
									glmDrawPointsSelection(temp->pObj->model);
									break;
								}
								temp = temp->next;
							}
						}

					glPopMatrix();
					glFlush();

					// Identify selection area
					hits = glRenderMode(GL_RENDER);
					selection(hits, selectBuf);

					if (hits == -1 || hits == 0)
						renderLine = false;
					else
						renderLine = true;

					// Reset windows settings
					glMatrixMode (GL_PROJECTION);
					glLoadIdentity();
					glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
					gluPerspective(45, ratio, 0.01, 1000);
					
					if (objectName != -1 && lMouseVal == 1)
					{
						iMouseModel = objectName;
						pModel *temp = phyzxModels;
						while(temp->next != NULL)
						{
							if(temp->mIndex == iMouseModel)
							{
								dispPhysics(temp->pObj);
								break;
							}
							temp = temp->next;
						}
					} //end if
				}// if lMouseVal != 0

				// Save current mouse position
				pMousePos.x = x;
				pMousePos.y = y;
			} //end if
			else if (state == GLUT_UP)
			{
				// Save button state
				leftButton = (state == GLUT_DOWN);
				userForce = vMake(0.0);
				objectName = -1;
			} //end else

			break;

		case GLUT_MIDDLE_BUTTON:
			if (state == GLUT_DOWN)
			{
				// Save button state
				middleButton = (state == GLUT_DOWN);
			} //end if
			else if (state == GLUT_UP)
			{
				// Save button state
				middleButton = (state == GLUT_DOWN);
			} //end else

			break;

		case GLUT_RIGHT_BUTTON:
			if (state == GLUT_DOWN)
			{
				// Save button state
				rightButton = (state == GLUT_DOWN);
			} //end if
			else if (state == GLUT_UP)
			{
				// Save button state
				rightButton = (state == GLUT_DOWN);
			} //end else
			break;
	} //end switch
} //end mouse

