/* Author: Samuel Yung
 * Source: vector
 * Description: Contains vector mathematics functions.
 */

#include "vector.h"

/* Function: pArrayInit
 * Description: Allocates space for an array of points
 * Input: pA - Array of points
 *        size - Size of array
 * Output: Nothing
 */
void pArrayInit(struct pArray *pA, int size)
{
	// Initialize array of points
	(*pA).p = new point[size];
	for (int index = 0; index < size; index++)
	{
		(*pA).p[index].x = 0.0;
		(*pA).p[index].y = 0.0;
		(*pA).p[index].z = 0.0;
	} //end for
} //end pArrayInit

/* Function: pDisp
 * Description: Displays the coordinates of a point
 * Input: name - Name of point
 *        p - Coordinates of a point
 * Output: Nothing
 */
void pDisp(char *name, point p)
{
	printf("Point %s: (%0.2f, %0.2f, %0.2f)\n", name, p.x, p.y, p.z);
} //end pDisp

/* Function: vecDisp
 * Description: Displays the coordinates of a point
 * Input: p1 - Coordinates of a point
 * Output: returns the length of the vector
 */
void vecDisp(point p1)
{
	printf("( %0.2f, %0.2f, %0.2f )\n", p1.x, p1.y, p1.z);
} //end vecDisp

/* Function: vMake
 * Description: Creates a point data structure
 * Input: val - value of the point
 * Output: returns the vector data structure
 */
point vMake(float val)
{
	point vector;

	vector.x = val;
	vector.y = val;
	vector.z = val;

	return vector;
} //end vMake

/* Function: vMake
 * Description: Creates a point data structure
 * Input: x - X Coordinate of point
 *        y - Y Coordinate of point
 *        z - Z Coordinate of point
 * Output: returns the vector data structure
 */
point vMake(float x, float y, float z)
{
	point vector;

	vector.x = x;
	vector.y = y;
	vector.z = z;

	return vector;
} //end vMake

/* Function: vMake
 * Description: Creates a point data structure
 * Input: x - X Coordinate of point
 *        y - Y Coordinate of point
 *        z - Z Coordinate of point
 * Output: returns the vector data structure
 */
/*point vMake(double x, double y, double z)
{
	point vector;

	vector.x = x;
	vector.y = y;
	vector.z = z;

	return vector;
} //end vMake*/

/* Function: vMake
 * Description: Creates a point data structure
 * Input: v - Array of the point
 * Output: returns the vector data structure
 */
point vMake(float v[])
{
	point vector;

	vector.x = v[0];
	vector.y = v[1];
	vector.z = v[2];

	return vector;
} //end vMake

/* Function: vMake
 * Description: Creates a point data structure
 * Input: v - Array of the point
 * Output: returns the vector data structure
 */
point vMake(double v[])
{
	point vector;

	vector.x = v[0];
	vector.y = v[1];
	vector.z = v[2];

	return vector;
} //end vMake

/* Function: vComp
 * Description: Compares two points to determine if they are the same
 * Input: v1 - First point
 *        v2 - Second point
 * Output: returns true if points are the same, else returns false
 */
bool vComp(point v1, point v2)
{
	if ((v1.x != v2.x) || (v1.y != v2.y) || (v1.z != v2.z))
		return false;
	else
		return true;
} //end vComp

/* Function: vecDiff
 * Description: Calculates difference of two vectors.
 * Input: v1 - Coordinates of the first vector
 *        v2 - Coordinates of the second vector
 * Output: returns the length of the vector
 */
point vecDiff(point v1, double v2[3])
{
	point dest;

	dest.x = v1.x - v2[0];
	dest.y = v1.y - v2[1];
	dest.z = v1.z - v2[2];

	return dest;
} //end vecDiff

/* Function: vecDiff
 * Description: Calculates difference of two vectors.
 * Input: v1 - Coordinates of the first vector
 *        v2 - Coordinates of the second vector
 * Output: returns the length of the vector
 */
point vecDiff(double v1[3], point v2)
{
	point dest;

	dest.x = v1[0] - v2.x;
	dest.y = v1[1] - v2.y;
	dest.z = v1[2] - v2.z;

	return dest;
} //end vecDiff

/* Function: dotProd
 * Description: Calculates the dot product of two 3D vectors
 * Input: v1 - Coordinates of the first vector
 *        v2 - Coordinates of the second vector
 * Output: returns the dot product results of the two input vectors
 */
double dotProd(point v1, point v2)
{
	return (double)(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
} //end dotProd

/* Function: dotProd
 * Description: Calculates the dot product of two 3D vectors
 * Input: v1 - Coordinates of the first vector
 *        v2 - Coordinates of the second vector
 * Output: returns the dot product results of the two input vectors
 */
double dotProd(double v1[3], point v2)
{
	return (double)(v1[0] * v2.x + v1[1] * v2.y + v1[2] * v2.z);
} //end dotProd

/* Function: dotProd
 * Description: Calculates the dot product of two vectors
 * Input: v1 - Coordinates of the first vector
 *        v2 - Coordinates of the second vector
 * Output: returns the dot product results of the two input vectors
 */
double dotProd(double v1[], double v2[], int size)
{
	double result = 0.0;

	for (int i = 0; i < size; i++)
		result += v1[i] * v2[i];
	return result;
} //end dotProd

/* Function: vecLeng
 * Description: Calculates length of a vector betweem two points
 * Input: v1 - Coordinates of the first vector
 *        v2 - Coordinates of the second vector
 * Output: returns the length of the vector
 */
double vecLeng(point v1, point v2)
{
	point dest;

	pDIFFERENCE(v1, v2, dest);

	return sqrt(dest.x * dest.x + dest.y * dest.y + dest.z * dest.z);
} //end vecLeng