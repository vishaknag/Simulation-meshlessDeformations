/* Source: physics
 * Description: Contains functions that compute physics in real time.
 */

#include "physics.h"

// Constructor
phyzx::phyzx()
{
	model = NULL;
	h = 0.0f;
	n = 0;
	surArea = 0.0;
	stable = NULL;						
	goal = NULL;				
	mass = NULL;
	totalMass = 0.0;
	velocity = NULL;
	triAreas = NULL;
	extForce = NULL;			
	alpha = 0.0f;
	beta = 0.0f;
	delta = 0.0f;
	kWall = 0.0f;
	dWall = 0.0f;
	kSphere = 0.0f;
	dSphere = 0.0f;
	memset( (void*)&cmStable, 0, sizeof(cmStable));
	memset( (void*)&cmDeformed, 0, sizeof(cmDeformed));
	memset( (void*)&avgVel, 0, sizeof(avgVel));
	relStableLoc = NULL;
	relDeformedLoc = NULL;
	NBTStruct = NULL;
	NBVStruct = NULL;
	q = NULL;
	qT = NULL;
	memset( (void*)&Apq, 0, sizeof(Apq));	
	memset( (void*)&Aqq, 0, sizeof(Aqq));
	memset( (void*)&R, 0, sizeof(R));							
}

/* Function: phyzxInit
 * Description: Creates and initializes the phyzx object
 * Input: inputModel - Object model information
 *		  phyzxObj - current object structure 
 * Output: None
 */
void phyzxInit(phyzx *phyzxObj)
{
	int numVertices = 0;
	int size = 0;
	point v1, v2, v3;

	numVertices = phyzxObj->model->numvertices + 1;		// Count of the number of vertices in the Model

	phyzxObj->h = gTStep;
	phyzxObj->n = gNStep;
	phyzxObj->alpha = gAlpha;
	phyzxObj->beta = gBeta;
	phyzxObj->delta = gDelta;
	phyzxObj->kWall = gKCol;
	phyzxObj->dWall = gDCol;
	phyzxObj->kSphere = 50.0;
	phyzxObj->dSphere = 0.2;
	phyzxObj->totalMass = 0.0;
	phyzxObj->avgVel = vMake(0.0);

	phyzxObj->velocity = (point *)calloc(numVertices, sizeof(point));
	phyzxObj->extForce = (point *)calloc(numVertices, sizeof(point));
	phyzxObj->stable = (point *)calloc(numVertices, sizeof(point));
	phyzxObj->goal = (point *)calloc(numVertices, sizeof(point));
	phyzxObj->relStableLoc = (point *)calloc(numVertices, sizeof(point));
	phyzxObj->relDeformedLoc = (point *)calloc(numVertices, sizeof(point));
	phyzxObj->mass = (double *)calloc(numVertices, sizeof(double));
	phyzxObj->triAreas = (double *)calloc(phyzxObj->model->numtriangles, sizeof(double));
	phyzxObj->q = (matrix *)calloc(numVertices, sizeof(matrix));
	phyzxObj->qT = (matrix *)calloc(numVertices, sizeof(matrix));

	// Initialise attributes with stable values
	for(int index = STARTFROM; index <= numVertices; index++)
	{
		phyzxObj->stable[index].x = phyzxObj->model->vertices[3*index];
		phyzxObj->stable[index].y = phyzxObj->model->vertices[3*index+1];
		phyzxObj->stable[index].z = phyzxObj->model->vertices[3*index+2];
		phyzxObj->mass[index] = 0.0;//param.MASS;
		phyzxObj->extForce[index] = vMake(0.0, gGravity, 0.0);
		phyzxObj->velocity[index] = vMake(0.01, 0.0, 0.0);
	}
	
	for(unsigned int index = 0; index < phyzxObj->model->numtriangles; index++)
	{
		v1 = vMake(phyzxObj->model->vertices[3*phyzxObj->model->triangles[index].vindices[0]], phyzxObj->model->vertices[3*phyzxObj->model->triangles[index].vindices[0]+1], phyzxObj->model->vertices[3*phyzxObj->model->triangles[index].vindices[0]+2]);
		v2 = vMake(phyzxObj->model->vertices[3*phyzxObj->model->triangles[index].vindices[1]], phyzxObj->model->vertices[3*phyzxObj->model->triangles[index].vindices[1]+1], phyzxObj->model->vertices[3*phyzxObj->model->triangles[index].vindices[1]+2]);
		v3 = vMake(phyzxObj->model->vertices[3*phyzxObj->model->triangles[index].vindices[2]], phyzxObj->model->vertices[3*phyzxObj->model->triangles[index].vindices[2]+1], phyzxObj->model->vertices[3*phyzxObj->model->triangles[index].vindices[2]+2]);
		phyzxObj->triAreas[index] = AreaOfTri(v1, v2, v3); 

		(*phyzxObj).surArea += phyzxObj->triAreas[index];
	}
	
	phyzxObj->NBTStruct = glmBuildNeighborStructure(phyzxObj->model);
	phyzxObj->NBVStruct = vertexList(phyzxObj->model, phyzxObj->NBTStruct, 0.05);
	filterNBV(phyzxObj->model, phyzxObj->NBVStruct);
	compMass(phyzxObj->NBTStruct, phyzxObj);

	CalcCM(0, phyzxObj);
	CalcRelLoc(0, phyzxObj);
	CalcAqq(phyzxObj);
	calcTAqq(phyzxObj);
}

