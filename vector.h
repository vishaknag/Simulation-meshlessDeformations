/* Author: Samuel Yung
 * Header: vector
 * Description: Header file for vector mathematics
 */

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdio.h>
#include <math.h>

/*** Variables ***/
// data structure for a point
struct point
{
	double x;
	double y;
	double z;
};

// data structure for array of points
struct pArray
{
	point *p;
};

/***********************************  VECTOR MATHEMATICS ****************************************/

// computes crossproduct of three vectors, which are given as points
// struct point vector1, vector2, dest
// result goes into dest
#define CROSSPRODUCTp(vector1,vector2,dest)\
  CROSSPRODUCT( (vector1).x, (vector1).y, (vector1).z,\
                (vector2).x, (vector2).y, (vector2).z,\
                (dest).x, (dest).y, (dest).z )

// computes crossproduct of three vectors, which are specified by floating-point coordinates
// double x1,y1,z1,x2,y2,z2,x,y,z
// result goes into x,y,z
#define CROSSPRODUCT(x1,y1,z1,x2,y2,z2,x,y,z)\
\
  x = (y1) * (z2) - (y2) * (z1);\
  y = (x2) * (z1) - (x1) * (z2);\
  z = (x1) * (y2) - (x2) * (y1)

// normalizes vector dest
// struct point dest
// result returned in dest
// must declare a double variable called 'length' somewhere inside the scope of the NORMALIZE macrp
// macro will change that variable
#define pNORMALIZE(dest)\
\
  length = sqrt((dest).x * (dest).x + (dest).y * (dest).y + (dest).z * (dest).z);\
  (dest).x /= length;\
  (dest).y /= length;\
  (dest).z /= length;

// copies vector source to vector dest
// struct point source,dest
#define pCPY(source,dest)\
\
  (dest).x = (source).x;\
  (dest).y = (source).y;\
  (dest).z = (source).z;

// sums points src1 and src2 to dest
// struct point src1,src2,dest
#define pSUM(src1,src2,dest)\
\
  (dest).x = (src1).x + (src2).x;\
  (dest).y = (src1).y + (src2).y;\
  (dest).z = (src1).z + (src2).z;

// dest = src2 - src1
// struct point src1,src2,dest
#define pDIFFERENCE(src1,src2,dest)\
\
  (dest).x = (src1).x - (src2).x;\
  (dest).y = (src1).y - (src2).y;\
  (dest).z = (src1).z - (src2).z;

// mulitplies components of point src by scalar and returns the result in dest
// struct point src,dest
// double scalar
#define pMULTIPLY(src,scalar,dest)\
\
  (dest).x = (src).x * (scalar);\
  (dest).y = (src).y * (scalar);\
  (dest).z = (src).z * (scalar);

/*** Functions ***/
void pArrayInit(struct pArray *pA, int size);
void pDisp(char *name, point p);
void vecDisp(point p1);
point vMake(float val);
point vMake(float x, float y, float z);
//point vMake(double x, double y, double z);
point vMake(float v[]);
point vMake(double v[]);
bool vComp(point v1, point v2);
point vecDiff(point v1, double v2[3]);
point vecDiff(double v1[3], point v2);
double dotProd(point v1, point v2);
double dotProd(double v1[3], point v2);
double dotProd(double v1[], double v2[], int size);
double vecLeng(point v1, point v2);

#endif