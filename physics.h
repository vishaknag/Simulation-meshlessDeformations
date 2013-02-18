/* Header: physics
 * Description: Header file for physics calculations
 */

#ifndef _PHYSICS_H_
#define _PHYSICS_H_

#include "render.h"
#include "matrix.h"
#include <iostream>

#define STARTFROM 1
#define WALLDIST 1.9985
#define PENETRATE 2.0015

//6.0     0.006
class phyzx
{
public:
		double h;					// timestep
		int n;						// display every nth timepoint  
		int deformMode;				// Deformation mode Basic Shapematching / Linear / Rigid / Quadratic
		GLMmodel *model;			// Model information
		point *stable;				// Initial vertices position
		point *goal;				// Final positions of each vertex due to shapematching
		double *mass;				// Masses of each model vertex
		double totalMass;			// The total mass of the object model
		point *velocity;			// Current velocity values of each model vertex
		point *extForce;			// External force values	
		double alpha;				// Stiffness [0..1]
		double beta;				// Linear and Quadratic deformation value
		double delta;				// Velocity Damping
		point cmStable;				// Center of mass in stable state
		point cmDeformed;			// Center of mass in deformed position
		point *relStableLoc;		// Relative location of each model vertex 
									// from the Center of mass in stable state
		point *relDeformedLoc;		// Relative Location of each model vertex
									// from the Center of mass in Deformed state
		point avgVel;				// velocity of the object model 
		matrix33 Apq;				// 3x3 matrix to store Apq
		matrix33 Aqq;				// 3x3 matrix to store Aqq
		matrix33 R;					// 3x3 Rotation matrix 
		matrix TApq;				// 9x9 matrix
		matrix TAqq;				// 9x9 matrix
		matrix *q;					// Relative location of each model vertex for quadratic deformation
		matrix *qT;					// Relative location of each model vertex transpose
		double kWall;				// Hooks law co-efficient
		double dWall;				// Damping co-efficient
		double kSphere;				// Hooks co-efficient for models bounding sphere collision
		double dSphere;				// Damping co-efficient for models bounding sphere collision
		double *triAreas;			// Areas of all the triangles in the model
		double surArea;				// Surface area of the model
		
		GLMnode **NBTStruct;		// list of lists having triangles for every vertex of the model
		GLMnode **NBVStruct;		// list of lists having adjacent vertices of every vertex in the model

		phyzx::phyzx();
};

struct pModel
{
	phyzx *pObj;
	point translate;
	int mIndex;
	char file[50];
	point cModel;
	double radius;
	struct pModel *next;
};
extern struct pModel *phyzxModels;

void phyzxInit(phyzx *phyzxObj);
double AreaOfTri(point A, point B, point C);
void compMass(GLMnode **NBTStruct, phyzx *phyzxObj);
void filterNBV(GLMmodel *obj, GLMnode **NBVStruct);
void delNode(GLMnode *current, GLMnode *next);
GLMnode ** vertexList(GLMmodel *obj, GLMnode **NBTStruct, double radius);
void CalcCM(int toggle, phyzx *phyzxObj);
void CalcRelLoc(int toggle, phyzx *phyzxObj);
void CalcApq(phyzx *phyzxObj);
void CalcAqq(phyzx *phyzxObj);
void CalcRotMat(phyzx *phyzxObj);
void CalcGoalPos(phyzx *phyzxObj);
void ModEuler(phyzx *phyzxObj, int mIndex, int deformMode);
int SphereCollisionDetection(point p1, point p2, double r1, double r2);
void SphereCollisionResponse(pModel *cur);
point computeHooksForce(int index, point B, phyzx *phyzxObj, bool penetrate);
point computeDampingForce(int index, point B, phyzx *phyzxObj, bool penetrate);
point penaltyForce(point p, point pV, point I, point V, double kH, double kD);
void PenaltyPushBack(pModel *cur, pModel *next);
void PenaltyPushBack(int index, point wallP, phyzx *phyzxObj, bool penetrate);
//void CheckForCollision(int index, pModel *temp);
void CheckForCollision(int index, phyzx *phyzxObj, int mIndex);
void CallPerFrame();

void defaultDeform(phyzx *phyzxObj);
void rigidBody(phyzx *phyzxObj);
void linearDeform(phyzx *phyzxObj);
void quadRotMat(matrix *rot, phyzx *phyzxObj);
void calcQ(point p, matrix *q);
void calcTAqq(phyzx *phyzxObj);
void calcTAqq(phyzx *phyzxObj);
void quadDeformRot(matrix *R, phyzx *phyzxObj);
void quadDeform(phyzx *phyzxObj);

void reset();
void resetModel(phyzx *phyzxObj);
void modelCopy(GLMmodel *src, GLMmodel *dest);
#endif
