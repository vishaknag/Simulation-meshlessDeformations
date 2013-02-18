/*    
   glme.h
   A library to load, manipulate and render obj meshes.
   This library is a modified version of the "glm" library written by Nate Robins.
   The original glm library can be downloaded from the URL below.

   ========================================================
   glm.h
   Nate Robins, 1997, 2000
   nate@pobox.com, http://www.pobox.com/~nate
 
   Wavefront OBJ model file format reader/writer/manipulator.

   Includes routines for generating smooth normals with
   preservation of edges, welding redundant vertices & texture
   coordinate generation (spheremap and planar projections) + more.
   ========================================================

   Note: vertices in the glm and glme libraries are 1-indexed.
*/

#ifndef _GLME_H_
#define _GLME_H_

#include <math.h>
#include <assert.h>

#ifndef M_PI
  #define M_PI 3.14159265f
#endif

#define GLM_NONE     (0)            /* render with only vertices */
#define GLM_FLAT     (1 << 0)       /* render with facet normals */
#define GLM_SMOOTH   (1 << 1)       /* render with vertex normals */
#define GLM_TEXTURE  (1 << 2)       /* render with texture coords */
#define GLM_COLOR    (1 << 3)       /* render with colors */
#define GLM_MATERIAL (1 << 4)       /* render with materials */

#define GLM_LIGHTINGMODULATION 1
#define GLM_REPLACE 0
#define GLM_MODULATE 1

#define GLM_MIPMAP 2
#define GLM_NOMIPMAP 0
#define GLM_USEMIPMAP 2

// memory allocation modes, use conservative setting when adding vertices and faces dynamically
#define GLM_TIGHT 0
#define GLM_CONSERVATIVE 1

#define GLM_TRIANGLE(model,x) ((model)->triangles[(x)])

/* GLMmaterial: Structure that defines a material in a model. 
 */
typedef struct _GLMmaterial
{
  char * name;                   /* name of material */
  GLfloat diffuse[4];           /* diffuse component */
  GLfloat ambient[4];           /* ambient component */
  GLfloat specular[4];          /* specular component */
  GLfloat emmissive[4];         /* emmissive component */
  GLfloat shininess;            /* specular exponent */
  GLuint textureName;           /* the texture name for rendering*/
  char* textureFile;            /* the name of the file containing the texture */
  GLubyte* textureData;         /* the texture image file */

} GLMmaterial;

/* GLMtriangle: Structure that defines a triangle in a model.
 */
typedef struct _GLMtriangle {
  GLuint vindices[3];           /* array of triangle vertex indices */
  GLuint nindices[3];           /* array of triangle normal indices */
  GLuint tindices[3];           /* array of triangle texcoord indices*/
  GLuint findex;                /* index of triangle facet normal */
} GLMtriangle;

/* GLMgroup: Structure that defines a group in a model.
 */
typedef struct _GLMgroup {
  char*             name;           /* name of this group */
  GLuint            numtriangles;   /* number of triangles in this group */
  GLuint*           triangles;      /* array of triangle indices (index into the global triangle table) */
  GLuint            numEdges;       /* num of the edges of all the triangles in the group; if original mesh had non-triangle faces, edges will correspond to that*/
  GLuint*           edges;          /* edges of all the triangles in the group; */
  GLuint            material;       /* index to material for group */
  struct _GLMgroup* next;           /* pointer to next group in model */
} GLMgroup;

/* GLMmodel: Structure that defines a model.
 */
