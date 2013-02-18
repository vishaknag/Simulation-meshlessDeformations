/* Source: matrix
 * Description: Contains functions that compute matrix calculations
 */

#include "render.h"
#include "matrix.h"
#include "eig3.h"

/* Function: matInit
 * Description: Allocates memory and initializes values of the sparse matrix
 * Input: mat - Sparse Matrix
 *        row - Dimensions of sparse matrix
 *        col - Dimensions of sparse matrix
 * Output: None
 */
void matInit(matrix *mat, int row, int col)
{
	if ((*mat).data != NULL)
		delete[] (*mat).data;

	(*mat).row = row;
	(*mat).col = col;
	(*mat).data = new double[row * col];
	memset((*mat).data, 0, sizeof(double)* row * col);
} //end matInit

/* Function: matDisp
 * Description: Displays the matrix
 * Input: name - name of matrix
 *        m - matrix
 * Output: None
 */
void matDisp(char *name, matrix m)
{
	int end;

	printf("Matrix Size: %d x %d\n", m.row, m.col);
	printf("Matrix %s = { \n ", name);
	if ((m.col == 1) || (m.row == 1))
	{
		if (m.col == 1)
			end = m.row;
		else
			end = m.col;
		for (int index = 0; index < end; index++)
			if (m.data[index] == 0.0)
				printf("0 ");
			else
				printf("%0.2f ", m.data[index]);
		printf("}\n");
	} //end if
	else
	{
		for (int row = 0; row < m.row * m.col; row+=m.col)
		{
			for (int col = 0; col < m.col; col++)
				if (m.data[row+col] == 0.0)
					printf("0 ");
				else
					printf("%0.2f ", m.data[row+col]);
			
			if ((row + m.col) >= (m.row * m.col))
				printf("}\n");
			else
				printf("\n ");
		} //end for
	} //end else
	printf("\n");
} //end matDisp

/* Function: matDisp
 * Description: Displays the 3x3 matrix
 * Input: name - Name of matrix
 *        m - 3x3 matrix
 * Output: None
 */
void matDisp33(char *name, matrix33 m)
{
	printf("Matrix %s = { \n ", name);

	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 3; col++)
			printf("%0.3f ", m[row][col]);
		printf("\n ");
	} // end for

	printf("}\n\n");
} //end matDisp33

/* Function: matToPoint
 * Description: Coverts a matrix data type to point data type
 * Input: mat - Input 3x1 matrix
 * Output: Point data type of the input matrix
 */
point matToPoint(matrix mat)
{
	point p;

	p.x = mat.data[0];
	p.y = mat.data[1];
	p.z = mat.data[2];

	return p;
} //end matToPoint

/* Function: pointToMat
 * Description: Coverts a matrix data type to point data type
 * Input: p - Input point coordinates
 *        mat - Output 3x1 matrix
 * Output: None
 */
void pointToMat(point p, matrix *mat)
{
	matInit(mat, 3, 1);

	(*mat).data[0] = p.x;
	(*mat).data[1] = p.y;
	(*mat).data[2] = p.z;
} //end pointToMat

/* Function: matToMat33
 * Description: Coverts a matrix data type to matrix33 data type
 * Input: m1 - Input 3x3 matrix
 *        mat - Output 3x3 matrix33 data type
 * Output: None
 */
void matToMat33(matrix m1, matrix33 *mat)
{
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			(*mat)[row][col] = m1.data[row*m1.col+col];
} //end matToMat33

/* Function: matToMat
 * Description: Coverts a matrix data type to matrix33 data type
 * Input: m1 - Input 3x3 matrix33
 *        mat - Output 3x3 matrix data type
 * Output: None
 */
void matToMat(matrix33 m1, matrix *mat)
{
	matInit(mat, 3, 3);

	for (int row = 0; row < (*mat).row * (*mat).col; row+=(*mat).col)
		for (int col = 0; col < (*mat).col; col++)
			(*mat).data[row*(*mat).col+col] = m1[(int)(row/(*mat).col)][col];
} //end matToMat

/* Function: matCopy33
 * Description: Copy on matrix into another
 * Input: m1 - 3x3 input matrix
 *        mat - resulting 3x3 matrix pointer
 * Output: None
 */
void matCopy33(matrix33 m1, matrix33 *mat)
{
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			(*mat)[row][col] = m1[row][col];
} //end matCopy33

/* Function: matCopy
 * Description: Copy matrix data into another matrix
 * Input: mat1 - input first matrix
 *        mat - Resulting matrix
 * Output: None
 */
