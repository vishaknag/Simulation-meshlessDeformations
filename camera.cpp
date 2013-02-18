/* Author: Samuel Yung
 * Source: camera
 * Description: Contains functions that controls the camera.
 */

#include "render.h"
#include "camera.h"

/* Function: camInit
 * Description: Initializes the camera control variables.
 * Input: None
 * Output: None
 */
void camInit()
{
	Theta = 0;  // Z-X plane
	Phi = PI + PI / 2;  // Y-X plane
	R = 4;

	lineOfSight.x = 0.0;
	lineOfSight.y = 0.0;
	lineOfSight.z = 0.0;

	cameraPos.x = lineOfSight.x + (R * cos(Phi) * cos(Theta));
	cameraPos.y = lineOfSight.y + (R * sin(Theta));
	cameraPos.z = lineOfSight.z + (-R * sin(Phi) * cos(Theta));

	viewVector.x = 0.0;
	viewVector.y = 1.0;
	viewVector.z = 0.0;

	camFol = 0;
} //end camInit

/* Function: setCamera()
 * Description: Sets the camera position and viewing.
 * Input: None
 * Output: None
 */
void setCamera()
{
	glLoadIdentity();
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, lineOfSight.x, lineOfSight.y, lineOfSight.z, viewVector.x, viewVector.y, viewVector.z);
} //end setCamera

/* Function: zoom
 * Description: Zooms the camera in and out.
 * Input: dir - Zoom direction.  (Zoom In = 1, Zoom Out = -1)
 * Output: None
 */
void zoom(int dir, int follow)
{
	R += dir*ZOOM; // Increment/decrement zoom distance

	if (R < 0.2)  // Zoom In Limit
		R = 0.2;

	if (follow == 0)
	{
		// Recompute camera position
		cameraPos.x = lineOfSight.x + (R * cos(Phi) * cos(Theta));
		cameraPos.y = lineOfSight.y + (R * sin(Theta));
		cameraPos.z = lineOfSight.z + (-R * sin(Phi) * cos(Theta));
	} //end if
	else if (follow == 1)
		cameraPos.z = lineOfSight.z + R;

	setCamera();
} //end zoom


/* Function: cameraOrbit
 * Description: Rotates the entire camera about a point using the mouse.
 * Input: x - X Coordinate of the mouse
 *        y - Y Coordinate of the mouse
 * Output: None
 */
void cameraOrbit(int x, int y)
{
	point mouse;

	mouse.x = x - mousePos.x;
	mouse.y = y - mousePos.y;

	Phi += mouse.x * 0.01;
	Theta += mouse.y * 0.01;
    
    if (Phi > (2 * PI))
		Phi -= 2 * PI;
    
    if (Phi < 0)
		Phi += 2 * PI;
    
    if (Theta > (PI / 2 - 0.01)) // dont let the point enter the north pole
		Theta = PI / 2 - 0.01;
    
    if (Theta < (-PI / 2 + 0.01))
		Theta = -PI / 2 + 0.01;

	// Recompute camera position
	cameraPos.x = lineOfSight.x + (R * cos(Phi) * cos(Theta));
	cameraPos.y = lineOfSight.y + (R * sin(Theta));
	cameraPos.z = lineOfSight.z + (-R * sin(Phi) * cos(Theta));

	setCamera();
} //end cameraOrbit

/* Function: cameraFreeMove
 * Description: Free movement camera computations.  Allow camera to move 
 *              forward, backword, up, down, strafe left, strafe right, 
 *              tilt up/down and turn right/left.
 * Input: dir - Direction of movement
 *              0. Move Camera Forward
 *              1. Move Camera Backward
 *              2. Move Camera Up
 *              3. Move Camera Down
 *              4. Move Camera Right
 *              5. Move Camera Left
 *              6. Tilt Camera Up		(Not Implemented)
 *              7. Tilt Camera Down		(Not Implemented)
 *              8. Turn Camera Right	(Not Implemented)
 *              9. Turn Camera Left		(Not Implemented)
 * Output: None
 */