typedef struct _GLMmodel {
  char*    pathname;            /* path to this model */
  char*    mtllibname;          /* name of the material library */

  GLuint   numvertices;         /* number of vertices in model */
  GLfloat* vertices;            /* array of vertices  */

  GLfloat * verticesRest;

  GLuint   numnormals;          /* number of normals in model */
  GLfloat* normals;             /* array of normals */

  GLuint   numtexcoords;        /* number of texcoords in model */
  GLfloat* texcoords;           /* array of texture coordinates */

  GLuint   numfacetnorms;       /* number of facetnorms in model */
  GLfloat* facetnorms;          /* array of facetnorms */

  GLuint       numtriangles;    /* number of triangles in model */
  GLMtriangle* triangles;       /* array of triangles */

  GLuint       nummaterials;    /* number of materials in model */
  GLMmaterial* materials;       /* array of materials */

  GLuint       numgroups;       /* number of groups in model */
  GLMgroup*    groups;          /* linked list of groups */

  GLuint textureMode;

  GLfloat position[3];          /* position of the model */

  char * directory;             /* the folder where the model is */

} GLMmodel;


/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.  Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure 
 */
GLfloat glmUnitize(GLMmodel* model);

/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model      - initialized GLMmodel structure
 * dimensions - array of 3 GLfloats (GLfloat dimensions[3])
 */
GLvoid glmDimensions(GLMmodel* model, GLfloat* dimensions);

/* glmScale: Scales a model by a given amount.
 * 
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid glmScale(GLMmodel* model, GLfloat scale);

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.  Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 * 
 * model - properly initialized GLMmodel structure 
 */
GLvoid glmReverseWinding(GLMmodel* model);

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid glmFacetNormals(GLMmodel* model);

/* glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.  Then
 * loops through each vertex in the the list averaging all the facet
 * normals of the triangles each vertex is in.  Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.  If the dot product of a facet normal and the facet normal
 * associated with the first triangle in the list of triangles the
 * current vertex is in is greater than the cosine of the angle
 * parameter to the function, that facet normal is not added into the
 * average normal calculation and the corresponding vertex is given
 * the facet normal.  This tends to preserve hard edges.  The angle to
 * use depends on the model, but 90 degrees is usually a good start.
 *
 * model - initialized GLMmodel structure
 * angle - maximum angle (in degrees) to smooth across
 */
GLvoid glmVertexNormals(GLMmodel* model, GLfloat angle);

// sets all vertex normals to the face normals
GLvoid glmSetNormalsToFaceNormals(GLMmodel* model);

// for every vertex, computes the average normal of incident triangles, and store it into "normals" (not to the mesh)
// "normals" must be pre-allocated (required size is 3 * sizeof(float) * model->numvertices)
GLvoid glmVertexAveragedNormals(GLMmodel* model, float * normals);

/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid glmLinearTexture(GLMmodel* model);

/* glmSpheremapTexture: Generates texture coordinates according to a
 * spherical projection of the texture map.  Sometimes referred to as
 * spheremap, or reflection map texture coordinates.  It generates
 * these by using the normal to calculate where that vertex would map
 * onto a sphere.  Since it is impossible to map something flat
 * perfectly onto something spherical, there is distortion at the
 * poles.  This particular implementation causes the poles along the X
 * axis to be distorted.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid glmSpheremapTexture(GLMmodel* model);

GLvoid glmDeleteMaterials(GLMmodel * model);

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
GLvoid glmDelete(GLMmodel* model);

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.  
 */
GLMmodel* glmReadOBJ(char* filename);

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model    - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode     - a bitwise or of values describing what is written to the file
 *            GLM_NONE    -  write only vertices
 *            GLM_FLAT    -  write facet normals
 *            GLM_SMOOTH  -  write vertex normals
 *            GLM_TEXTURE -  write texture coords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid glmWriteOBJ(GLMmodel* model, char* filename, GLuint mode, int writeMTL=1);

/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE    -  render with only vertices
 *            GLM_FLAT    -  render with facet normals
 *            GLM_SMOOTH  -  render with vertex normals
 *            GLM_TEXTURE -  render with texture coords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.
 */
GLvoid glmDraw(GLMmodel* model, unsigned int mode);

// render normals
GLvoid glmDrawNormals(GLMmodel * model);

// render edges
GLvoid glmDrawEdges(GLMmodel * model, char * groupName=0); // if groupName is NULL, renders the entire model, otherwise just the group