void matCopy(matrix mat1, matrix *mat)
{
	matInit(mat, mat1.row, mat1.col);

	for (int index = 0; index < (*mat).row * (*mat).col; index++)
		(*mat).data[index] = mat1.data[index];
} //end matCopy

/* Function: matComp
 * Description: Compares two matrix to determine if they are equal
 * Input: mat1 - Input first matrix
 *        mat2 - Input second matrix
 * Output: None
 */
bool matComp(matrix mat1, matrix mat2)
{
	if (mat1.col != mat2.col || mat1.row != mat2.row)
		return false;

	if (mat1.data != mat2.data)
		return false;

	for (int index = 0; index < (mat1.row * mat1.col); index++)
		if (mat1.data[index] != mat2.data[index])
			return false;

	return true;
} //end matComp

/* Function: matMult31
 * Description: Computes matrix multiplication of 3x1 * 1x3
 * Input: m1 - 3x1 input matrix
		  m2 - 1x3 input matrix
		  mat - resulting 3x3 matrix pointer
 * Output: None
 */
void matMult31(point m1, point m2, matrix33 *mat)
{
	(*mat)[0][0] = m1.x * m2.x;
	(*mat)[0][1] = m1.x * m2.y;
	(*mat)[0][2] = m1.x * m2.z;

	(*mat)[1][0] = m1.y * m2.x;
	(*mat)[1][1] = m1.y * m2.y;
	(*mat)[1][2] = m1.y * m2.z;

	(*mat)[2][0] = m1.z * m2.x;
	(*mat)[2][1] = m1.z * m2.y;
	(*mat)[2][2] = m1.z * m2.z;
} //end matMult31


/* Function: matMult3331
 * Description: Computes the multiplication of a 3x3 matrix with a vector
 * Input: m1 - 3x3 input matrix
 *        v1 - vector
 *        mat - resulting vector
 * Output: None
 */
void matMult3331(matrix33 m1, point v1, point *vec)
{
	/*(*vec).x = v1.x * m1[0][0] + v1.y * m1[0][1] + v1.z * m1[0][2];
	(*vec).y = v1.x * m1[1][0] + v1.y * m1[1][1] + v1.z * m1[1][2];
	(*vec).z = v1.x * m1[2][0] + v1.y * m1[2][1] + v1.z * m1[2][2];*/

	(*vec).x = dotProd(m1[0], v1);
	(*vec).y = dotProd(m1[1], v1);
	(*vec).z = dotProd(m1[2], v1);
} //end matMult3331

/* Function: matMult
 * Description: Computes a matrix multiplication with a matrix
 * Input: mat1 - First matrix
 *        mat2 - Second matrix
 *        mat - Resulting matrix
 * Output: None
 */
void matMult(matrix mat1, matrix mat2, matrix *mat)
{
	matrix matRow, matCol;

	if (matComp(mat1, *mat) || matComp(mat2, *mat))
		return; // Can not use matrix used in multiplication as the resulting matrix

	matInit(mat, mat1.row, mat2.col);
	matInit(&matRow, 1, mat1.col);
	matInit(&matCol, 1, mat2.row);

	for (int row = 0; row < (*mat).row * (*mat).col; row+=(*mat).col)
	{
		for (int i = 0; i < mat1.col; i++)
			matRow.data[i] = mat1.data[(int)(row/(*mat).col)*mat1.col+i];

		for (int col = 0; col < (*mat).col; col++)
		{
			for (int j = 0; j < mat2.row; j++)
				matCol.data[j] = mat2.data[j*mat2.col+col];
			(*mat).data[row+col] = dotProd(matRow.data, matCol.data, mat2.row);
		} //end for
	} //end for

	delete[] matRow.data;
	delete[] matCol.data;
} //end matMult

/* Function: matSMult
 * Description: Computes a matrix multiplication with a scalar
 * Input: s - scalar
 *        mat1 - input matrix
 *        mat - Resulting vector
 * Output: None
 */
void matSMult(double s, matrix mat1, matrix *mat)
{
	if (!matComp(mat1, *mat))
		matInit(mat, mat1.row, mat1.col);

	for (int index = 0; index < (*mat).row * (*mat).col; index++)
		(*mat).data[index] = s * mat1.data[index];
} //end matSMult

/* Function: matScalarMult33
 * Description: Computes scalar multiplication of a 3x3
 * Input: scalar - scalar value
 *        m1 - 3x3 input matrix
 *        mat - resulting 3x3 matrix pointer
 * Output: None
 */
