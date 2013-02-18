/* Source: deform
 * Description: Contains functions that controls the deformation simulation.
 */

//#include "render.h"
#include "physics.h"
#include "ui.h"
#include "world.h"
#include "input.h"
#include "keyboard.h"

/* Function: reset
 * Description: Reinitializes the values for simulation.
 * Input: None
 * Output: None
 */
void reset()
{
//	pModel *temp;
	
	DeleteModels();

//	temp = phyzxModels;

	inputInit();
	keyInit();

//	resetModel(temp->pObj);
	
	objectName = -1;
	iMouseModel = 0;
	userForce = vMake(0.0);

//	dispPhysics(temp->pObj);
} //end reset

/* Function: resetModel
 * Description: Revert the models current positions to its rest state.
 * Input: src - Model Data Source
 * Output: None
 */
void resetModel(phyzx *phyzxObj)
{
	for (unsigned int i = 0; i <= phyzxObj->model->numvertices; i++)					/* array of vertices  */
	{
		phyzxObj->model->vertices[3*i+0] = phyzxObj->model->verticesRest[3*i+0];
		phyzxObj->model->vertices[3*i+1] = phyzxObj->model->verticesRest[3*i+1];
		phyzxObj->model->vertices[3*i+2] = phyzxObj->model->verticesRest[3*i+2];
		phyzxObj->velocity[i] = vMake(0.0);
		phyzxObj->avgVel = vMake(0.0);
		phyzxObj->extForce[i] = vMake(0.0);
	} //end for
} //end resetModel

/* Function: modelCopy
 * Description: Copy model data into another model.
 * Input: src - Model Data Source
 *        dest - Model Data Destination
 * Output: None
 */
void modelCopy(GLMmodel *src, GLMmodel *dest)
{
    dest->mtllibname    = NULL;
    dest->numvertices   = 0;
    dest->vertices    = NULL;
    dest->verticesRest = NULL;
    dest->numnormals    = 0;
    dest->normals     = NULL;
    dest->numtexcoords  = 0;
    dest->texcoords       = NULL;
    dest->numfacetnorms = 0;
    dest->facetnorms    = NULL;
    dest->numtriangles  = 0;
    dest->triangles       = NULL;
    dest->nummaterials  = 0;
    dest->materials       = NULL;
    dest->numgroups       = 0;
    dest->groups      = NULL;
    dest->position[0]   = 0.0;
    dest->position[1]   = 0.0;
    dest->position[2]   = 0.0;
    
	/* allocate memory for the triangles in each group */
	GLMgroup *group;
	
	group = src->groups;
	while(src->groups) 
	{
		dest->groups->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
		group->numtriangles = 0;
		group->edges = (GLuint*)malloc(sizeof(GLuint) * group->numEdges * 2);
		group->numEdges = 0;
		group = group->next;
	}
	dest->numvertices = src->numvertices;
	dest->vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (dest->numvertices + 1));

	dest->verticesRest = src->verticesRest;
	dest->verticesRest = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (dest->numvertices + 1));

	dest->numtriangles = src->numtriangles;
	dest->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) * dest->numtriangles);

	dest->numnormals = src->numnormals;
	if (dest->numnormals) 
	{
	  dest->normals = (GLfloat*)malloc(sizeof(GLfloat) *
		  3 * (dest->numnormals + 1));
	}
	
	dest->numtexcoords = src->numtexcoords;
	if (dest->numtexcoords) 
	{
	  dest->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
		  2 * (dest->numtexcoords + 1));
	}

	strcpy((*dest).pathname, (*src).pathname);						/* path to this model */
	strcpy((*dest).mtllibname, (*src).mtllibname);					/* name of the material library */

	(*dest).numvertices = (*src).numvertices;						/* number of vertices in model */
	for (int i = 0; i <= (*dest).numvertices; i++)					/* array of vertices  */
	{
		(*dest).vertices[3*i+0] = (*src).vertices[3*i+0];
		(*dest).vertices[3*i+1] = (*src).vertices[3*i+1];
		(*dest).vertices[3*i+2] = (*src).vertices[3*i+2];

		(*dest).verticesRest[3*i+0] = (*src).verticesRest[3*i+0];
		(*dest).verticesRest[3*i+1] = (*src).verticesRest[3*i+1];
		(*dest).verticesRest[3*i+2] = (*src).verticesRest[3*i+2];
	} //end for

	(*dest).numnormals = (*src).numnormals;							/* number of normals in model */
	(*dest).normals = (*src).normals;								/* array of normals */

	(*dest).numtexcoords = (*src).numtexcoords;						/* number of texcoords in model */
	(*dest).texcoords = (*src).texcoords;							/* array of texture coordinates */

	(*dest).numfacetnorms = (*src).numfacetnorms;					/* number of facetnorms in model */
	(*dest).facetnorms = (*src).facetnorms;							/* array of facetnorms */

	(*dest).numtriangles = (*src).numtriangles;						/* number of triangles in model */
	(*dest).facetnorms = (*src).facetnorms;							/* array of triangles */

	(*dest).nummaterials = (*src).nummaterials;						/* number of materials in model */
	(*dest).materials = (*src).materials;							/* array of materials */

	(*dest).numgroups = (*src).numgroups;							/* number of groups in model */
	(*dest).groups = (*src).groups;									/* linked list of groups */

	(*dest).textureMode = (*src).textureMode;

	/* position of the model */
	(*dest).position[0] = (*src).position[0];
	(*dest).position[1] = (*src).position[1];
	(*dest).position[2] = (*src).position[2];

	strcpy((*dest).directory, (*src).directory);					/* the folder where the model is */

} //end modelCopy