void cameraFreeMove(int dir)
{
	point mouse, vec, pVec, cVec, cPos;
	double ang, length;

	pDIFFERENCE(lineOfSight, cameraPos, vec);
	pNORMALIZE(vec);

	if (dir == 0)			// Move Camera Forward
	{
		pMULTIPLY(vec, CAMSPEED, vec);
		pSUM(cameraPos, vec, cameraPos);
		pSUM(lineOfSight, vec, lineOfSight);

	} //end if 
	else if (dir == 1)		// Move Camera Backward
	{
		pMULTIPLY(vec, CAMSPEED, vec);
		pDIFFERENCE(cameraPos, vec, cameraPos);
		pDIFFERENCE(lineOfSight, vec, lineOfSight);
	} //end else
	else if (dir == 2)		// Move Camera Up
	{
		ang = Phi + CAMROT;
		cPos.x = lineOfSight.x + (R * cos(ang) * cos(Theta));
		cPos.y = lineOfSight.y + (R * sin(Theta));
		cPos.z = lineOfSight.z + (-R * sin(ang) * cos(Theta));

		pDIFFERENCE(cPos, cameraPos, pVec);
		pNORMALIZE(pVec);
		CROSSPRODUCTp(pVec, vec, cVec);
		pMULTIPLY(cVec, CAMSPEED, cVec);
		pSUM(cameraPos, cVec, cameraPos);
		pSUM(lineOfSight, cVec, lineOfSight);
	} //end else
	else if (dir == 3)		// Move Camera Down
	{
		ang = Phi + CAMROT;
		cPos.x = lineOfSight.x + (R * cos(ang) * cos(Theta));
		cPos.y = lineOfSight.y + (R * sin(Theta));
		cPos.z = lineOfSight.z + (-R * sin(ang) * cos(Theta));

		pDIFFERENCE(cPos, cameraPos, pVec);
		pNORMALIZE(pVec);
		CROSSPRODUCTp(pVec, vec, cVec);
		pMULTIPLY(cVec, CAMSPEED, cVec);
		pDIFFERENCE(cameraPos, cVec, cameraPos);
		pDIFFERENCE(lineOfSight, cVec, lineOfSight);
	} //end else
	else if (dir == 4)		// Move Camera Strafe Right
	{
		ang = Theta + CAMROT;
		cPos.x = lineOfSight.x + (R * cos(Phi) * cos(ang));
		cPos.y = lineOfSight.y + (R * sin(ang));
		cPos.z = lineOfSight.z + (-R * sin(Phi) * cos(ang));

		pDIFFERENCE(cPos, cameraPos, pVec);
		pNORMALIZE(pVec);
		CROSSPRODUCTp(vec, pVec, cVec);
		pMULTIPLY(cVec, CAMSPEED, cVec);
		pSUM(cameraPos, cVec, cameraPos);
		pSUM(lineOfSight, cVec, lineOfSight);
	} //end else
	else if (dir == 5)		// Move Camera Strafe Left
	{
		ang = Theta + CAMROT;
		cPos.x = lineOfSight.x + (R * cos(Phi) * cos(ang));
		cPos.y = lineOfSight.y + (R * sin(ang));
		cPos.z = lineOfSight.z + (-R * sin(Phi) * cos(ang));

		pDIFFERENCE(cPos, cameraPos, pVec);
		pNORMALIZE(pVec);
		CROSSPRODUCTp(vec, pVec, cVec);
		pMULTIPLY(cVec, CAMSPEED, cVec);
		pDIFFERENCE(cameraPos, cVec, cameraPos);
		pDIFFERENCE(lineOfSight, cVec, lineOfSight);
	} //end else
} //end cameraFreeMove

/* Function: cameraFollow
 * Description: Fixates the camera focus on an obj and follows the object.
 * Input: model - Model information
 * Output: None
 */
void cameraFollow(GLMmodel *model)
{
	point obj;
	double objRadius;
	
	Theta = 0;  // Z-X plane
	Phi = PI + PI / 2;  // Y-X plane

	glmMeshGeometricParameters(model, &obj.x, &obj.y, &obj.z, &objRadius);

	pCPY(obj, lineOfSight);
	cameraPos.x = lineOfSight.x;
	cameraPos.y = lineOfSight.y;
	cameraPos.z = lineOfSight.z + R;
} //end cameraFollow