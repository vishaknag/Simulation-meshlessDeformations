/* Source: linear
 * Description: Contains functions for Linear Deformations.
 */

#include "physics.h"

/* Function: linearDeform
 * Description: Turns on Linear Deformation.
 * Input: None
 * Output: None
 */
void linearDeform(phyzx *phyzxObj)
{
	matrix33 rot, A, temp, nA;
	double det;

	memset( (void*)&A, 0, sizeof(A));
	memset( (void*)&nA, 0, sizeof(nA));
	memset( (void*)&rot, 0, sizeof(rot));
	memset( (void*)&temp, 0, sizeof(temp));

	matMult33(phyzxObj->Apq, phyzxObj->Aqq, &A);
	det = matDeterminant33(A);
	matScalarMult33(1.0 / pow(det, 1.0/3.0), A, &nA);
	matScalarMult33(phyzxObj->beta, nA, &temp);
	matScalarMult33(1.0 - phyzxObj->beta, phyzxObj->R, &rot);
	matAdd33(temp, rot, &phyzxObj->R);

} //end linearDeform