// render vertices
GLvoid glmDrawPoints(GLMmodel * model);
GLvoid glmDrawPoints(GLMmodel * model, int * selectedVertices, int numSelectedVertices);
GLvoid glmDrawPoints(GLMmodel * model, int start, int end);
inline GLvoid glmDrawPoint(GLMmodel * model, int vertex) { glVertex3fv(&model->vertices[3*vertex+3]); }
GLvoid glmDrawUnselectedPoints(GLMmodel * model, int * selectionArray);
GLvoid glmDrawSelectedPoints(GLMmodel * model, int * selectedVertices, int numSelectedVertices, int oneIndexed=1);
GLvoid glmDrawPointsSelection(GLMmodel * model);
// shows all point labels, 1-indexing
GLvoid glmShowPointLabels(GLMmodel* model);
// shows point labels from [k to l], 1-indexing
GLvoid glmShowPointLabels(GLMmodel* model, int k, int l);

/* glmList: Generates and returns a display list for the model using
 * the mode specified.
 *
 * model    - initialized GLMmodel structure
 * mode     - a bitwise OR of values describing what is to be rendered.
 *            GLM_NONE    -  render with only vertices
 *            GLM_FLAT    -  render with facet normals
 *            GLM_SMOOTH  -  render with vertex normals
 *            GLM_TEXTURE -  render with texture coords
 *            GLM_FLAT and GLM_SMOOTH should not both be specified.  
 */
GLuint glmList(GLMmodel* model, unsigned int mode);

// create display list for the edges
GLuint glmListEdges(GLMmodel* model);

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model      - initialized GLMmodel structure
 * epsilon    - maximum difference between vertices
 *              ( 0.00001 is a good start for a unitized model)
 *
 */
GLvoid glmWeld(GLMmodel* model, GLfloat epsilon);

/* glmReadPPM: read a PPM raw (type P6) file.  The PPM file has a header
 * that should look something like:
 *
 *    P6
 *    # comment
 *    width height max_value
 *    rgbrgbrgb...
 *
 * where "P6" is the magic cookie which identifies the file type and
 * should be the only characters on the first line followed by a
 * carriage return.  Any line starting with a # mark will be treated
 * as a comment and discarded.   After the magic cookie, three integer
 * values are expected: width, height of the image and the maximum
 * value for a pixel (max_value must be < 256 for PPM raw files).  The
 * data section consists of width*height rgb triplets (one byte each)
 * in binary format (i.e., such as that written with fwrite() or
 * equivalent).
 *
 * The rgb data is returned as an array of unsigned chars (packed
 * rgb).  The malloc()'d memory should be free()'d by the caller.  If
 * an error occurs, an error message is sent to stderr and NULL is
 * returned.
 *
 * filename   - name of the .ppm file.
 * width      - will contain the width of the image on return.
 * height     - will contain the height of the image on return.
 *
 */
GLubyte* glmReadPPM(char* filename, int* width, int* height);

// set memory allocation mode
// GLM_TIGHT: amount allocated equals the exact amount needed for the mesh... this is default
// GLM_CONSERVATIVE: allocate enough memory to support full fracture of the model down into individual tetrahedra
void glmSetMemoryAllocationMode(int mode);
// set maximum allocation sizes, to be used in GLM_CONSERVATIVE mode
void glmSetMemoryAllocationSizes(
  int maxNumTriangles, int maxNumVertices, int maxNumNormals, int maxNumTexCoords);

// set up textures
// mode is either GLM_REPLACE or GLM_MODULATE
int glmSetUpTextures(GLMmodel* model, int mode);

// deforms the vertices of the mesh
GLvoid glmApplyDeformation(GLMmodel* model, float * U, float * q, int n, int r);
GLvoid glmApplyDeformation(GLMmodel * mode, float * u);
GLvoid glmApplyDeformation(GLMmodel * mode, double * u);
// these functions allows for renumbering of vertices
// so that a surface mesh and volumetric mesh can coexist
GLvoid glmApplyDeformation(GLMmodel * mode, float * u, int * mask);
GLvoid glmApplyDeformation(GLMmodel * mode, double * u, int * mask);