/* Function: AreaOfTri
 * Description: Computes the area of a triangle
 * Input: inputModel - three vertices of the triangle
 * Output: Area
 */
double AreaOfTri(point A, point B, point C)
{
	double mside1, mside2; 
	point side1, side2;
	double dot = 0.0;
	
	pDIFFERENCE(A, B, side1);
	pDIFFERENCE(C, B, side2);
	mside1 = vecLeng(A, B);
	mside2 = vecLeng(C, B);
	dot = dotProd(side1, side2);

	if(mside1 > mside2)
		return (sqrt(mside2 * mside2 -  dot * dot) * mside1) / 2.0;
	else
		return (sqrt(mside1 * mside1 -  dot * dot) * mside2) / 2.0;
}

/* Function: adjustMass
 * Description: Computes an adjustment value for mass.
 * Input: model - Object model information
 * Output: Returns a value to be multiplied to each mass.
 */
double adjustMass(GLMmodel *model)
{
	double ten = 1.0, value;
	int test;

	value = 1.0 / (*model).numvertices;
	test = floor(value);

	while ((test % 10) == 0)
	{
		value *= 10.0;
		test = floor(value);
		ten *= 10.0;
	} //end while

	ten /= 100.0;

	return ten;
} //end adjustMass

/* Function: compMass
 * Description: Computes the mass of each vertex
 * Input: NBVStruct - neighbouring triangles list for every vertex in the model
 * Output: None
 */
void compMass(GLMnode **NBTStruct, phyzx *phyzxObj)
{
	GLMnode *curNode;
	double adj = adjustMass(phyzxObj->model);

	for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
	{
		curNode = NBTStruct[index];

		while(curNode->next != NULL)	
		{
			phyzxObj->mass[index] += phyzxObj->triAreas[curNode->index];

			curNode = curNode->next;
		}
		phyzxObj->mass[index] /= 3.0;

		phyzxObj->mass[index] /= phyzxObj->surArea;
		
		phyzxObj->mass[index] *= adj;
		continue;
	}
}

void filterNBV(GLMmodel *obj, GLMnode **NBVStruct)
{
	for(unsigned int index = STARTFROM; index <= (*obj).numvertices; index++)
	{
		GLMnode *currCol;
		GLMnode *nextCol;
		GLMnode *preNode;


		nextCol = NBVStruct[index];

		while (nextCol->next != NULL)
		{
			currCol = nextCol;
			preNode = currCol;
			while (preNode->next != NULL)
			{
				currCol = preNode;
				currCol = currCol->next;

				if (nextCol->index == currCol->index)
					delNode(preNode, currCol->next);
				
				if(preNode->next != NULL)
					preNode = preNode->next;
			} //end while
			
			if(nextCol->next != NULL)
				nextCol = nextCol->next;
		} //end while
	} //end for
} //end filterNBV


void delNode(GLMnode *preNode, GLMnode *next)
{
	delete(preNode->next);

	if(next != NULL)
		preNode->next = next;
	else 
		preNode->next = NULL;
} //end delNode

/* Function: vertexList
 * Description: Creates a structure that lists
 * Input: None
 * Output: None
 */
GLMnode ** vertexList(GLMmodel *obj, GLMnode **NBTStruct, double radius)
{
	GLMnode *node, *temp;
	GLMnode **NBVStruct;

	NBVStruct = (GLMnode**)malloc(sizeof(GLMnode*) * ((*obj).numvertices + 1));
	
	for (unsigned int i = 1; i <= (*obj).numvertices; i++)
		NBVStruct[i] = NULL;
    
	for(unsigned int index = STARTFROM; index <= (*obj).numvertices; index++)
	{
		temp = NBTStruct[index];
		while (temp->next != NULL)
		{
			node = (GLMnode*)malloc(sizeof(GLMnode));
			node->index = (*obj).triangles[temp->index].vindices[0];
			node->next  = NBVStruct[index];
			NBVStruct[index] = node;

			node = (GLMnode*)malloc(sizeof(GLMnode));
			node->index = (*obj).triangles[temp->index].vindices[1];
			node->next  = NBVStruct[index];
			NBVStruct[index] = node;

			node = (GLMnode*)malloc(sizeof(GLMnode));
			node->index = (*obj).triangles[temp->index].vindices[2];
			node->next  = NBVStruct[index];
			NBVStruct[index] = node;

			temp = temp->next;
		}
	}

	return NBVStruct;
} //end vertexList

