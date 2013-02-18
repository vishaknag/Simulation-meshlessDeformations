/* Header: matrix
 * Description: Header file for matrix calculations
 */

#ifndef _MATRIX_H_
#define _MATRIX_H_

typedef double matrix33[3][3];

struct matrix
{
	int row;
	int col;
	double *data;

	matrix(): row(0), col(0), data(NULL) {};
};

void matInit(matrix *mat, int row, int col);
void matDisp(char *name, matrix m);
void matDisp33(char *name, matrix33 m);
point matToPoint(matrix mat);
void pointToMat(point p, matrix *mat);
void matToMat33(matrix m1, matrix33 *mat);
void matToMat(matrix33 m1, matrix *mat);
void matCopy33(matrix33 m1, matrix33 *mat);
void matCopy(matrix mat1, matrix *mat);
bool matComp(matrix mat1, matrix mat2);
void matMult31(point m1, point m2, matrix33 *mat);
void matMult3331(matrix33 m1, point v1, point *vec);
void matMult(matrix mat1, matrix mat2, matrix *mat);
void matSMult(double s, matrix mat1, matrix *mat);
void matScalarMult33(double scalar, matrix33 m1, matrix33 *mat);
void matAdd33(matrix33 m1, matrix33 m2, matrix33 *mat);
void matAdd(matrix mat1, matrix mat2, matrix *mat);
void matSub(matrix mat1, matrix mat2, matrix *mat);
void matTranspose33(matrix33 m1, matrix33 *mat);
void matTranspose(matrix mat1, matrix *mat);
double matDeterminant33(matrix33 m);
void matCofactors33(matrix33 m1, matrix33 *mat);
void matInverse33(matrix33 m1, matrix33 *mat);
void matInverse(matrix m1, matrix *mat);
void matMult33(matrix33 m1, matrix33 m2, matrix33 *mat);
void vecToMat(point v1, matrix33 *mat);
void colSwap(matrix33 m1, matrix33 *mat);
void matSqrt33(matrix33 m1, matrix33 *mat);
void matTest();

#endif