void matScalarMult33(double scalar, matrix33 m1, matrix33 *mat)
{
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			(*mat)[row][col] = scalar * m1[row][col];
}

/* Function: matAdd33
 * Description: Computes addition of 3x3 matrices
 * Input: m1 - 3x3 input matrix
 *        m2 - 3x3 input matrix
 *        mat - resulting 3x3 matrix pointer
 * Output: None
 */
void matAdd33(matrix33 m1, matrix33 m2, matrix33 *mat)
{
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			(*mat)[row][col] = m1[row][col] + m2[row][col];
}

/* Function: matAdd
 * Description: Computes a matrix addition with another matrix
 * Input: mat1 - input first matrix
 *        mat2 - input second matrix
 *        mat - Resulting matrix
 * Output: None
 */
void matAdd(matrix mat1, matrix mat2, matrix *mat)
{
	if (!matComp(mat1, *mat) && !matComp(mat2, *mat))
		matInit(mat, mat1.row, mat1.col);

	for (int index = 0; index < (*mat).row * (*mat).col; index++)
		(*mat).data[index] = mat1.data[index] + mat2.data[index];
} //end matAdd

/* Function: matSub
 * Description: Computes a matrix subtraction with another matrix
 * Input: mat1 - input first matrix
 *        mat2 - input second matrix
 *        mat - Resulting matrix
 * Output: None
 */
void matSub(matrix mat1, matrix mat2, matrix *mat)
{
	if (!matComp(mat1, *mat) && !matComp(mat2, *mat))
		matInit(mat, mat1.row, mat1.col);

	for (int index = 0; index < (*mat).row * (*mat).col; index++)
		(*mat).data[index] = mat1.data[index] - mat2.data[index];
} //end matSub

/* Function: matTranspose33
 * Description: Computes the transpose of a 3x3 matrix
 * Input: m1 - 3x3 input matrix
 *        mat - resulting transpose of the 3x3 matrix
 * Output: None
 */
void matTranspose33(matrix33 m1, matrix33 *mat)
{
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			(*mat)[col][row] = m1[row][col];
}

/* Function: matTranspose
 * Description: Computes the transpose of a matrix
 * Input: mat1 - Input matrix
 *        mat - Resulting transpose of the matrix
 * Output: None
 */
void matTranspose(matrix mat1, matrix *mat)
{
	if (!matComp(mat1, *mat))
	{
		matInit(mat, mat1.col, mat1.row);

		for (int row = 0; row < (*mat).row * (*mat).col; row+=(*mat).col)
			for (int col = 0; col < (*mat).col; col++)
				(*mat).data[row+col] = mat1.data[(col*mat1.col) + (int)(row/(*mat).col)];
	} //end if
	else
	{
		matrix matTemp;

		matCopy(mat1, &matTemp);

		matInit(mat, mat1.col, mat1.row);

		for (int row = 0; row < (*mat).row * (*mat).col; row+=(*mat).col)
			for (int col = 0; col < (*mat).col; col++)
				(*mat).data[row+col] = matTemp.data[(col*mat1.col) + (int)(row/(*mat).col)];

		delete[] matTemp.data;
	} //end else

	
} //end sMatTranspose

/* Function: matDeterminant33
 * Description: Computes the Determinant of a 3x3 matrix
 * Input: m1 - 3x3 input matrix
 *        mat - resulting Determinant of the 3x3 matrix
 * Output: None
 */
double matDeterminant33(matrix33 m)
{
	double term1 = 0, term2 = 0, term3 = 0, result = 0;

	term1 = m[0][0] * (m[1][1]*m[2][2] - m[1][2]*m[2][1]);
	term2 = m[0][1] * (m[1][2]*m[2][0] - m[2][2]*m[1][0]);
	term3 = m[0][2] * (m[1][0]*m[2][1] - m[1][1]*m[2][0]);

	result = term1+term2+term3;

	return result;
}

/* Function: matCofactors33
 * Description: Computes the cofactors of a 3x3 matrix
 * Input: m1 - 3x3 input matrix
 *        mat - resulting cofactors of the 3x3 matrix
 * Output: None
 */
