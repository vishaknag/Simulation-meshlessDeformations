/* Source: quadratic
 * Description: Contains functions for Quadratic Deformations.
 */

#include "physics.h"

/* Function: quadRotMat
 * Description: Transforms the rotation matrix to a 3x9 matrix
 * Input: None
 * Output: None
 */
void quadRotMat(matrix *rot, phyzx *phyzxObj)
{
	matInit(rot, 3, 9);

	for (int row = 0; row < (*rot).row * (*rot).col; row+=(*rot).col)
		for (int col = 0; col < 3; col++)
			(*rot).data[row+col] = phyzxObj->R[(int)(row/(*rot).col)][col];
} //end quadRotMat


/* Function: calcQ
 * Description: Transforms the intial position to a 9x1 matrix
 * Input: None
 * Output: None
 */
void calcQ(point p, matrix *q)
{
	matInit(q, 9, 1);

	(*q).data[0] = p.x;
	(*q).data[1] = p.y;
	(*q).data[2] = p.z;

	(*q).data[3] = p.x * p.x;
	(*q).data[4] = p.y * p.y;
	(*q).data[5] = p.z * p.z;

	(*q).data[6] = p.x * p.y;
	(*q).data[7] = p.y * p.z;
	(*q).data[8] = p.z * p.x;
} //end calcQ

void calcTApq(phyzx *phyzxObj)
{
	matrix pqT, p, mpqT;

	matInit(&p, 3, 1);
	matInit(&phyzxObj->TApq, 3, 9);
	
	for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
	{
		p.data[0] = phyzxObj->relDeformedLoc[index].x;
		p.data[1] = phyzxObj->relDeformedLoc[index].y;
		p.data[2] = phyzxObj->relDeformedLoc[index].z;

		//calcQ(phyzxObj->relStableLoc[index], &q);
		//matTranspose(q, &qT);
		matMult(p, phyzxObj->qT[index], &pqT);					// p x qT
		matSMult(phyzxObj->mass[index], pqT, &mpqT);			// m * (p X qT)
		matAdd(phyzxObj->TApq, mpqT, &phyzxObj->TApq);			// Apq += m * (p X qT)  
	}

	delete[] pqT.data;
	delete[] p.data;
	delete[] mpqT.data;
} //end calcTApq

void calcTAqq(phyzx *phyzxObj)
{
	matrix qqT, mqqT, TAqqInv;

	matInit(&phyzxObj->TAqq, 9, 9);
	matInit(&TAqqInv, 9, 9);
	
	for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
	{
		calcQ(phyzxObj->relStableLoc[index], &phyzxObj->q[index]);
		matTranspose(phyzxObj->q[index], &phyzxObj->qT[index]);
		matMult(phyzxObj->q[index], phyzxObj->qT[index], &qqT);			// q x qT
		matSMult(phyzxObj->mass[index], qqT, &mqqT);					// m * (q X qT)
		matAdd(TAqqInv, mqqT, &TAqqInv);								// Aqq += m * (q X qT)  
	}

	matInverse(TAqqInv, &phyzxObj->TAqq);
	
	delete[] qqT.data;
	delete[] mqqT.data;
	delete[] TAqqInv.data;
} //end calcTApq

/* Function: quadDeformRot
 * Description: Calculates the rotational matrix for Quadratic Deformation.
 * Input: None
 * Output: None
 */
void quadDeformRot(matrix *R, phyzx *phyzxObj)
{
	matrix A, rot, bA, bR;

	calcTApq(phyzxObj);
	matMult(phyzxObj->TApq, phyzxObj->TAqq, &A);
	matSMult(phyzxObj->beta, A, &bA);

	quadRotMat(&rot, phyzxObj);
	matSMult((1.0 - phyzxObj->beta), rot, &bR);
	matAdd(bA, bR, R);

	delete[] A.data;
	delete[] rot.data;
	delete[] bA.data;
	delete[] bR.data;
} //end quadDeformRot

/* Function: quadDeform
 * Description: Calculates the rotational matrix for Quadratic Deformation.
 * Input: None
 * Output: None
 */
void quadDeform(phyzx *phyzxObj)
{
	point temp;
	matrix R, matTemp;

	quadDeformRot(&R, phyzxObj);

	// Calculate Goal Position with 3x9 Matrix R
	for(unsigned int index = STARTFROM; index <= phyzxObj->model->numvertices; index++)
	{
		//calcQ(phyzxObj->relStableLoc[index], &q);
		matMult(R, phyzxObj->q[index], &matTemp);						// R(q)
		temp = matToPoint(matTemp);										// Data type conversion
		pSUM(temp, phyzxObj->cmDeformed, phyzxObj->goal[index]);		// g = R(q) + xcm
	}

	delete[] R.data;
	delete[] matTemp.data;
} //end quadDeform