/* Function: CalcCM
 * Description: Computes the center of mass of the model
 * Input: toggle - value used toggle between stable CM(0) / deformed CM(1) computation
 * Output: None
 */
void CalcCM(int toggle, phyzx *phyzxObj)
{
	point numerator;
	//double denominator = 0;
	
	memset( (void*)&numerator, 0, sizeof(numerator));

	switch(toggle)
	{
		case 0:
			for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
			{
				numerator.x += phyzxObj->mass[index] * phyzxObj->stable[index].x;
				numerator.y += phyzxObj->mass[index] * phyzxObj->stable[index].y;
				numerator.z += phyzxObj->mass[index] * phyzxObj->stable[index].z;
				
				//denominator += phyzxObj->mass[index];
				phyzxObj->totalMass += phyzxObj->mass[index];
			}
			phyzxObj->cmStable.x = numerator.x / phyzxObj->totalMass;
			phyzxObj->cmStable.y = numerator.y / phyzxObj->totalMass;
			phyzxObj->cmStable.z = numerator.z / phyzxObj->totalMass;
			break;

		case 1:
			for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
			{
				numerator.x += phyzxObj->mass[index] * phyzxObj->model->vertices[3*index];
				numerator.y += phyzxObj->mass[index] * phyzxObj->model->vertices[3*index+1];
				numerator.z += phyzxObj->mass[index] * phyzxObj->model->vertices[3*index+2];

				//denominator += phyzxObj->mass[index];
			}
			phyzxObj->cmDeformed.x = numerator.x / phyzxObj->totalMass;
			phyzxObj->cmDeformed.y = numerator.y / phyzxObj->totalMass;
			phyzxObj->cmDeformed.z = numerator.z / phyzxObj->totalMass;
			break;
	}
}


/* Function: CalcRelLoc
 * Description: Computes the relative location of each vertex wrt center of mass of the model
 * Input: toggle - value used toggle between stable CM(0) / deformed CM(1) computation
 * Output: None
 */
void CalcRelLoc(int toggle, phyzx *phyzxObj)
{

	switch(toggle)
	{
		case 0:
			for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
			{
				phyzxObj->relStableLoc[index].x = phyzxObj->stable[index].x - phyzxObj->cmStable.x;
				phyzxObj->relStableLoc[index].y = phyzxObj->stable[index].y - phyzxObj->cmStable.y;
				phyzxObj->relStableLoc[index].z = phyzxObj->stable[index].z - phyzxObj->cmStable.z;
			}
			break;

		case 1:
			for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
			{
				phyzxObj->relDeformedLoc[index].x = phyzxObj->model->vertices[3*index] - phyzxObj->cmDeformed.x;
				phyzxObj->relDeformedLoc[index].y = phyzxObj->model->vertices[3*index+1] - phyzxObj->cmDeformed.y;
				phyzxObj->relDeformedLoc[index].z = phyzxObj->model->vertices[3*index+2] - phyzxObj->cmDeformed.z;
			}
			break;
	}
}


/* Function: CalcApq
 * Description: Computes the Apq matrix which is the product of rotation and scaling matrices
 *				Apq = Summation(m * (p x qT))
 * Input: None
 * Output: None
 */
void CalcApq(phyzx *phyzxObj)
{
	matrix33 pqT, mpqT;

	memset( (void*)&phyzxObj->Apq, 0, sizeof(phyzxObj->Apq));
	memset( (void*)&pqT, 0, sizeof(pqT));
	memset( (void*)&mpqT, 0, sizeof(mpqT));
	
	for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
	{
		// Compute Relative Location
		phyzxObj->relDeformedLoc[index].x = phyzxObj->model->vertices[3*index] - phyzxObj->cmDeformed.x;
		phyzxObj->relDeformedLoc[index].y = phyzxObj->model->vertices[3*index+1] - phyzxObj->cmDeformed.y;
		phyzxObj->relDeformedLoc[index].z = phyzxObj->model->vertices[3*index+2] - phyzxObj->cmDeformed.z;

		matMult31(phyzxObj->relDeformedLoc[index], phyzxObj->relStableLoc[index], &pqT);		// p x qT
		matScalarMult33(phyzxObj->mass[index], pqT, &mpqT);									// m * (p X qT)
		matAdd33(phyzxObj->Apq, mpqT, &phyzxObj->Apq);											// Apq += m * (p X qT) 
	}
	return;
}


/* Function: CalcAqq
 * Description: Computes the Aqq matrix which is the product of rotation and scaling matrices
 *				Aqq = Summation(m * (q x qT))
 * Input: None
 * Output: None
 */