void matCofactors33(matrix33 m1, matrix33 *mat)
{
	(*mat)[0][0] = m1[1][1]*m1[2][2] - m1[1][2]*m1[2][1];
	(*mat)[0][1] = -1.0*(m1[1][0]*m1[2][2] - m1[1][2]*m1[2][0]);
	(*mat)[0][2] = m1[1][0]*m1[2][1] - m1[1][1]*m1[2][0];

	(*mat)[1][0] = -1.0*(m1[0][1]*m1[2][2] - m1[0][2]*m1[2][1]);
	(*mat)[1][1] = m1[0][0]*m1[2][2] - m1[0][2]*m1[2][0];
	(*mat)[1][2] = -1.0*(m1[0][0]*m1[2][1] - m1[0][1]*m1[2][0]);

	(*mat)[2][0] = m1[0][1]*m1[1][2] - m1[0][2]*m1[1][1];
	(*mat)[2][1] = -1.0*(m1[0][0]*m1[1][2] - m1[0][2]*m1[1][0]);
	(*mat)[2][2] = m1[0][0]*m1[1][1] - m1[0][1]*m1[1][0];
} //end matCofactors33

/* Function: matInverse33
 * Description: Computes the Inverse of a 3x3 matrix
 * Input: m1 - 3x3 input matrix
 *        mat - resulting Inverse of the 3x3 matrix
 * Output: None
 */
void matInverse33(matrix33 m1, matrix33 *mat)
{
	double detM = 0;
	matrix33 temp, temp1;

	memset( (void*)&temp, 0, sizeof(temp));
	memset( (void*)&temp1, 0, sizeof(temp1));

	detM = 1.0 / matDeterminant33(m1);
	matCofactors33(m1, &temp);
	matTranspose33(temp, &temp1);
	matScalarMult33(detM, temp1, mat);
}

/* Function: matInverse
 * Description: Computes the Inverse of a matrix using GSL LU
 * Input: m1 - 3x3 input matrix
 *        mat - resulting Inverse of the 3x3 matrix
 * Output: None
 */
void matInverse(matrix m1, matrix *mat)
{
	int s=0;

	if (!matComp(m1, *mat))
		matInit(mat, m1.row, m1.col);

	gsl_matrix_view m = gsl_matrix_view_array(m1.data, m1.row, m1.col);

    /* calculate the inverse */
	gsl_matrix *inverse = gsl_matrix_alloc(m1.row, m1.col);
	gsl_permutation*perm = gsl_permutation_alloc(m1.row);
    
    gsl_linalg_LU_decomp(&m.matrix, perm, &s);
	gsl_linalg_LU_invert(&m.matrix, perm, inverse);

	for(int i = 0; i < ((*mat).row * (*mat).col); i++)
		(*mat).data[i] = inverse->data[i];
}

/* Function: matMult33
 * Description: Computes the Multiplication of two 3x3 matrices
 * Input: m1 - 3x3 input matrix
 *		  m2 - 3x3 input matrix
 *        mat - resulting Product of two 3x3 matrices
 * Output: None
 */
void matMult33(matrix33 m1, matrix33 m2, matrix33 *mat)
{
	point curCol;
	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			 curCol.x = m2[0][col];
			 curCol.y = m2[1][col];
			 curCol.z = m2[2][col];

			 (*mat)[row][col] = dotProd(m1[row], curCol);
		}
	}
}

/* Function: vecToMat
 * Description: Turns a vector into a 3x3 matrix
 * Input: v1 - vector
 *        mat - resulting 3x3 matrix
 * Output: None
 */
void vecToMat(point v1, matrix33 *mat)
{
	(*mat)[0][0] = v1.x;
	(*mat)[0][1] = v1.y;
	(*mat)[0][2] = v1.z;

	(*mat)[1][0] = v1.x;
	(*mat)[1][1] = v1.y;
	(*mat)[1][2] = v1.z;

	(*mat)[2][0] = v1.x;
	(*mat)[2][1] = v1.y;
	(*mat)[2][2] = v1.z;
} //end vecToMat

/* Function: colSwap
 * Description: Swaps first column with last column of a matrix
 * Input: m1 - 3x3 input matrix
 *        mat - resulting 3x3 matrix
 * Output: None
 */
void colSwap(matrix33 m1, matrix33 *mat)
{
	(*mat)[0][0] = -m1[0][2];
	(*mat)[0][1] = m1[0][1];
	(*mat)[0][2] = -m1[0][0];

	(*mat)[1][0] = -m1[1][2];
	(*mat)[1][1] = m1[1][1];
	(*mat)[1][2] = -m1[1][0];

	(*mat)[2][0] = -m1[2][2];
	(*mat)[2][1] = m1[2][1];
	(*mat)[2][2] = -m1[2][0];
} //end colSwap

/* Function: matSqrt33
 * Description: Computes the Inverse of the Square Root of a 3x3 matrix
 * Input: m1 - 3x3 input matrix
 *        mat - resulting Inverse of the Square Root of the 3x3 matrix
 * Output: None
 */