// add a group to the model
GLMgroup* glmAddGroup(GLMmodel* model, char* name);
void glmPrintGroupInfo(GLMmodel * model);

// adds new vertices/normals/tex coords to the mesh
// returns the index of the newly added quantity
// indices are 1-indexed
int glmAddVertex(GLMmodel * model, GLfloat x, GLfloat y, GLfloat z);
int glmAddNormal(GLMmodel * model, GLfloat x, GLfloat y, GLfloat z);
int glmAddTexCoord(GLMmodel * model, GLfloat u, GLfloat v);
// makes a new vertex, which is a copy of vertex i 
int glmCopyVertex(GLMmodel * model, int i);
// adds a triangle spanned on the given indices/normals/texcoords
void glmAddTriangle(GLMmodel * model, GLMgroup * group, int v1, int v2, int v3, int n1, int n2, int n3, int t1, int t2, int t3);
void glmAddTriangle(GLMmodel * model, GLMgroup * group, int v1, int v2, int v3, int n1, int n2, int n3);
void glmAddTriangle(GLMmodel * model, GLMgroup * group, int v1, int v2, int v3);

// returns the three indices of vertices of triangle 'triangleID'
// triangles are 0-indexed, vertices are 1-indexed
void glmGetTriangle(GLMmodel * model, int triangleID, int & i1, int & i2, int & i3);
// sets vertex pos of triangle triangleID to a new value
// triangles are 0-indexed, vertices are 1-indexed
void glmSetTriangle(GLMmodel * model, int triangleID, int pos, int newValue);

// note: vertex indices are 1-indexed
inline GLint glmGetNumVertices(GLMmodel * model) { return model->numvertices; }
void glmGetVertex(GLMmodel * model, int index, float & v1, float & v2, float & v3);
void glmSetVertex(GLMmodel * model, int index, int pos, float value);
void glmPermuteVertices(GLMmodel * model, int * permutation);

// saves the model to a "flat" obj format
GLvoid saveToFlatFormat(GLMmodel* model, char * filename);

/* _GLMnode: general purpose node */
typedef struct _GLMnode {
    GLuint         index;
    GLboolean      averaged;
    struct _GLMnode* next;
} GLMnode;
// for every vertex, builds the indices of the indicent triangles
GLMnode ** glmBuildNeighborStructure(GLMmodel* model); 
GLvoid glmDeleteNeighborStructure(GLMmodel* model, GLMnode ** structure);

// sets the normal of every face to the averaged normal of the neighboring faces
// neighboring faces where the angle with the face normal is too small are excluded from the average
GLvoid glmSetNormalsToFaceNormalsThresholded(GLMmodel* model, GLMnode ** neighborStructure, float thresholdAngle);

// compute the mesh "center" and a bounding radius
GLvoid glmMeshGeometricParameters(GLMmodel * mesh, double * centerX, double * centerY, double * centerZ, double * radius);
// compute smallest ball radius that covers the entire model, centered at the given center
GLvoid glmMeshRadius(GLMmodel * mesh, double centerX, double centerY, double centerZ, double * radius);
// finds the closest vertex to a given query position (using linear exhaustive search)
int glmClosestVertex(GLMmodel * model, double queryPosX, double queryPosY, double queryPosZ);

// exports the mesh data (vertices, triangles); vertices and trianges will be allocated as necessary
GLvoid glmMeshData(GLMmodel * mesh, int * numVertices, double ** vertices, int * numTriangles, int ** triangles);

// sets the alpha value for all the materials
GLvoid glmSetMaterialAlpha(GLMmodel * model, double alpha); 

/* glmDot: compute the dot product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 */
static inline GLfloat glmDot(GLfloat* u, GLfloat* v)
{
  return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
static inline GLvoid glmNormalize(GLfloat* v)
{
  GLfloat l = (GLfloat)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  v[0] /= l;
  v[1] /= l;
  v[2] /= l;
}

#endif