void CalcAqq(phyzx *phyzxObj)
{
	matrix33 qqT, mqqT, Aqq;

	memset( (void*)&phyzxObj->Aqq, 0, sizeof(phyzxObj->Aqq));
	memset( (void*)&qqT, 0, sizeof(qqT));
	memset( (void*)&mqqT, 0, sizeof(mqqT));
	memset( (void*)&Aqq, 0, sizeof(Aqq));
	
	for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
	{
		matMult31(phyzxObj->relStableLoc[index], phyzxObj->relStableLoc[index], &qqT);			// q x qT
		matScalarMult33(phyzxObj->mass[index], qqT, &mqqT);									// m * (q X qT)
		matAdd33(Aqq, mqqT, &Aqq);											// temp += m * (q X qT) 
	}
	matInverse33(Aqq, &phyzxObj->Aqq);												// Aqq = temp Inv
	return;
}

/* Function: CalcRotMat
 * Description: Computes the Rotation matrix
 *				R = Apq x SInv
 * Input: None
 * Output: None
 */
void CalcRotMat(phyzx *phyzxObj)
{
	matrix33 ApqTemp, ApqTrans, ApqSQRT, ApqInv;
	memset( (void*)&ApqTemp, 0, sizeof(ApqTemp));
	memset( (void*)&ApqTrans, 0, sizeof(ApqTrans));
	memset( (void*)&ApqSQRT, 0, sizeof(ApqSQRT));
	memset( (void*)&ApqInv, 0, sizeof(ApqInv));

	CalcApq(phyzxObj);													// Apq
	matTranspose33(phyzxObj->Apq, &ApqTrans);					// ApqT		
	matMult33(ApqTrans, phyzxObj->Apq, &ApqTemp);				// ApqT x Apq
	matSqrt33(ApqTemp, &ApqSQRT);								// S = sqrt(ApqT x Apq)
	matInverse33(ApqSQRT, &ApqInv);								// S = SInv
	matMult33(phyzxObj->Apq, ApqInv, &phyzxObj->R);				// R = Apq X SInv
}


/* Function: CalcGoalPos
 * Description: Computes the Goal position of the model
 *				g = R(xi0 - xcm0) + xcm
 * Input: None
 * Output: None
 */
void CalcGoalPos(phyzx *phyzxObj)
{
	point temp;

	memset( (void*)&temp, 0, sizeof(temp));

	for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
	{
		//pDIFFERENCE(phyzxObj->stable[index], phyzxObj->cmStable, temp);			// xi0 - xcm0
		matMult3331(phyzxObj->R, phyzxObj->relStableLoc[index], &temp);				// R(xi0 - xcm0)
		pSUM(temp, phyzxObj->cmDeformed, phyzxObj->goal[index]);					// g = R(xi0 - xcm0) + xcm
	}
}

/* Function: modEuler
 * Description: Modified Euler Integrator using Implicit and Explicit
 *				vi(t + h) = vi(t) + (ALPHA / h) * (gi(t) - xi(t)) + (h / mi) * Fext(t) 
 *				xi(t + h) = xi(t) + h * vi(t + h)
 * Input: None
 * Output: None
 */