void matSqrt33(matrix33 m1, matrix33 *mat)
{
	matrix33 eigVec, eigVecTrans, temp, matDiag;
	double eigVal[3];

	memset( (void*)&eigVec, 0, sizeof(eigVec));
	memset( (void*)&eigVecTrans, 0, sizeof(eigVecTrans));
	memset( (void*)&temp, 0, sizeof(temp));
	memset( (void*)&eigVal, 0, sizeof(eigVal));

	eigen_decomposition(m1, eigVec, eigVal);

	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
		{
			if (row != col)
				matDiag[row][col] = 0;
			else
				matDiag[row][col] = sqrt(eigVal[row]);
		} //end for

	matTranspose33(eigVec, &eigVecTrans);
	matMult33(eigVec, matDiag, &temp);	
	matMult33(temp, eigVecTrans, mat);

} //end matSqrt33

/* Function: matTest
 * Description: Modifiable function for testing matrix calculations
 * Input: None
 * Output: None
 */
void matTest()
{
	matrix33 m1, m2, m3;
	matrix m4, m5, m6;
	point v1, v2, v3;
	double test = 0, count = 1.0, step = 81;

	matInit(&m4, 3, 3);
	matInit(&m5, 9, 9);

	v1 = vMake(2, 5, 6);
	v2 = vMake(3, 4, 2);
	v3 = vMake(5);
	for (int row = 0; row < 3; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			m2[row][col] = 4;
			m3[row][col] = 7;
		} //end for
	} //end for

	m1[0][0] = 7;
	m1[0][1] = 4;
	m1[0][2] = 3;
	m1[1][0] = 2;
	m1[1][1] = 6;
	m1[1][2] = 4;
	m1[2][0] = 4;
	m1[2][1] = 3;
	m1[2][2] = 9;

	m2[0][0] = 9;
	m2[0][1] = 4;
	m2[0][2] = 1;
	m2[1][0] = 4;
	m2[1][1] = 9;
	m2[1][2] = 4;
	m2[2][0] = 1;
	m2[2][1] = 4;
	m2[2][2] = 9;

	m4.data[0] = 1;
	m4.data[1] = 2;
	m4.data[2] = 5;
	m4.data[3] = 4;

	m4.data[4] = 3;
	m4.data[5] = 4;
	m4.data[6] = 6;
	m4.data[7] = 5;

	m4.data[8] = 4;
	/*m4.data[9] = 3;
	m4.data[10] = 8;
	m4.data[11] = 3;

	m4.data[12] = 1;
	m4.data[13] = 9;
	m4.data[14] = 8;
	m4.data[15] = 4;*/

	for (int row = 0; row < m5.row * m4.col; row++)
		m5.data[row] = step--;

	//pDisp("V1", v1);
	//pDisp("V2", v2);
	//pDisp("V3", v3);
	//matDisp33("M1", m1);
	//matDisp33("M2", m2);
	//matDisp33("M3", m3);
	matDisp("M4", m4);
	//matDisp("M5", m5);
	//matDisp("M6", m6);

	//matMult31(v1, v2, &m3);
	//matDisp33("vMult", m3);
	
	//matMult3331(m1, v1, &v3);
	//pDisp("MatVMult", v3);

	//matScalarMult33(5, m1, &m3);
	//matDisp33("MatSMult", m3);

	//matAdd33(m1, m2, &m3);
	//matDisp33("mAdd", m3);

	//matTranspose33(m1, &m3);
	//matDisp33("Trans", m3);

	//test = matDeterminant33(m1);
	//printf("Det: %0.5f\n", test);

	//matCofactors33(m1, &m3);
	//matDisp33("Cofactors", m3);

	//matScalarMult33(1.0/test, m3, &m3);
	//matDisp33("MatSMult", m3);

	//matInverse33(m1, &m3);
	//matDisp33("Inverse", m3);

	//matMult33(m1, m2, &m3);
	//matDisp33("mMult", m3);

	/*matDisp33("M2", m2);
	matSqrt33(m2, &m3);
	matDisp33("sqrt", m3);

	matMult33(m3, m3, &m1);
	matDisp33("M2*M2", m1);*/

	//matInverse33(m3, &m2);
	//matMult33(m3, m2, &m1);
	//matDisp33("Mat", m1);

	//matMult(m4, m5, &m6);
	//matDisp("Mult", m6);

	//matInverse(m4, &m6);
	//matDisp("Inverse", m6);

	matToMat33(m4, &m3);
	matDisp33("Mat33", m3);
} //end matTest()