void ModEuler(phyzx *phyzxObj, int mIndex, int deformMode)
{
	point vertex, velocity, extVel, position, velDamp;
	point vDiff, velTotal, newPos, temp;
	matrix R, matTemp;

	memset( (void*)&temp, 0, sizeof(temp));
	memset((void*)&extVel, 0, sizeof(point));
	memset((void*)&velocity, 0, sizeof(point));
	memset((void*)&position, 0, sizeof(point));
	memset((void*)&vDiff, 0, sizeof(point));
	memset((void*)&velTotal, 0, sizeof(point));
	memset((void*)&newPos, 0, sizeof(point));
	memset((void*)&phyzxObj->avgVel, 0, sizeof(point));

	matInit(&R, 0, 0);
	matInit(&matTemp, 0, 0);

	if (deformMode == 3)
		quadDeformRot(&R, phyzxObj);

	for (unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
	{
		if (deformMode == 3)
		{
			// Compute Quadratic Deformation Goal Positions
			matMult(R, phyzxObj->q[index], &matTemp);						// R(q)
			temp = matToPoint(matTemp);										// Data type conversion
			pSUM(temp, phyzxObj->cmDeformed, phyzxObj->goal[index]);		// g = R(q) + xcm
		} //end if
		else
		{
			// Compute Goal Positions
			matMult3331(phyzxObj->R, phyzxObj->relStableLoc[index], &temp);				// R(xi0 - xcm0)
			pSUM(temp, phyzxObj->cmDeformed, phyzxObj->goal[index]);					// g = R(xi0 - xcm0) + xcm
		} //end if

		vertex.x = phyzxObj->model->vertices[3*index];
		vertex.y = phyzxObj->model->vertices[3*index + 1];
		vertex.z = phyzxObj->model->vertices[3*index + 2];\

		if (stickyFloor == 1)
			if (vertex.y <= -WALLDIST)
				continue;

		// Add user force
		if (mIndex == iMouseModel && lMouseVal == 2 && objectName != -1)// && index == objectName)
		{
			//point uForce;
			/*GLMnode *node;
			node = NBVStruct[objectName];

			while (node->next != NULL)
			{
				pSUM(phyzxObj->extForce[node->index], userForce, phyzxObj->extForce[node->index]);
				node = node->next;
			} //end while*/
			/*if (index != objectName)
			{
				point extPos = vMake(phyzxObj->model->vertices[3*objectName], phyzxObj->model->vertices[3*objectName+1], phyzxObj->model->vertices[3*objectName+2]);
				double dist = vecLeng(extPos, vertex);
				//if (dist > 0.04)
				//{
					pMULTIPLY(userForce, (1.0/dist), uForce);
					pSUM(phyzxObj->extForce[index], uForce, phyzxObj->extForce[index]);
					//pDisp("user", userForce);
				//} //end if
				//else
				//{
					//pSUM(phyzxObj->extForce[index], userForce, phyzxObj->extForce[index]);
				//} //end else
			} //end if
			else
			{*/
				pSUM(phyzxObj->extForce[index], userForce, phyzxObj->extForce[index]);
			//} //end else
		} //end if

		// Explicit Euler Integrator for veloctiy -> vi(t + h)
		pDIFFERENCE(phyzxObj->goal[index], vertex, vDiff);												// gi(t) - xi(t)
		pMULTIPLY(vDiff, (phyzxObj->alpha / phyzxObj->h), velocity);									// vi(h) = (ALPHA / h) * (gi(t) - xi(t))
		pMULTIPLY(phyzxObj->extForce[index], (phyzxObj->h / phyzxObj->mass[index]), extVel);			// (h / mi) * Fext(t)
//		pMULTIPLY(phyzxObj->extForce[index], phyzxObj->h, extVel);			// (h / mi) * Fext(t)
		pSUM(velocity, extVel, velTotal);																// vi(h) = (ALPHA / h) * (gi(t) - xi(t)) + (h / mi) * Fext(t) 

		pSUM(phyzxObj->velocity[index], velTotal, phyzxObj->velocity[index]);							// vi(t + h) = vi(t) + vi(h)
		
		// Velocity Damping
		pMULTIPLY(phyzxObj->velocity[index], -phyzxObj->delta, velDamp);
		pSUM(phyzxObj->velocity[index], velDamp, phyzxObj->velocity[index]);

		// Implicity Euler Integrator for position
		pMULTIPLY(phyzxObj->velocity[index], phyzxObj->h, position);									// xi(h) = h * vi(t + h)
		pSUM(vertex, position, newPos);																// xi(t + h) = xi(t) + xi(h)

		// Store new position into data structure
		phyzxObj->model->vertices[3*index] = newPos.x;
		phyzxObj->model->vertices[3*index + 1] = newPos.y;
		phyzxObj->model->vertices[3*index + 2] = newPos.z;

		pSUM(phyzxObj->avgVel, phyzxObj->velocity[index], phyzxObj->avgVel);

		//if (objCollide)
			CheckForCollision(index, phyzxObj, mIndex);
	} //end for

	pMULTIPLY(phyzxObj->avgVel, 1.0 / phyzxObj->model->numvertices, phyzxObj->avgVel);

	delete[] R.data;
	delete[] matTemp.data;
} //end ModEuler()

/* Function: SphereCollisionDetection
 * Description: Check for model-model collisions
 * Input: p1 r1 p2 r2
 * Output: collided 1(true) / 0(false)
 */
int SphereCollisionDetection(point p1, point p2, double r1, double r2)
{
	double dist = vecLeng(p1, p2);

	if(dist > (r1+r2))
		return 0;
	else
		return 1;
}

/* Function: boundSphereToWallCollisionDetection
 * Description: Check for model-model collisions
 * Input: *cur - Model information
 * Output: collided 1(true) / 0(false)
 */
int boundSphereToWallCollisionDetection(pModel *cur)
{
	if (cur->cModel.x + cur->radius >= WALLDIST || cur->cModel.x - cur->radius <= -WALLDIST)
		return 1;
	if (cur->cModel.y + cur->radius >= WALLDIST || cur->cModel.y - cur->radius <= -WALLDIST)
		return 1;
	if (cur->cModel.z + cur->radius >= WALLDIST || cur->cModel.z - cur->radius <= -WALLDIST)
		return 1;
	return 0;
} //end boundSphereToWallCollisionDetection

/* Function: SphereCollisionResponse
 * Description: Perform model-model collision response
 * Input: temp - pointer to the current model structure
 * Output: 
 */
void SphereCollisionResponse(pModel *cur)
{
	pModel *temp;
	int collided = 0;

	temp = phyzxModels;

	while(temp->next != NULL)
	{
		if(cur->mIndex == temp->mIndex)
		{
			temp = temp->next;
			continue;
		}

		if(collided = SphereCollisionDetection(cur->cModel, temp->cModel, cur->radius, temp->radius))
		{
			objCollide = true;
			PenaltyPushBack(cur, temp);
		}
		else
			objCollide = false;

		temp = temp->next;
	}
}


/* Function: computeHooksForce
 * Description: Compute Hook's Law in 3D
 * Input: index - index of the vertex which collided 
 *		  wallP - point on the wall where the vertex is colliding
 * Output: Computed hooks force 
 */
point computeHooksForce(int index, point B, phyzx *phyzxObj)
{
	
	point L, unitV;
	double mag = 0;
	double restLength = 0, length;
	point hooksForce, A;
	
	memset( (void*)&L, 0, sizeof(L));
	memset( (void*)&unitV, 0, sizeof(unitV));
	memset( (void*)&hooksForce, 0, sizeof(hooksForce));
	memset( (void*)&A, 0, sizeof(A));

	A = vMake(phyzxObj->model->vertices[3*index], phyzxObj->model->vertices[3*index + 1], phyzxObj->model->vertices[3*index + 2]);
	pDIFFERENCE(A, B, L);
	pCPY(L, unitV);
	pNORMALIZE(unitV);
	//xxx
	/*length = A.y - B.y;
	unitV.x = 0;
	unitV.y = 1;
	unitV.z = 0;*/
	pMULTIPLY(unitV, -(phyzxObj->kWall) * length * phyzxObj->mass[index], hooksForce);
	//pMULTIPLY(unitV, -(phyzxObj->kWall) * length, hooksForce);

	return hooksForce;
}


/* Function: computeDampingForce
 * Description: Compute Damping in 3D
 * Input: index - index of the vertex which collided 
 *		  wallP - point on the wall where the vertex is colliding
 * Output: Computed damping force 
 */
point computeDampingForce(int index, point B, phyzx *phyzxObj)
{
	point L, unitV, vDiff, zero;
	double mag = 0, length;
	double dot = 0;
	point dampingForce, A;
	point v1, v2;	// to store the velocities of both the points connected to the spring
	
	memset( (void*)&L, 0, sizeof(L));
	memset( (void*)&unitV, 0, sizeof(unitV));
	memset( (void*)&vDiff, 0, sizeof(vDiff));
	memset( (void*)&dampingForce, 0, sizeof(dampingForce));
	memset( (void*)&v1, 0, sizeof(v1));
	memset( (void*)&v2, 0, sizeof(v2));
	memset( (void*)&A, 0, sizeof(A));
		
/*	A.x = phyzxObj->model->vertices[3*index];
	A.y = phyzxObj->model->vertices[3*index + 1];
	A.z = phyzxObj->model->vertices[3*index + 2];

	L.x = A.x - B.x;
	L.y = A.y - B.y;
	L.z = A.z - B.z;

	// Magnitude
	mag = sqrt((L.x * L.x) + (L.y * L.y) + (L.z * L.z));*/
	
	unitV.x = phyzxObj->velocity[index].x;
	unitV.y = phyzxObj->velocity[index].y;
	unitV.z = phyzxObj->velocity[index].z;

//	pNORMALIZE(unitV);

	// Difference in velocities

/*	v1.x = phyzxObj->velocity[index].x;
	v1.y = phyzxObj->velocity[index].y;
	v1.z = phyzxObj->velocity[index].z;*/

	// vDiff Dot L
	dot = (v1.x * L.x) + (v1.y * L.y) + (v1.z * L.z);
/*
	dampingForce.x = (-phyzxObj->dWall) * ( dot / mag ) * ( phyzxObj->mass[index] ) * (unitV.x);
	dampingForce.y = (-phyzxObj->dWall) * ( dot / mag ) * ( phyzxObj->mass[index] ) * (unitV.y);
	dampingForce.z = (-phyzxObj->dWall) * ( dot / mag ) * ( phyzxObj->mass[index] ) * (unitV.z);*/

	/*dampingForce.x = (-phyzxObj->dWall) * ( dot / mag ) * (unitV.x);
	dampingForce.y = (-phyzxObj->dWall) * ( dot / mag ) * (unitV.y);
	dampingForce.z = (-phyzxObj->dWall) * ( dot / mag ) * (unitV.z);*/

	dampingForce.x = (-phyzxObj->dWall) * ( phyzxObj->mass[index] ) * (unitV.x);
	dampingForce.y = (-phyzxObj->dWall) * ( phyzxObj->mass[index] ) * (unitV.y);
	dampingForce.z = (-phyzxObj->dWall) * ( phyzxObj->mass[index] ) * (unitV.z);
/*
	zero = vMake(0.0);
	A = vMake(phyzxObj->model->vertices[3*index], phyzxObj->model->vertices[3*index + 1], phyzxObj->model->vertices[3*index + 2]);
	pDIFFERENCE(A, B, L);
	pCPY(L, unitV);
	pNORMALIZE(unitV);
	pDIFFERENCE(phyzxObj->velocity[index], zero, v1);
	dot = dotProd(v1, L);
	pMULTIPLY(unitV, -(phyzxObj->dWall) * (dot/length), dampingForce);
*/	
	return dampingForce;
}	

/* Function: penaltyForce
 * Description: Computes the penalty force between two points.
 * Input: p - Coordinates of first point
 *        pV - Velocity of first point
 *        I - Intersection point
 *        V - Velocity of Intersection point
 *        kH - K value for Hook's Law
 *        kD - K value for damping force
 * Output: Penalty force vector
 */
point penaltyForce(point p, point pV, point I, point V, double kH, double kD)
{
	double mag, length, dot;
	point dist, hForce, dForce, pVel, vDiff, pForce;

	// Initialize force computation variables
	pDIFFERENCE(p, I, dist);
	pDIFFERENCE(pV, V, vDiff);
	dot = dotProd(vDiff, dist);

	// Compute Hooks Force
	pNORMALIZE(dist);
	pMULTIPLY(dist, -(kH * length), hForce);

	// Compute Damping Forces
	mag = length;
	pNORMALIZE(pV);
	pMULTIPLY(pV, (kD * (dot/length)), dForce);
	
	// Compute Penalty Force
	pSUM(hForce, dForce, pForce);

	return pForce;
} //end penaltyForce

/* Function: PenaltyPushBack
 * Description: Responds to the collision that is detected and performs penalty method for model spheres
 * Input: cur - current model structure
 *		  p2 - center of the other model
 *		  r1 - radius of the other model
 * Output: void
 */
void PenaltyPushBack(pModel *cur, pModel *next)
{
	point distCI, distIC, inter, velDir, cVel;
	double length;

	pDIFFERENCE(cur->cModel, next->cModel, inter);
	pNORMALIZE(inter);
	pCPY(next->pObj->avgVel, velDir);
	pNORMALIZE(velDir);
	pCPY(cur->pObj->avgVel, cVel);
	pNORMALIZE(cVel);

	pMULTIPLY(inter, next->radius, inter);
	
	for (unsigned int index = STARTFROM; index <= cur->pObj->model->numvertices; index++)
	{
		point cP = vMake(cur->pObj->model->vertices[3*index], cur->pObj->model->vertices[3*index + 1], cur->pObj->model->vertices[3*index + 2]);
		length = vecLeng(cP, inter);
		point pForce = penaltyForce(cP, cur->pObj->velocity[index], inter, velDir, cur->pObj->kSphere, cur->pObj->dSphere);

		// Add the forces to the collided vertex
		pMULTIPLY(pForce, (1.0 / (length * length)) * cur->pObj->mass[index], pForce);
		pSUM(cur->pObj->extForce[index] , pForce, cur->pObj->extForce[index]);

		point nP = vMake(next->pObj->model->vertices[3*index], next->pObj->model->vertices[3*index + 1], next->pObj->model->vertices[3*index + 2]);
		length = vecLeng(nP, inter);
		point nForce = penaltyForce(nP, next->pObj->velocity[index], inter, cVel, next->pObj->kSphere, next->pObj->dSphere);

		// Add the forces to the collided vertex
		pMULTIPLY(nForce, -(1.0 / (length * length)) * next->pObj->mass[index], nForce);
		pSUM(next->pObj->extForce[index], nForce, next->pObj->extForce[index]);
	}
}

/* Function: PenaltyPushBack
 * Description: Responds to the collision that is detected and performs penalty method
 * Input: index - index of the vertex which collided 
 *		  wallP - point on the wall where the vertex is colliding
 * Output: void
 */
void PenaltyPushBack(int index, point wallP, phyzx *phyzxObj)
{
	GLMnode *node;
	point hooksF, dampF, temp;
	
	memset( (void*)&hooksF, 0, sizeof(hooksF));
	memset( (void*)&dampF, 0, sizeof(dampF));

	hooksF = computeHooksForce(index, wallP, phyzxObj);
	dampF = computeDampingForce(index, wallP, phyzxObj);
	
	// Add the forces to the collided vertex
	phyzxObj->extForce[index].x += hooksF.x + dampF.x;
	phyzxObj->extForce[index].y += hooksF.y + dampF.y;
	phyzxObj->extForce[index].z += hooksF.z + dampF.z;
}


/* Function: CheckForCollision
 * Description: Checks for collision with the walls and invokes penalty method for collision response
 * Input: index - index of the vertex whose collision status is to be determined
 * Output: void
 */
void CheckForCollision(int index, phyzx *phyzxObj, int mIndex)
{

	point wallP, vertex, extPos, curPos, uForce;

	// Store vertex position
	vertex.x = phyzxObj->model->vertices[3*index];
	vertex.y = phyzxObj->model->vertices[3*index + 1];
	vertex.z = phyzxObj->model->vertices[3*index + 2];

	memset( (void*)&wallP, 0, sizeof(wallP));
    
	phyzxObj->extForce[index].x  = 0.0;
	phyzxObj->extForce[index].y  = gGravity;
	phyzxObj->extForce[index].z  = 0.0;


	if(vertex.x > WALLDIST)
	{	// collision with right face
		// RIGHT -> (2, y, z)
		wallP.x = -1.0;
		wallP.y = vertex.y;
		wallP.z = vertex.z;
		
		PenaltyPushBack(index, wallP, phyzxObj);
		memset( (void*)&wallP, 0, sizeof(wallP));
	}

	if(vertex.x < -WALLDIST)
	{	// collision with left face
		// LEFT -> (-2, y, z)
		wallP.x = 1.0;
		wallP.y = vertex.y;
		wallP.z = vertex.z;
		
		PenaltyPushBack(index, wallP, phyzxObj);
		memset( (void*)&wallP, 0, sizeof(wallP));
	}

	if(vertex.y > WALLDIST)
	{	// collision with top face
		// TOP -> (x, 2, z)
		wallP.x = vertex.x;
		wallP.y = -1.0;
		wallP.z = vertex.z;
		
		PenaltyPushBack(index, wallP, phyzxObj);
		memset( (void*)&wallP, 0, sizeof(wallP));
	}

	if(vertex.y <= -WALLDIST)
	{	// collision with bottom face
		// BOTTOM -> (x, -2, z)
		wallP.x = vertex.x;
		wallP.y = 1.0;
		wallP.z = vertex.z;
		
		PenaltyPushBack(index, wallP, phyzxObj);
		//phyzxObj->extForce[index] = vMake(0.0);
		memset( (void*)&wallP, 0, sizeof(wallP));
	}

	if(vertex.z > WALLDIST)
	{	// collision with back face
		// BACK -> (x, y, 2)
		wallP.x = vertex.x;
		wallP.y = vertex.y;
		wallP.z = -1.0;
		
		PenaltyPushBack(index, wallP, phyzxObj);
		memset( (void*)&wallP, 0, sizeof(wallP));
	}

	if(vertex.z < -WALLDIST)
	{	// collision with front face
		// FRONT -> (x, y, -2)
		wallP.x = vertex.x;
		wallP.y = vertex.y;
		wallP.z = 1.0;
		
		PenaltyPushBack(index, wallP, phyzxObj);
		memset( (void*)&wallP, 0, sizeof(wallP));
	}
}


/* Function: CallPerFrame
 * Description: All the computations are performed for the new frame to obtain the new goal position
 * Input: None
 * Output: None
 */
void CallPerFrame()
{
	pModel *temp;

	temp = phyzxModels;

	while(temp->next != NULL)
	{
		// Compute the center of mass
		CalcCM(1, temp->pObj);

		// Compute the relative positions of the model vertices  from center of mass
		//CalcRelLoc(1, temp->pObj);

		// Compute the Rotational matrix using Apq
		CalcRotMat(temp->pObj);

		if (temp->pObj->deformMode == 1)
			rigidBody(temp->pObj);  // Rigid Body Deformation
		else if (temp->pObj->deformMode == 2)
			linearDeform(temp->pObj);  // Linear Deformation

		// Compute the Goal position for the current frame
/*		if (temp->deformMode == 3)
			quadDeform(temp->pObj);  // Quadratic Deformation
		else
			CalcGoalPos(temp->pObj);*/

		// Time step using modified Euler
		/*if (boundSphereToWallCollisionDetection(temp) == 1)
			objCollide = true;
		else
			objCollide = false;*/
		ModEuler(temp->pObj, temp->mIndex, temp->pObj->deformMode);

		// Check for collision and perform the response action upon collision
		//CollisionDetectionAndResponse(temp);
		
		// Compute the center of the model with  the radius of the bounding sphere
		glmMeshGeometricParameters(temp->pObj->model, &temp->cModel.x, &temp->cModel.y, &temp->cModel.z, &temp->radius);

		// Compute the radius of the best bounding sphere around the model
		glmMeshRadius(temp->pObj->model,  temp->cModel.x, temp->cModel.y, temp->cModel.z, &temp->radius);

		SphereCollisionResponse(temp);

		temp = temp->next;
	}
}