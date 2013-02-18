#include "render.h"

#define T(x) (model->triangles[(x)])

int memoryAllocationMode = GLM_TIGHT;

int maxNumVertices;
int maxNumTriangles;
int maxNumNormals;
int maxNumTexCoords;

static char * glmDuplicateString(char * s)
{
  // strdup sometimes causes problems, so we use this
  char * p = (char*) malloc (sizeof(char) * (strlen(s) + 1));
  memcpy(p, s, sizeof(char) * (strlen(s) + 1));
  return p;
}

/* glmMax: returns the maximum of two floats */
static GLfloat
glmMax(GLfloat a, GLfloat b) 
{
    if (b > a)
        return b;
    return a;
}

/* glmAbs: returns the absolute value of a float */
static GLfloat
glmAbs(GLfloat f)
{
    if (f < 0)
        return -f;
    return f;
}

/* glmCross: compute the cross product of two vectors
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3])
 * n - array of 3 GLfloats (GLfloat n[3]) to return the cross product in
 */
static GLvoid
glmCross(GLfloat* u, GLfloat* v, GLfloat* n)
{
    assert(u); assert(v); assert(n);
    
    n[0] = u[1]*v[2] - u[2]*v[1];
    n[1] = u[2]*v[0] - u[0]*v[2];
    n[2] = u[0]*v[1] - u[1]*v[0];
}



/* glmEqual: compares two vectors and returns GL_TRUE if they are
 * equal (within a certain threshold) or GL_FALSE if not. An epsilon
 * that works fairly well is 0.000001.
 *
 * u - array of 3 GLfloats (GLfloat u[3])
 * v - array of 3 GLfloats (GLfloat v[3]) 
 */
static GLboolean
glmEqual(GLfloat* u, GLfloat* v, GLfloat epsilon)
{
    if (glmAbs(u[0] - v[0]) < epsilon &&
        glmAbs(u[1] - v[1]) < epsilon &&
        glmAbs(u[2] - v[2]) < epsilon) 
    {
        return GL_TRUE;
    }
    return GL_FALSE;
}

/* glmWeldVectors: eliminate (weld) vectors that are within an
 * epsilon of each other.
 *
 * vectors     - array of GLfloat[3]'s to be welded
 * numvectors - number of GLfloat[3]'s in vectors
 * epsilon     - maximum difference between vectors 
 *
 */
GLfloat*
glmWeldVectors(GLfloat* vectors, GLuint* numvectors, GLfloat epsilon)
{
    GLfloat* copies;
    GLuint   copied;
    GLuint   i, j;
    
    copies = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (*numvectors + 1));
    memcpy(copies, vectors, (sizeof(GLfloat) * 3 * (*numvectors + 1)));
    
    copied = 1;
    for (i = 1; i <= *numvectors; i++) {
        for (j = 1; j <= copied; j++) {
            if (glmEqual(&vectors[3 * i], &copies[3 * j], epsilon)) {
                goto duplicate;
            }
        }
        
        /* must not be any duplicates -- add to the copies array */
        copies[3 * copied + 0] = vectors[3 * i + 0];
        copies[3 * copied + 1] = vectors[3 * i + 1];
        copies[3 * copied + 2] = vectors[3 * i + 2];
        j = copied;             /* pass this along for below */
        copied++;
        
duplicate:
/* set the first component of this vector to point at the correct
        index into the new copies array */
        vectors[3 * i + 0] = (GLfloat)j;
    }
    
    *numvectors = copied-1;
    return copies;
}

void glmPrintGroupInfo(GLMmodel * model)
{
  GLMgroup * group;
  assert(model);
    
  group = model->groups;
  int groupCounter = 0;
  while(group) 
  {
    printf("=== Group %d ===\n", groupCounter);
    printf("  Name: %s\n", group->name);
    printf("  Num triangles: %d\n", group->numtriangles);
    printf("  Num edges: %d\n", group->numEdges);
    printf("  Triangle pointer: %p\n", group->triangles);
    printf("  Edge pointer: %p\n", group->edges);
    printf("  Next group pointer: %p\n", group->next);
    printf("  Material index: %d\n", group->material);
    printf("  Material name: %s\n", model->materials[group->material].name);

    group = group->next;
    groupCounter++;
  }
  printf("Total num groups: %d\n", groupCounter);
}

/* glmFindGroup: Find a group in the model */
GLMgroup* glmFindGroup(GLMmodel* model, char* name)
{
    GLMgroup* group;
    
    assert(model);
    
    group = model->groups;
    while(group) 
    {
      if (!strcmp(name, group->name))
        break;
      group = group->next;
    }
    
    return group;
}

/* glmAddGroup: Add a group to the model */
GLMgroup* glmAddGroup(GLMmodel* model, char* name)
{
    GLMgroup* group;
    
    group = glmFindGroup(model, name);
    if (!group) {
        group = (GLMgroup*)malloc(sizeof(GLMgroup));
        group->name = glmDuplicateString(name);
        group->material = 0;
        group->numtriangles = 0;
        group->triangles = NULL;
        group->numEdges = 0;
        group->edges = NULL;
        group->next = model->groups;
        model->groups = group;
        model->numgroups++;
    }
    
    return group;
}

/* glmFindGroup: Find a material in the model */
GLuint
glmFindMaterial(GLMmodel* model, char* name)
{
    GLuint i;
    
    /* doing a linear search on a string key'd list is pretty lame,
    but it works and is fast enough for now. */
    for (i = 0; i < model->nummaterials; i++) {
        if (!strcmp(model->materials[i].name, name))
            goto found;
    }
    
    /* didn't find the name, so print a warning and return the default
    material (0). */
    printf("glmFindMaterial():  can't find material \"%s\".\n", name);
    i = 0;
    
found:
    return i;
}


/* glmDirName: return the directory given a path
 *
 * path - filesystem path
 *
 * NOTE: the return value should be free'd.
 */
static char* glmDirName(char* path)
{
    char * lastSeparator = NULL;
    char * ch = path;
    int counter=0;
    int lastSeparatorPos = 0;
    while (*ch != '\0')
    {
      if ((*ch == '\\') || (*ch == '/'))
      {
        lastSeparator = ch;
        lastSeparatorPos = counter;
      }
      ch++;
      counter++;
    }

    char * dir;
    if (lastSeparator != NULL)
    {
      dir = (char*) malloc (sizeof(char) * (lastSeparatorPos+1));
      dir[lastSeparatorPos] = '\0';
      strncpy(dir,path,sizeof(char)*lastSeparatorPos);
    }
    else
    {
      dir = (char*) malloc (sizeof(char) * 2);
      strcpy(dir,".");
    }

    return dir;
}

GLvoid glmDeleteMaterials(GLMmodel * model)
{
  for(unsigned int i=0; i<model->nummaterials; i++)
  {
    free(model->materials[i].name);
    free(model->materials[i].textureFile);
    free(model->materials[i].textureData);
  }
  free(model->materials);
}

GLvoid glmSetMaterialAlpha(GLMmodel * model, double alpha)
{
  for (unsigned int i = 0; i < model->nummaterials; i++) 
  {
    model->materials[i].diffuse[3] = alpha;
    model->materials[i].ambient[3] = alpha;
    model->materials[i].specular[3] = alpha;
  }
}

GLvoid glmMakeDefaultMaterials(GLMmodel * model, int numDefaultMaterials)
{
  if (model->materials) 
    glmDeleteMaterials(model);

  model->materials = (GLMmaterial*)malloc(sizeof(GLMmaterial) * numDefaultMaterials);
  model->nummaterials = numDefaultMaterials;
    
  // set the default materials 
  for (int i = 0; i < numDefaultMaterials; i++) 
  {
    model->materials[i].name = NULL;
    model->materials[i].shininess = 65.0;
    model->materials[i].diffuse[0] = 0.6;
    model->materials[i].diffuse[1] = 0.6;
    model->materials[i].diffuse[2] = 0.6;
    model->materials[i].diffuse[3] = 1.0;
    model->materials[i].ambient[0] = 0.2;
    model->materials[i].ambient[1] = 0.2;
    model->materials[i].ambient[2] = 0.2;
    model->materials[i].ambient[3] = 1.0;
    model->materials[i].specular[0] = 0.0;
    model->materials[i].specular[1] = 0.0;
    model->materials[i].specular[2] = 0.0;
    model->materials[i].specular[3] = 1.0;
    model->materials[i].textureFile = NULL;
    model->materials[i].textureData = NULL;
    model->materials[i].name = (char*) malloc (sizeof(char) * 8);
    strcpy(model->materials[i].name, "default");
  }
}

/* glmReadMTL: read a wavefront material library file
 *
 * model - properly initialized GLMmodel structure
 * name  - name of the material library
 */

static GLvoid glmReadMTL(GLMmodel* model, char* name)
{
  FILE* file;
  char* dir;
  char* filename;
  char buf[1024];
  char buf2[1024];
  char buf3[1024];
  GLuint nummaterials;
    
  dir = glmDirName(model->pathname);
  filename = (char*)malloc(sizeof(char) * (strlen(dir) + strlen(name) + 2));
  strcpy(filename, dir);
  strcat(filename, "/");
  strcat(filename, name);
  free(dir);
    
  file = fopen(filename, "r");
  if (!file) 
  {
    fprintf(stderr, "glmReadMTL() failed: can't open material file %s.\n", filename);
    exit(1);
  }
  free(filename);
    
  /* count the number of materials in the file */
  nummaterials = 1;
  while(fscanf(file, "%s", buf) != EOF) {
      switch(buf[0]) {
      case '#':               /* comment */
          /* eat up rest of line */
          fgets(buf, sizeof(buf), file);
          break;
      case 'n':               /* newmtl */
          fgets(buf, sizeof(buf), file);
          nummaterials++;
          sscanf(buf, "%s %s", buf2, buf3);
          break;
      default:
          /* eat up rest of line */
          fgets(buf, sizeof(buf), file);
          break;
      }
  }
  
  rewind(file);
    
  glmMakeDefaultMaterials(model,nummaterials);

  /* now, read in the data */
  nummaterials = 0;
  while(fscanf(file, "%s", buf) != EOF) 
  {
      switch(buf[0]) 
      {
      case '#':               /* comment */
          /* eat up rest of line */
          fgets(buf, sizeof(buf), file);
          break;
      case 'n':               /* newmtl */
          fscanf(file, "%s", buf2);
          nummaterials++;
          free(model->materials[nummaterials].name);
          model->materials[nummaterials].name = glmDuplicateString(buf2);
          fgets(buf, sizeof(buf), file);
          break;
      case 'N':
          if (buf[1] == 's')
          {
            fscanf(file, "%f", &model->materials[nummaterials].shininess);
            // wavefront shininess is from [0, 1000], so scale for OpenGL 
            model->materials[nummaterials].shininess *= 128.0 / 1000.0;
          }
          else
            fgets(buf, sizeof(buf), file); //eat rest of line
          break;
      case 'K':
          switch(buf[1]) 
          {
            case 'd':
                fscanf(file, "%f %f %f",
                     &model->materials[nummaterials].diffuse[0],
                     &model->materials[nummaterials].diffuse[1],
                     &model->materials[nummaterials].diffuse[2]);
                break;
            case 's':
                fscanf(file, "%f %f %f",
                    &model->materials[nummaterials].specular[0],
                    &model->materials[nummaterials].specular[1],
                    &model->materials[nummaterials].specular[2]);
                break;
            case 'a':
                fscanf(file, "%f %f %f",
                    &model->materials[nummaterials].ambient[0],
                    &model->materials[nummaterials].ambient[1],
                    &model->materials[nummaterials].ambient[2]);
                break;
            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
          }
          break;

      case 'm': 
          // texture file, allow only one texture file, has to be ppm format
          // different materials can have different textures
          // however, each single material can have only one texture

          // if texture file not already assigned it, assign it, 
          // and then load the texture file into main memory and assign texture name to it

          fgets(buf, sizeof(buf), file);
          sscanf(buf, "%s %s", buf, buf);
          free(model->materials[nummaterials].textureFile);
          model->materials[nummaterials].textureFile = glmDuplicateString(buf);
          break;

      default:
          /* eat up rest of line */
          fgets(buf, sizeof(buf), file);
          break;
      }
  }
}

/* glmWriteMTL: write a wavefront material library file
 *
 * model   - properly initialized GLMmodel structure
 * modelpath  - pathname of the model being written
 * mtllibname - name of the material library to be written
 */
static GLvoid
glmWriteMTL(GLMmodel* model, char* modelpath, char* mtllibname)
{
    FILE* file;
    char* dir;
    char* filename;
    GLMmaterial* material;
    GLuint i;
    
    dir = glmDirName(modelpath);
    filename = (char*)malloc(sizeof(char) * (strlen(dir)+strlen(mtllibname)));
    strcpy(filename, dir);
    strcat(filename, "/");
    strcat(filename, mtllibname);
    free(dir);
    
    /* open the file */
    file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "glmWriteMTL() failed: can't open file \"%s\".\n",
            filename);
        exit(1);
    }
    free(filename);
    
    /* spit out a header */
    fprintf(file, "#  \n");
    fprintf(file, "#  Wavefront MTL generated by GLM library\n");
    fprintf(file, "#  \n");
    fprintf(file, "#  GLM library\n");
    fprintf(file, "#  Nate Robins\n");
    fprintf(file, "#  ndr@pobox.com\n");
    fprintf(file, "#  http://www.pobox.com/~ndr\n");
    fprintf(file, "#  \n\n");
    
    for (i = 0; i < model->nummaterials; i++) {
        material = &model->materials[i];
        fprintf(file, "newmtl %s\n", material->name);
        fprintf(file, "Ka %f %f %f\n", 
            material->ambient[0], material->ambient[1], material->ambient[2]);
        fprintf(file, "Kd %f %f %f\n", 
            material->diffuse[0], material->diffuse[1], material->diffuse[2]);
        fprintf(file, "Ks %f %f %f\n", 
            material->specular[0],material->specular[1],material->specular[2]);
        fprintf(file, "Ns %f\n", material->shininess / 128.0 * 1000.0);
        fprintf(file, "\n");
    }
}


/* glmFirstPass: first pass at a Wavefront OBJ file that gets all the
 * statistics of the model (such as #vertices, #normals, etc)
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor 
 * directory - the directory where the obj file resides (necessary to open the material file)
 */
static GLvoid glmFirstPass(GLMmodel* model, FILE* file)
{
    GLuint  numvertices;        /* number of vertices in model */
    GLuint  numnormals;         /* number of normals in model */
    GLuint  numtexcoords;       /* number of texcoords in model */
    GLuint  numtriangles;       /* number of triangles in model */
    GLMgroup* group;            /* current group */
    unsigned    v, n, t;
    char        buf[128];
    
    /* make a default group */
    group = glmAddGroup(model, "default");
    
    numvertices = numnormals = numtexcoords = numtriangles = 0;
    while(fscanf(file, "%s", buf) != EOF) {
        switch(buf[0]) {
        case '#':               /* comment */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'v':               /* v, vn, vt */
            switch(buf[1]) {
            case '\0':          /* vertex */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numvertices++;
                break;
            case 'n':           /* normal */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numnormals++;
                break;
            case 't':           /* texcoord */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                numtexcoords++;
                break;
            default:
                printf("glmFirstPass(): Unknown token \"%s\".\n", buf);
                exit(1);
                break;
            }
            break;
            case 'm':
                fgets(buf, sizeof(buf), file);
                sscanf(buf, "%s %s", buf, buf);
                model->mtllibname = glmDuplicateString(buf);
                glmReadMTL(model, model->mtllibname);
                break;
            case 'u':
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
            case 'g':               /* group */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
                sscanf(buf, "%s", buf);
#else
                buf[strlen(buf)-1] = '\0';  /* nuke '\n' */
#endif
                group = glmAddGroup(model, buf);
                break;
            case 'f':               /* face */
                v = n = t = 0;
                fscanf(file, "%s", buf);
                /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
                if (strstr(buf, "//")) {
                    /* v//n */
                    sscanf(buf, "%d//%d", &v, &n);
                    fscanf(file, "%d//%d", &v, &n);
                    fscanf(file, "%d//%d", &v, &n);
                    numtriangles++;
                    group->numtriangles++;
                    group->numEdges += 3;
                    while(fscanf(file, "%d//%d", &v, &n) > 0) {
                        numtriangles++;
                        group->numtriangles++;
                        group->numEdges++;
                    }
                } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
                    /* v/t/n */
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    numtriangles++;
                    group->numtriangles++;
                    group->numEdges += 3;
                    while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
                        numtriangles++;
                        group->numtriangles++;
                        group->numEdges++;
                    }
                } else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
                    /* v/t */
                    fscanf(file, "%d/%d", &v, &t);
                    fscanf(file, "%d/%d", &v, &t);
                    numtriangles++;
                    group->numtriangles++;
                    group->numEdges += 3;
                    while(fscanf(file, "%d/%d", &v, &t) > 0) {
                        numtriangles++;
                        group->numtriangles++;
                        group->numEdges++;
                    }
                } else {
                    /* v */
                    fscanf(file, "%d", &v);
                    fscanf(file, "%d", &v);
                    numtriangles++;
                    group->numtriangles++;
                    group->numEdges += 3;
                    while(fscanf(file, "%d", &v) > 0) {
                        numtriangles++;
                        group->numtriangles++;
                        group->numEdges++;
                    }
                }
                break;
                
            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
        }
  }
  
  /* set the stats in the model structure */
  model->numvertices  = numvertices;
  model->numnormals   = numnormals;
  model->numtexcoords = numtexcoords;
  model->numtriangles = numtriangles;
  
}

// internal routine: adds the edges corresponding to the given fan of triangles
void AddEdges(GLMmodel * model, GLMgroup * group, int startFanTriangles, int endFanTriangle)
{
  // add first two edges
  group->edges[2*group->numEdges+0] = T(startFanTriangles).vindices[0];
  group->edges[2*group->numEdges+1] = T(startFanTriangles).vindices[1];
  group->numEdges++;

  group->edges[2*group->numEdges+0] = T(startFanTriangles).vindices[1];
  group->edges[2*group->numEdges+1] = T(startFanTriangles).vindices[2];
  group->numEdges++;

  // add edges in the middle
  for(int tri=startFanTriangles+1; tri <= endFanTriangle; tri++)
  {
    group->edges[2*group->numEdges+0] = T(tri).vindices[1];
    group->edges[2*group->numEdges+1] = T(tri).vindices[2];
    group->numEdges++;
  }

  // link last point to first point
  group->edges[2*group->numEdges+0] = T(endFanTriangle).vindices[2];
  group->edges[2*group->numEdges+1] = T(startFanTriangles).vindices[0];
  group->numEdges++;

}

/* glmSecondPass: second pass at a Wavefront OBJ file that gets all
 * the data.
 *
 * model - properly initialized GLMmodel structure
 * file  - (fopen'd) file descriptor 
 */
static GLvoid
glmSecondPass(GLMmodel* model, FILE* file) 
{
    GLuint  numvertices;        /* number of vertices in model */
    GLuint  numnormals;         /* number of normals in model */
    GLuint  numtexcoords;       /* number of texcoords in model */
    GLuint  numtriangles;       /* number of triangles in model */
    GLfloat*    vertices;           /* array of vertices  */
    GLfloat*    normals;            /* array of normals */
    GLfloat*    texcoords;          /* array of texture coordinates */
    GLMgroup* group;            /* current group pointer */
    GLuint  material;           /* current material */
    GLuint  v, n, t;
    char        buf[128];
    
    /* set the pointer shortcuts */
    vertices       = model->vertices;
    normals    = model->normals;
    texcoords    = model->texcoords;
    group      = model->groups;
    
    int startTriangle; // for edges

    /* on the second pass through the file, read all the data into the allocated arrays */
    numvertices = numnormals = numtexcoords = 1;
    numtriangles = 0;
    material = 0;
    while(fscanf(file, "%s", buf) != EOF) {
        switch(buf[0]) {
        case '#':               /* comment */
            /* eat up rest of line */
            fgets(buf, sizeof(buf), file);
            break;
        case 'v':               /* v, vn, vt */
            switch(buf[1]) 
            {
            case '\0':          /* vertex */
                fscanf(file, "%f %f %f", 
                    &vertices[3 * numvertices + 0], 
                    &vertices[3 * numvertices + 1], 
                    &vertices[3 * numvertices + 2]);
                numvertices++;
                break;
            case 'n':           /* normal */
                fscanf(file, "%f %f %f", 
                    &normals[3 * numnormals + 0],
                    &normals[3 * numnormals + 1], 
                    &normals[3 * numnormals + 2]);
                numnormals++;
                break;
            case 't':           /* texcoord */
                fscanf(file, "%f %f", 
                    &texcoords[2 * numtexcoords + 0],
                    &texcoords[2 * numtexcoords + 1]);
                numtexcoords++;
                break;
            }
            break;
            case 'u':
                fgets(buf, sizeof(buf), file);
                sscanf(buf, "%s %s", buf, buf);
                group->material = material = glmFindMaterial(model, buf);
                break;
            case 'g':               /* group */
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
#if SINGLE_STRING_GROUP_NAMES
                sscanf(buf, "%s", buf);
#else
                buf[strlen(buf)-1] = '\0';  /* nuke '\n' */
#endif
                group = glmFindGroup(model, buf);
                group->material = material;
                break;
            case 'f':               /* face */
                v = n = t = 0;
                startTriangle = numtriangles;

                fscanf(file, "%s", buf);
                /* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
                if (strstr(buf, "//")) {
                    /* v//n */
                    sscanf(buf, "%d//%d", &v, &n);
                    T(numtriangles).vindices[0] = v;
                    T(numtriangles).nindices[0] = n;
                    fscanf(file, "%d//%d", &v, &n);
                    T(numtriangles).vindices[1] = v;
                    T(numtriangles).nindices[1] = n;
                    fscanf(file, "%d//%d", &v, &n);
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).nindices[2] = n;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                    while(fscanf(file, "%d//%d", &v, &n) > 0) {
						// tesselating the polygon on the fly
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
                        T(numtriangles).vindices[2] = v;
                        T(numtriangles).nindices[2] = n;
                        group->triangles[group->numtriangles++] = numtriangles;
                        numtriangles++;
                    }

                    AddEdges(model, group, startTriangle, numtriangles-1);

                } else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3) {
                    /* v/t/n */
                    T(numtriangles).vindices[0] = v;
                    T(numtriangles).tindices[0] = t;
                    T(numtriangles).nindices[0] = n;
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    T(numtriangles).vindices[1] = v;
                    T(numtriangles).tindices[1] = t;
                    T(numtriangles).nindices[1] = n;
                    fscanf(file, "%d/%d/%d", &v, &t, &n);
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).tindices[2] = t;
                    T(numtriangles).nindices[2] = n;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                    while(fscanf(file, "%d/%d/%d", &v, &t, &n) > 0) {
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
                        T(numtriangles).nindices[0] = T(numtriangles-1).nindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
                        T(numtriangles).nindices[1] = T(numtriangles-1).nindices[2];
                        T(numtriangles).vindices[2] = v;
                        T(numtriangles).tindices[2] = t;
                        T(numtriangles).nindices[2] = n;
                        group->triangles[group->numtriangles++] = numtriangles;
                        numtriangles++;
                    }

                    AddEdges(model, group, startTriangle, numtriangles-1);

                } else if (sscanf(buf, "%d/%d", &v, &t) == 2) {
                    /* v/t */
                    T(numtriangles).vindices[0] = v;
                    T(numtriangles).tindices[0] = t;
                    fscanf(file, "%d/%d", &v, &t);
                    T(numtriangles).vindices[1] = v;
                    T(numtriangles).tindices[1] = t;
                    fscanf(file, "%d/%d", &v, &t);
                    T(numtriangles).vindices[2] = v;
                    T(numtriangles).tindices[2] = t;
                    group->triangles[group->numtriangles++] = numtriangles;
                    numtriangles++;
                    while(fscanf(file, "%d/%d", &v, &t) > 0) {
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).tindices[0] = T(numtriangles-1).tindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).tindices[1] = T(numtriangles-1).tindices[2];
                        T(numtriangles).vindices[2] = v;
                        T(numtriangles).tindices[2] = t;
                        group->triangles[group->numtriangles++] = numtriangles;
                        numtriangles++;
                    }

                    AddEdges(model, group, startTriangle, numtriangles-1);

                } else {
                    /* v */

                    sscanf(buf, "%d", &v);
                    T(numtriangles).vindices[0] = v;
                    fscanf(file, "%d", &v);
                    T(numtriangles).vindices[1] = v;
                    fscanf(file, "%d", &v);
                    T(numtriangles).vindices[2] = v;
                    group->triangles[group->numtriangles++] = numtriangles; // link the new triangle to the group
                    numtriangles++;

                    while(fscanf(file, "%d", &v) > 0) 
					{
                        T(numtriangles).vindices[0] = T(numtriangles-1).vindices[0];
                        T(numtriangles).vindices[1] = T(numtriangles-1).vindices[2];
                        T(numtriangles).vindices[2] = v;
                        group->triangles[group->numtriangles++] = numtriangles;
                        numtriangles++;
                    }

					AddEdges(model, group, startTriangle, numtriangles-1);
                }
                break;
                
            default:
                /* eat up rest of line */
                fgets(buf, sizeof(buf), file);
                break;
    }
  }
  
#if 0
  /* announce the memory requirements */
  printf(" Memory: %d bytes\n",
      numvertices  * 3*sizeof(GLfloat) +
      numnormals   * 3*sizeof(GLfloat) * (numnormals ? 1 : 0) +
      numtexcoords * 3*sizeof(GLfloat) * (numtexcoords ? 1 : 0) +
      numtriangles * sizeof(GLMtriangle));
#endif
}


/* public functions */


/* glmUnitize: "unitize" a model by translating it to the origin and
 * scaling it to fit in a unit cube around the origin.   Returns the
 * scalefactor used.
 *
 * model - properly initialized GLMmodel structure 
 */
GLfloat
glmUnitize(GLMmodel* model)
{
    GLuint  i;
    GLfloat maxx, minx, maxy, miny, maxz, minz;
    GLfloat cx, cy, cz, w, h, d;
    GLfloat scale;
    
    assert(model);
    assert(model->vertices);
    
    /* get the max/mins */
    maxx = minx = model->vertices[3 + 0];
    maxy = miny = model->vertices[3 + 1];
    maxz = minz = model->vertices[3 + 2];
    for (i = 1; i <= model->numvertices; i++) {
        if (maxx < model->vertices[3 * i + 0])
            maxx = model->vertices[3 * i + 0];
        if (minx > model->vertices[3 * i + 0])
            minx = model->vertices[3 * i + 0];
        
        if (maxy < model->vertices[3 * i + 1])
            maxy = model->vertices[3 * i + 1];
        if (miny > model->vertices[3 * i + 1])
            miny = model->vertices[3 * i + 1];
        
        if (maxz < model->vertices[3 * i + 2])
            maxz = model->vertices[3 * i + 2];
        if (minz > model->vertices[3 * i + 2])
            minz = model->vertices[3 * i + 2];
    }
    
    /* calculate model width, height, and depth */
    w = glmAbs(maxx) + glmAbs(minx);
    h = glmAbs(maxy) + glmAbs(miny);
    d = glmAbs(maxz) + glmAbs(minz);
    
    /* calculate center of the model */
    cx = (maxx + minx) / 2.0;
    cy = (maxy + miny) / 2.0;
    cz = (maxz + minz) / 2.0;
    
    /* calculate unitizing scale factor */
    scale = 2.0 / glmMax(glmMax(w, h), d);
    
    /* translate around center then scale */
    for (i = 1; i <= model->numvertices; i++) {
        model->vertices[3 * i + 0] -= cx;
        model->vertices[3 * i + 1] -= cy;
        model->vertices[3 * i + 2] -= cz;
        model->vertices[3 * i + 0] *= scale;
        model->vertices[3 * i + 1] *= scale;
        model->vertices[3 * i + 2] *= scale;
    }
    
    return scale;
}

/* glmDimensions: Calculates the dimensions (width, height, depth) of
 * a model.
 *
 * model   - initialized GLMmodel structure
 * dimensions - array of 3 GLfloats (GLfloat dimensions[3])
 */
GLvoid
glmDimensions(GLMmodel* model, GLfloat* dimensions)
{
    GLuint i;
    GLfloat maxx, minx, maxy, miny, maxz, minz;
    
    assert(model);
    assert(model->vertices);
    assert(dimensions);
    
    /* get the max/mins */
    maxx = minx = model->vertices[3 + 0];
    maxy = miny = model->vertices[3 + 1];
    maxz = minz = model->vertices[3 + 2];
    for (i = 1; i <= model->numvertices; i++) {
        if (maxx < model->vertices[3 * i + 0])
            maxx = model->vertices[3 * i + 0];
        if (minx > model->vertices[3 * i + 0])
            minx = model->vertices[3 * i + 0];
        
        if (maxy < model->vertices[3 * i + 1])
            maxy = model->vertices[3 * i + 1];
        if (miny > model->vertices[3 * i + 1])
            miny = model->vertices[3 * i + 1];
        
        if (maxz < model->vertices[3 * i + 2])
            maxz = model->vertices[3 * i + 2];
        if (minz > model->vertices[3 * i + 2])
            minz = model->vertices[3 * i + 2];
    }
    
    /* calculate model width, height, and depth */
    dimensions[0] = glmAbs(maxx) + glmAbs(minx);
    dimensions[1] = glmAbs(maxy) + glmAbs(miny);
    dimensions[2] = glmAbs(maxz) + glmAbs(minz);
}

/* glmScale: Scales a model by a given amount.
 * 
 * model - properly initialized GLMmodel structure
 * scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
 */
GLvoid
glmScale(GLMmodel* model, GLfloat scale)
{
    GLuint i;
    
    for (i = 1; i <= model->numvertices; i++) {
        model->vertices[3 * i + 0] *= scale;
        model->vertices[3 * i + 1] *= scale;
        model->vertices[3 * i + 2] *= scale;
    }
}

/* glmReverseWinding: Reverse the polygon winding for all polygons in
 * this model.   Default winding is counter-clockwise.  Also changes
 * the direction of the normals.
 * 
 * model - properly initialized GLMmodel structure 
 */
GLvoid
glmReverseWinding(GLMmodel* model)
{
    GLuint i, swap;
    
    assert(model);
    
    for (i = 0; i < model->numtriangles; i++) {
        swap = T(i).vindices[0];
        T(i).vindices[0] = T(i).vindices[2];
        T(i).vindices[2] = swap;
        
        if (model->numnormals) {
            swap = T(i).nindices[0];
            T(i).nindices[0] = T(i).nindices[2];
            T(i).nindices[2] = swap;
        }
        
        if (model->numtexcoords) {
            swap = T(i).tindices[0];
            T(i).tindices[0] = T(i).tindices[2];
            T(i).tindices[2] = swap;
        }
    }
    
    /* reverse facet normals */
    for (i = 1; i <= model->numfacetnorms; i++) {
        model->facetnorms[3 * i + 0] = -model->facetnorms[3 * i + 0];
        model->facetnorms[3 * i + 1] = -model->facetnorms[3 * i + 1];
        model->facetnorms[3 * i + 2] = -model->facetnorms[3 * i + 2];
    }
    
    /* reverse vertex normals */
    for (i = 1; i <= model->numnormals; i++) {
        model->normals[3 * i + 0] = -model->normals[3 * i + 0];
        model->normals[3 * i + 1] = -model->normals[3 * i + 1];
        model->normals[3 * i + 2] = -model->normals[3 * i + 2];
    }
}

/* glmFacetNormals: Generates facet normals for a model (by taking the
 * cross product of the two vectors derived from the sides of each
 * triangle).  Assumes a counter-clockwise winding.
 *
 * model - initialized GLMmodel structure
 */
GLvoid glmFacetNormals(GLMmodel* model)
{
    GLuint  i;
    GLfloat u[3];
    GLfloat v[3];
    
    assert(model);
    assert(model->vertices);
    
    /* clobber any old facetnormals */
    if (model->facetnorms)
        free(model->facetnorms);
    
    /* allocate memory for the new facet normals */
    model->numfacetnorms = model->numtriangles;
    model->facetnorms = (GLfloat*)malloc(sizeof(GLfloat) *
                       3 * (model->numfacetnorms + 1));
    
    for (i = 0; i < model->numtriangles; i++) {
        model->triangles[i].findex = i+1;
        
        u[0] = model->vertices[3 * T(i).vindices[1] + 0] -
            model->vertices[3 * T(i).vindices[0] + 0];
        u[1] = model->vertices[3 * T(i).vindices[1] + 1] -
            model->vertices[3 * T(i).vindices[0] + 1];
        u[2] = model->vertices[3 * T(i).vindices[1] + 2] -
            model->vertices[3 * T(i).vindices[0] + 2];
        
        v[0] = model->vertices[3 * T(i).vindices[2] + 0] -
            model->vertices[3 * T(i).vindices[0] + 0];
        v[1] = model->vertices[3 * T(i).vindices[2] + 1] -
            model->vertices[3 * T(i).vindices[0] + 1];
        v[2] = model->vertices[3 * T(i).vindices[2] + 2] -
            model->vertices[3 * T(i).vindices[0] + 2];
        
        glmCross(u, v, &model->facetnorms[3 * (i+1)]);
        glmNormalize(&model->facetnorms[3 * (i+1)]);
    }
}

GLvoid glmSetNormalsToFaceNormals(GLMmodel* model)
{
  /* nuke any previous normals */
  if (model->normals)
    free(model->normals);
    
  /* allocate space for new normals */
  model->numnormals = model->numtriangles; /* 1 normal per triangle */
  model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3 * (model->numnormals+1)); // 3 floats per each normal

  for (unsigned int i = 0; i < model->numtriangles; i++) 
  {
	  // get the facet normal
    float * normal = &(model->facetnorms[3 * (i+1)]);
	  // assign it to the correct slot
	  memcpy(&(model->normals[3*(i+1)]),normal,sizeof(float)*3);
    
	  // rewire indices of triangle normal
	  T(i).nindices[0] = T(i).nindices[1] = T(i).nindices[2] = i+1;
  }
}

GLvoid glmDeleteNeighborStructure(GLMmodel* model, GLMnode ** structure)
{
  for (unsigned int i = 1; i <= model->numvertices; i++)
    free(structure[i]);
  free(structure);
}

GLMnode ** glmBuildNeighborStructure(GLMmodel* model)
{
  GLMnode*    node;
  GLMnode** members;

  unsigned int i;

  /* allocate a structure that will hold a linked list of triangle
    indices for each vertex */
  members = (GLMnode**)malloc(sizeof(GLMnode*) * (model->numvertices + 1));
  for (i = 1; i <= model->numvertices; i++)
    members[i] = NULL;
    
  /* for every triangle, create a node for each vertex in it */
  for (i = 0; i < model->numtriangles; i++) 
  {
    node = (GLMnode*)malloc(sizeof(GLMnode));
    node->index = i;
    node->next  = members[T(i).vindices[0]];
    members[T(i).vindices[0]] = node;
        
    node = (GLMnode*)malloc(sizeof(GLMnode));
    node->index = i;
    node->next  = members[T(i).vindices[1]];
    members[T(i).vindices[1]] = node;
        
    node = (GLMnode*)malloc(sizeof(GLMnode));
    node->index = i;
    node->next  = members[T(i).vindices[2]];
    members[T(i).vindices[2]] = node;
  }

  return members;
}

GLvoid glmSetNormalsToFaceNormalsThresholded(GLMmodel* model, GLMnode ** neighborStructure, float thresholdAngle)
{
  // deallocate any previous normals 
  if ((model->numnormals < 3*model->numtriangles) && (model->normals))
  {
    free(model->normals);
    // allocate space for new normals 
    model->numnormals = 3*model->numtriangles; /* 3 normals per triangle */
    model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3 * (model->numnormals+1)); // 3 floats per each normal
  }

  float cosThreshold = cos(thresholdAngle * M_PI / 180.0);

  for (unsigned int i = 0; i < model->numtriangles; i++) 
  {
	// get the facet normal
    float * facetNormal = &(model->facetnorms[3 * (i+1)]);

    // go over all neighbors
	float normal[3];

	for(int vertex=0; vertex<3; vertex++)
	{
      int vertexID = T(i).vindices[vertex];
      GLMnode * node = neighborStructure[vertexID];
	  int numNeighbors = 0;
	  normal[0] = 0.0; normal[1] = 0.0; normal[2] = 0.0;
	  while (node)
	  {
        // node->index is 0-indexed neighboring triangle
	    float * neighborFacetNormal = &(model->facetnorms[3 * (node->index+1)]);
        if (glmDot(neighborFacetNormal, facetNormal) >= cosThreshold)
		{
          // angle is fat, add this normal to the average
          normal[0] += neighborFacetNormal[0];
		  normal[1] += neighborFacetNormal[1];
  		  normal[2] += neighborFacetNormal[2];
		}

        node = node->next;

		numNeighbors++;
	  }

	  if (numNeighbors == 0)
	  {
        printf("Warning: something is wrong with the neighboring triangle datastructure...\n");
        normal[0] = 0.0; normal[1] = 0.0; normal[2] = 1.0;
	  }

	  // normalize normal
      glmNormalize(normal);
	  
	  // assign it to the correct slot
	  int normalIndex = 3*i+1+vertex;
	  memcpy(&(model->normals[3*normalIndex]),normal,sizeof(float)*3);
    
	  // rewire index of triangle normal
	  T(i).nindices[vertex] = normalIndex;
	}
  }
}

/* glmVertexNormals: Generates smooth vertex normals for a model.
 * First builds a list of all the triangles each vertex is in.   Then
 * loops through each vertex in the the list averaging all the facet
 * normals of the triangles each vertex is in.   Finally, sets the
 * normal index in the triangle for the vertex to the generated smooth
 * normal.   If the dot product of a facet normal and the facet normal
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
GLvoid glmVertexNormals(GLMmodel* model, GLfloat angle)
{
    GLMnode*    node;
    GLMnode*    tail;
    GLMnode** members;
    GLfloat*    normals;
    GLuint  numnormals;
    GLfloat average[3];
    GLfloat dot, cos_angle;
    GLuint  i, avg;
    
    assert(model);
    assert(model->facetnorms);
    
    /* calculate the cosine of the angle (in degrees) */
    cos_angle = cos(angle * M_PI / 180.0);
    
    /* nuke any previous normals */
    if (model->normals)
        free(model->normals);
    
    /* allocate space for new normals */
    model->numnormals = model->numtriangles * 3; /* 3 normals per triangle */
    model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));
    
    /* allocate a structure that will hold a linked list of triangle
    indices for each vertex */
    members = (GLMnode**)malloc(sizeof(GLMnode*) * (model->numvertices + 1));
    for (i = 1; i <= model->numvertices; i++)
        members[i] = NULL;
    
    /* for every triangle, create a node for each vertex in it */
    for (i = 0; i < model->numtriangles; i++) {
        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[0]];
        members[T(i).vindices[0]] = node;
        
        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[1]];
        members[T(i).vindices[1]] = node;
        
        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[2]];
        members[T(i).vindices[2]] = node;
    }
    
    /* calculate the average normal for each vertex */
    numnormals = 1;
    for (i = 1; i <= model->numvertices; i++) {
    /* calculate an average normal for this vertex by averaging the
        facet normal of every triangle this vertex is in */
        node = members[i];
        //if (!node)
            //fprintf(stderr, "glmVertexNormals(): vertex w/o a triangle\n");
        average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
        avg = 0;
        while (node) {
        /* only average if the dot product of the angle between the two
        facet normals is greater than the cosine of the threshold
        angle -- or, said another way, the angle between the two
            facet normals is less than (or equal to) the threshold angle */
            dot = glmDot(&model->facetnorms[3 * T(node->index).findex],
                &model->facetnorms[3 * T(members[i]->index).findex]);
            if (dot > cos_angle) {
                node->averaged = GL_TRUE;
                average[0] += model->facetnorms[3 * T(node->index).findex + 0];
                average[1] += model->facetnorms[3 * T(node->index).findex + 1];
                average[2] += model->facetnorms[3 * T(node->index).findex + 2];
                avg = 1;            /* we averaged at least one normal! */
            } else {
                node->averaged = GL_FALSE;
            }
            node = node->next;
        }
        
        if (avg) {
            /* normalize the averaged normal */
            glmNormalize(average);
            
            /* add the normal to the vertex normals list */
            model->normals[3 * numnormals + 0] = average[0];
            model->normals[3 * numnormals + 1] = average[1];
            model->normals[3 * numnormals + 2] = average[2];
            avg = numnormals;
            numnormals++;
        }
        
        /* set the normal of this vertex in each triangle it is in */
        node = members[i];
        while (node) {
            if (node->averaged) {
                /* if this node was averaged, use the average normal */
                if (T(node->index).vindices[0] == i)
                    T(node->index).nindices[0] = avg;
                else if (T(node->index).vindices[1] == i)
                    T(node->index).nindices[1] = avg;
                else if (T(node->index).vindices[2] == i)
                    T(node->index).nindices[2] = avg;
            } else {
                /* if this node wasn't averaged, use the facet normal */
                model->normals[3 * numnormals + 0] = 
                    model->facetnorms[3 * T(node->index).findex + 0];
                model->normals[3 * numnormals + 1] = 
                    model->facetnorms[3 * T(node->index).findex + 1];
                model->normals[3 * numnormals + 2] = 
                    model->facetnorms[3 * T(node->index).findex + 2];
                if (T(node->index).vindices[0] == i)
                    T(node->index).nindices[0] = numnormals;
                else if (T(node->index).vindices[1] == i)
                    T(node->index).nindices[1] = numnormals;
                else if (T(node->index).vindices[2] == i)
                    T(node->index).nindices[2] = numnormals;
                numnormals++;
            }
            node = node->next;
        }
    }
    
    model->numnormals = numnormals - 1;
    
    /* free the member information */
    for (i = 1; i <= model->numvertices; i++) {
        node = members[i];
        while (node) {
            tail = node;
            node = node->next;
            free(tail);
        }
    }
    free(members);
    
    /* pack the normals array (we previously allocated the maximum
    number of normals that could possibly be created (numtriangles *
    3), so get rid of some of them (usually alot unless none of the
    facet normals were averaged)) */
    normals = model->normals;
    model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));
    for (i = 1; i <= model->numnormals; i++) 
	{
        model->normals[3 * i + 0] = normals[3 * i + 0];
        model->normals[3 * i + 1] = normals[3 * i + 1];
        model->normals[3 * i + 2] = normals[3 * i + 2];
    }
    free(normals);
}


/* glmLinearTexture: Generates texture coordinates according to a
 * linear projection of the texture map.  It generates these by
 * linearly mapping the vertices onto a square.
 *
 * model - pointer to initialized GLMmodel structure
 */
GLvoid glmLinearTexture(GLMmodel* model)
{
    GLMgroup *group;
    GLfloat dimensions[3];
    GLfloat x, y, scalefactor;
    GLuint i;
    
    assert(model);
    
    if (model->texcoords)
        free(model->texcoords);
    model->numtexcoords = model->numvertices;
    model->texcoords=(GLfloat*)malloc(sizeof(GLfloat)*2*(model->numtexcoords+1));
    
    glmDimensions(model, dimensions);
    scalefactor = 2.0 / 
        glmAbs(glmMax(glmMax(dimensions[0], dimensions[1]), dimensions[2]));
    
    /* do the calculations */
    for(i = 1; i <= model->numvertices; i++) {
        x = model->vertices[3 * i + 0] * scalefactor;
        y = model->vertices[3 * i + 2] * scalefactor;
        model->texcoords[2 * i + 0] = (x + 1.0) / 2.0;
        model->texcoords[2 * i + 1] = (y + 1.0) / 2.0;
    }
    
    /* go through and put texture coordinate indices in all the triangles */
    group = model->groups;
    while(group) {
        for(i = 0; i < group->numtriangles; i++) {
            T(group->triangles[i]).tindices[0] = T(group->triangles[i]).vindices[0];
            T(group->triangles[i]).tindices[1] = T(group->triangles[i]).vindices[1];
            T(group->triangles[i]).tindices[2] = T(group->triangles[i]).vindices[2];
        }    
        group = group->next;
    }
    
#if 0
    printf("glmLinearTexture(): generated %d linear texture coordinates\n",
        model->numtexcoords);
#endif
}

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
GLvoid glmSpheremapTexture(GLMmodel* model)
{
    GLMgroup* group;
    GLfloat theta, phi, rho, x, y, z, r;
    GLuint i;
    
    assert(model);
    assert(model->normals);
    
    if (model->texcoords)
        free(model->texcoords);
    model->numtexcoords = model->numnormals;
    model->texcoords=(GLfloat*)malloc(sizeof(GLfloat)*2*(model->numtexcoords+1));
    
    for (i = 1; i <= model->numnormals; i++) {
        z = model->normals[3 * i + 0];  /* re-arrange for pole distortion */
        y = model->normals[3 * i + 1];
        x = model->normals[3 * i + 2];
        r = sqrt((x * x) + (y * y));
        rho = sqrt((r * r) + (z * z));
        
        if(r == 0.0) {
            theta = 0.0;
            phi = 0.0;
        } else {
            if(z == 0.0)
                phi = 3.14159265 / 2.0;
            else
                phi = acos(z / rho);
            
            if(y == 0.0)
                theta = 3.141592365 / 2.0;
            else
                theta = asin(y / r) + (3.14159265 / 2.0);
        }
        
        model->texcoords[2 * i + 0] = theta / 3.14159265;
        model->texcoords[2 * i + 1] = phi / 3.14159265;
    }
    
    /* go through and put texcoord indices in all the triangles */
    group = model->groups;
    while(group) {
        for (i = 0; i < group->numtriangles; i++) {
            T(group->triangles[i]).tindices[0] = T(group->triangles[i]).nindices[0];
            T(group->triangles[i]).tindices[1] = T(group->triangles[i]).nindices[1];
            T(group->triangles[i]).tindices[2] = T(group->triangles[i]).nindices[2];
        }
        group = group->next;
    }
}

/* glmDelete: Deletes a GLMmodel structure.
 *
 * model - initialized GLMmodel structure
 */
GLvoid glmDelete(GLMmodel* model)
{
    GLMgroup* group;
    //GLuint i;
    
    assert(model);
    
    if (model->pathname)     free(model->pathname);
    if (model->mtllibname) free(model->mtllibname);
    if (model->vertices)     free(model->vertices);
    if (model->verticesRest)     free(model->verticesRest);
    if (model->normals)  free(model->normals);
    if (model->texcoords)  free(model->texcoords);
    if (model->facetnorms) free(model->facetnorms);
    if (model->triangles)  free(model->triangles);
    if (model->materials) 
      glmDeleteMaterials(model);
    while(model->groups) 
    {
      group = model->groups;
      model->groups = model->groups->next;
      free(group->name);
      free(group->triangles);
      free(group->edges);
      free(group);
    }
    if (model->directory)  free(model->directory);
    
    free(model);
}

void glmSetMemoryAllocationMode(int mode)
{
  memoryAllocationMode = mode;
}

void glmSetMemoryAllocationSizes(
  int maxNumTriangles_g, int maxNumVertices_g, int maxNumNormals_g, int maxNumTexCoords_g)
{
  maxNumTriangles = maxNumTriangles_g;
  maxNumVertices = maxNumVertices_g;
  maxNumNormals = maxNumNormals_g;
  maxNumTexCoords = maxNumTexCoords_g;
}


/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.  
 */
GLMmodel * glmReadOBJ(char* filename)
{
    GLMmodel * model;
    FILE * file;
    
    /* open the file */
    file = fopen(filename, "r");
    if (!file) 
    {
      fprintf(stderr, 
        "glmReadOBJ() failed: can't open data file \"%s\".\n", filename);
      exit(1);
    }
    
    /* allocate a new model */
    model = (GLMmodel*) malloc (sizeof(GLMmodel));
    model->pathname    = glmDuplicateString(filename);
    model->mtllibname    = NULL;
    model->numvertices   = 0;
    model->vertices    = NULL;
    model->verticesRest = NULL;
    model->numnormals    = 0;
    model->normals     = NULL;
    model->numtexcoords  = 0;
    model->texcoords       = NULL;
    model->numfacetnorms = 0;
    model->facetnorms    = NULL;
    model->numtriangles  = 0;
    model->triangles       = NULL;
    model->nummaterials  = 0;
    model->materials       = NULL;
    model->numgroups       = 0;
    model->groups      = NULL;
    model->position[0]   = 0.0;
    model->position[1]   = 0.0;
    model->position[2]   = 0.0;
    
    // now directory is known
    // make a copy for possible future use (as in loading textures)
    model->directory=glmDirName(filename);

    // make one default material just in case there is no usemtl tag
    glmMakeDefaultMaterials(model,1);

    /* make a first pass through the file to get a count of the number
    of vertices, normals, texcoords & triangles */

    glmFirstPass(model, file);
    
    GLMgroup * group;

    if (memoryAllocationMode == GLM_TIGHT)
    {
      /* allocate memory for the triangles in each group */
      group = model->groups;
      while(group) 
      {
        group->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
        group->numtriangles = 0;
        group->edges = (GLuint*)malloc(sizeof(GLuint) * group->numEdges * 2);
        group->numEdges = 0;
        group = group->next;
      }

      //printf("--------- Allocator: num vertices = %d\n", model->numvertices);
      model->vertices = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (model->numvertices + 1));

      model->verticesRest = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (model->numvertices + 1));

      model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) * model->numtriangles);

      if (model->numnormals) 
      {
          model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
              3 * (model->numnormals + 1));
      }

      if (model->numtexcoords) 
      {
          model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
              2 * (model->numtexcoords + 1));
      }
    }
    else
    {
      /* allocate memory */

      group = model->groups;
      while(group) 
      {
        group->triangles = (GLuint*)malloc(sizeof(GLuint) * maxNumTriangles);
        group->numtriangles = 0;
        group->edges = (GLuint*)malloc(sizeof(GLuint) * group->numEdges * 2);
        group->numEdges = 0;
        group = group->next;
      }

      model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
          3 * (maxNumVertices + 1));

      model->verticesRest = (GLfloat*)malloc(sizeof(GLfloat) *
        3 * (maxNumVertices + 1));

      model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
          maxNumTriangles);

      model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
              3 * (maxNumNormals + 1));

      model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
              2 * (maxNumTexCoords + 1));
    }

    /* rewind to beginning of file and read in the data this pass */
    rewind(file);
    
    glmSecondPass(model, file);
    
    /* close the file */
    fclose(file);

    // copy vertices into their rest position
    for (unsigned int i=1; i <= model->numvertices; i++)
    {
      model->verticesRest[3*i+0] = model->vertices[3*i+0];
      model->verticesRest[3*i+1] = model->vertices[3*i+1];
      model->verticesRest[3*i+2] = model->vertices[3*i+2];
    }

    return model;
}

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
 * a file.
 *
 * model - initialized GLMmodel structure
 * filename - name of the file to write the Wavefront .OBJ format data to
 * mode  - a bitwise or of values describing what is written to the file
 *             GLM_NONE     -  render with only vertices
 *             GLM_FLAT     -  render with facet normals
 *             GLM_SMOOTH   -  render with vertex normals
 *             GLM_TEXTURE  -  render with texture coords
 *             GLM_COLOR    -  render with colors (color material)
 *             GLM_MATERIAL -  render with materials
 *             GLM_COLOR and GLM_MATERIAL should not both be specified.  
 *             GLM_FLAT and GLM_SMOOTH should not both be specified.  
 */
GLvoid
glmWriteOBJ(GLMmodel* model, char* filename, GLuint mode, int writeMTL)
{
    GLuint  i;
    FILE*   file;
    GLMgroup* group;
    
    assert(model);
    
    /* do a bit of warning */
    if (mode & GLM_FLAT && !model->facetnorms) {
        printf("glmWriteOBJ() warning: flat normal output requested "
            "with no facet normals defined.\n");
        mode &= ~GLM_FLAT;
    }
    if (mode & GLM_SMOOTH && !model->normals) {
        printf("glmWriteOBJ() warning: smooth normal output requested "
            "with no normals defined.\n");
        mode &= ~GLM_SMOOTH;
    }
    if (mode & GLM_TEXTURE && !model->texcoords) {
        printf("glmWriteOBJ() warning: texture coordinate output requested "
            "with no texture coordinates defined.\n");
        mode &= ~GLM_TEXTURE;
    }
    if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
        printf("glmWriteOBJ() warning: flat normal output requested "
            "and smooth normal output requested (using smooth).\n");
        mode &= ~GLM_FLAT;
    }
    if (mode & GLM_COLOR && !model->materials) {
        printf("glmWriteOBJ() warning: color output requested "
            "with no colors (materials) defined.\n");
        mode &= ~GLM_COLOR;
    }
    if (mode & GLM_MATERIAL && !model->materials) {
        printf("glmWriteOBJ() warning: material output requested "
            "with no materials defined.\n");
        mode &= ~GLM_MATERIAL;
    }
    if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
        printf("glmWriteOBJ() warning: color and material output requested "
            "outputting only materials.\n");
        mode &= ~GLM_COLOR;
    }
    
    
    /* open the file */
    file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "glmWriteOBJ() failed: can't open file \"%s\" to write.\n",
            filename);
        exit(1);
    }

    /* spit out a header */
    fprintf(file, "#  \n");
    fprintf(file, "#  Wavefront OBJ generated by GLM library\n");
    fprintf(file, "#  \n");
    fprintf(file, "#  GLM library\n");
    fprintf(file, "#  Nate Robins\n");
    fprintf(file, "#  ndr@pobox.com\n");
    fprintf(file, "#  http://www.pobox.com/~ndr\n");
    fprintf(file, "#  \n");
    
    if (writeMTL && (mode & GLM_MATERIAL && model->mtllibname)) {
        fprintf(file, "\nmtllib %s\n\n", model->mtllibname);
        glmWriteMTL(model, filename, model->mtllibname);
    }

    /* spit out the vertices */
    fprintf(file, "\n");
    fprintf(file, "# %d vertices\n", model->numvertices);
    for (i = 1; i <= model->numvertices; i++) {
        fprintf(file, "v %f %f %f\n", 
            model->vertices[3 * i + 0],
            model->vertices[3 * i + 1],
            model->vertices[3 * i + 2]);
    }

    fflush(file);
    
    /* spit out the smooth/flat normals */
    if (mode & GLM_SMOOTH) {
        fprintf(file, "\n");
        fprintf(file, "# %d normals\n", model->numnormals);
        for (i = 1; i <= model->numnormals; i++) {
            fprintf(file, "vn %f %f %f\n", 
                model->normals[3 * i + 0],
                model->normals[3 * i + 1],
                model->normals[3 * i + 2]);
        }
    } else if (mode & GLM_FLAT) {
        fprintf(file, "\n");
        fprintf(file, "# %d normals\n", model->numfacetnorms);
        for (i = 1; i <= model->numnormals; i++) {
            fprintf(file, "vn %f %f %f\n", 
                model->facetnorms[3 * i + 0],
                model->facetnorms[3 * i + 1],
                model->facetnorms[3 * i + 2]);
        }
    }

    fflush(file);
    
    /* spit out the texture coordinates */
    if (mode & GLM_TEXTURE) {
        fprintf(file, "\n");
        fprintf(file, "# %d texcoords\n", model->numtexcoords);
        for (i = 1; i <= model->numtexcoords; i++) {
            fprintf(file, "vt %f %f\n", 
                model->texcoords[2 * i + 0],
                model->texcoords[2 * i + 1]);
        }
    }

    fflush(file);
    
    fprintf(file, "\n");
    fprintf(file, "# %d groups\n", model->numgroups);
    fprintf(file, "# %d faces (triangles)\n", model->numtriangles);
    fprintf(file, "\n");
    
    group = model->groups;
    while(group) {
        fprintf(file, "g %s\n", group->name);
        if (mode & GLM_MATERIAL)
            fprintf(file, "usemtl %s\n", model->materials[group->material].name);
        for (i = 0; i < group->numtriangles; i++) {
            if (mode & GLM_SMOOTH && mode & GLM_TEXTURE) {
                fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                    T(group->triangles[i]).vindices[0], 
                    T(group->triangles[i]).tindices[0],
                    T(group->triangles[i]).nindices[0], 
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).tindices[1],
                    T(group->triangles[i]).nindices[1],
                    T(group->triangles[i]).vindices[2],
                    T(group->triangles[i]).tindices[2],
                    T(group->triangles[i]).nindices[2]);
            } else if (mode & GLM_FLAT && mode & GLM_TEXTURE) {
                fprintf(file, "f %d/%d %d/%d %d/%d\n",
                    T(group->triangles[i]).vindices[0],
                    T(group->triangles[i]).findex,
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).findex,
                    T(group->triangles[i]).vindices[2],
                    T(group->triangles[i]).findex);
            } else if (mode & GLM_TEXTURE) {
                fprintf(file, "f %d/%d %d/%d %d/%d\n",
                    T(group->triangles[i]).vindices[0],
                    T(group->triangles[i]).tindices[0],
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).tindices[1],
                    T(group->triangles[i]).vindices[2],
                    T(group->triangles[i]).tindices[2]);
            } else if (mode & GLM_SMOOTH) {
                fprintf(file, "f %d//%d %d//%d %d//%d\n",
                    T(group->triangles[i]).vindices[0],
                    T(group->triangles[i]).nindices[0],
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).nindices[1],
                    T(group->triangles[i]).vindices[2], 
                    T(group->triangles[i]).nindices[2]);
            } else if (mode & GLM_FLAT) {
                fprintf(file, "f %d//%d %d//%d %d//%d\n",
                    T(group->triangles[i]).vindices[0], 
                    T(group->triangles[i]).findex,
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).findex,
                    T(group->triangles[i]).vindices[2],
                    T(group->triangles[i]).findex);
            } else {
                fprintf(file, "f %d %d %d\n",
                    T(group->triangles[i]).vindices[0],
                    T(group->triangles[i]).vindices[1],
                    T(group->triangles[i]).vindices[2]);
            }
        }
        fprintf(file, "\n");
        group = group->next;
    }
    
    fclose(file);
}

int glmSetUpTextures(GLMmodel* model, int mode)
{
  if (!model->texcoords) 
  {
    printf("glmSetUpTextures() warning: texture setup requested "
           "with no texture coordinates defined.\n");
  }

  for (unsigned int i=0; i < model->nummaterials; i++)
  {
    printf("Initializing material %d: %s\n",i,model->materials[i].name);

    if (!model->materials[i].textureFile) 
    {
      printf("  No texture file defined for this material.\n");
      continue; // no texture file defined for this material
    }
  
    // set up texture map
    // make textures
    int width, height;
    char textureFile[4096];
    sprintf(textureFile,"%s/%s",model->directory,model->materials[i].textureFile);

    printf("  Loading texture from %s...\n  ", textureFile);
    model->materials[i].textureData = glmReadPPM(textureFile, &width, &height);
    if (model->materials[i].textureData == NULL)
    {
      printf("  Unable to load texture from %s.\n", textureFile);
      return 1;
    }

/*
        width=128;
        height=128;
	model->materials[i].textureData = (unsigned char*)malloc(sizeof(unsigned char)*width*height*3);
        for(int ii=0;ii<width; ii++)
          for(int jj=0;jj<height; jj++)
          {
            model->materials[i].textureData[3*(width*jj+ii)+0] = (ii/10 + jj/10)  % 2 == 0 ? (int)(1.0*255* ii / width ) : 0;
            model->materials[i].textureData[3*(width*jj+ii)+1] = 0;
            model->materials[i].textureData[3*(width*jj+ii)+2] = 0;
          }

	if (model->materials[i].textureData == NULL)
	{
	  printf("  Error: failed to read texture data.\n");
          exit (1);
	}
*/

    GLubyte * data = model->materials[i].textureData; 
    int twidth = width;
    int theight = height;
    printf("  Texture bitmap size is %d x %d\n",width, height);

/* // resize to power of 2
    int pow2 = 1;
    while ((width > pow2) || (height > pow2))
      pow2 *= 2;

    pow2 = 256;

    twidth = pow2;
    theight = pow2;

    printf("  Resizing to %d x %d .\n" , twidth, theight);
    data = (GLubyte*) malloc (sizeof(GLubyte) * 3 * twidth * theight);
    gluScaleImage(GL_RGB, width, height, GL_UNSIGNED_BYTE, model->materials[i].textureData, twidth, theight, GL_UNSIGNED_BYTE, data);
*/

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&(model->materials[i].textureName));
    glBindTexture(GL_TEXTURE_2D, model->materials[i].textureName);

    //printf("  Texture identifier is %d\n",model->materials[i].textureName);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //GL_NEAREST,GL_LINEAR,GL_LINEAR_MIPMAP_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    if ((mode & GLM_MIPMAP) == GLM_NOMIPMAP)
    {
      printf("Not using mipmaps.\n");
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
      printf("Using mipmaps.\n");
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    model->textureMode = mode & GLM_LIGHTINGMODULATION;

    if ((mode & GLM_LIGHTINGMODULATION) == GLM_REPLACE)
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    else
    if ((mode & GLM_LIGHTINGMODULATION) == GLM_MODULATE)
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    else
    {
      printf("  Warning: unknown texture lighting interaction mode specified.\n");
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }

    if ((mode & GLM_MIPMAP) == GLM_NOMIPMAP)
      glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    else
      gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,  
        twidth, theight, GL_RGB, GL_UNSIGNED_BYTE, data);
 

/*
    printf("  Generating mipmaps...\n");fflush(NULL);

    if (gluBuild2DMipmaps(GL_TEXTURE_2D,3,width,height,GL_RGB,GL_UNSIGNED_BYTE, model->materials[i].textureData) != 0)
      printf("  Warning: could not create mipmaps for the textures.\n");
*/

    glEnable(GL_TEXTURE_2D);
    printf("Texture loaded.\n");fflush(NULL);
  }

  return 0;
}

GLvoid glmDrawEdges(GLMmodel * model, char * groupName)
{
  assert(model);

  GLMgroup * group = model->groups;
  glBegin(GL_LINES);
  while (group)
  {
    if (groupName != NULL)
    {
      if (strcmp(groupName, group->name) != 0)
      {
        group = group->next;
        continue;
      }
    }

    for (GLuint i = 0; i < group->numEdges; i++)
    {
      float * vtx0 = &(model->vertices[3*group->edges[2*i+0]]);
      glVertex3f(vtx0[0], vtx0[1], vtx0[2]);
      float * vtx1 = &(model->vertices[3*group->edges[2*i+1]]);
      glVertex3f(vtx1[0], vtx1[1], vtx1[2]);
    }
    group = group->next;
  }
  glEnd();
}

/* glmDraw: Renders the model to the current OpenGL context using the
 * mode specified.
 *
 * model - initialized GLMmodel structure
 * mode  - a bitwise OR of values describing what is to be rendered.
 *             GLM_NONE     -  render with only vertices
 *             GLM_FLAT     -  render with facet normals
 *             GLM_SMOOTH   -  render with vertex normals
 *             GLM_TEXTURE  -  render with texture coords
 *             GLM_COLOR    -  render with colors (color material)
 *             GLM_MATERIAL -  render with materials
 *             GLM_COLOR and GLM_MATERIAL should not both be specified.  
 *             GLM_FLAT and GLM_SMOOTH should not both be specified.  
 *             GLM_TEXTURE to use textures
 */
GLvoid glmDraw(GLMmodel* model, unsigned int mode)
{
  GLuint i;
  GLMgroup* group;
  GLMtriangle* triangle;
  GLMmaterial* material = NULL;
    
  assert(model);
  assert(model->vertices);
    
  // do a bit of warning 
  if (mode & GLM_FLAT && !model->facetnorms) 
  {
      printf("glmDraw() warning: flat render mode requested "
          "with no facet normals defined.\n");
      mode &= ~GLM_FLAT;
  }
  if (mode & GLM_SMOOTH && !model->normals) 
  {
      printf("glmDraw() warning: smooth render mode requested "
          "with no normals defined.\n");
      mode &= ~GLM_SMOOTH;
  }
  if (mode & GLM_TEXTURE && !model->texcoords) 
  {
      printf("glmDraw() warning: texture render mode requested "
          "with no texture coordinates defined.\n");
      mode &= ~GLM_TEXTURE;
  }
  if (mode & GLM_FLAT && mode & GLM_SMOOTH) 
  {
      printf("glmDraw() warning: flat render mode requested "
          "and smooth render mode requested (using smooth).\n");
      mode &= ~GLM_FLAT;
  }
  if (mode & GLM_COLOR && !model->materials) 
  {
      printf("glmDraw() warning: color render mode requested "
          "with no materials defined.\n");
      mode &= ~GLM_COLOR;
  }
  if (mode & GLM_MATERIAL && !model->materials) 
  {
      printf("glmDraw() warning: material render mode requested "
          "with no materials defined.\n");
      mode &= ~GLM_MATERIAL;
  }
  if (mode & GLM_COLOR && mode & GLM_MATERIAL) 
  {
      printf("glmDraw() warning: color and material render mode requested "
          "using only material mode.\n");
      mode &= ~GLM_COLOR;
  }
	
  if (mode & GLM_COLOR)
    glEnable(GL_COLOR_MATERIAL);
  else 
    if (mode & GLM_MATERIAL)
      glDisable(GL_COLOR_MATERIAL);
    
  //glEnableClientState(GL_VERTEX_ARRAY);
  //glVertexPointer(3,GL_FLOAT,0,model->vertices);

  //perhaps this loop should be unrolled into material, color, flat,
  //smooth, etc. loops?  since most cpu's have good branch prediction
  //schemes (and these branches will always go one way), probably
  //wouldn't gain too much? 
  
  group = model->groups;
  while (group != NULL) 
  {
    if (group->numtriangles == 0)
    {
      group = group->next;
      continue;
    }

    glDisable(GL_TEXTURE_2D);
    if (mode & GLM_MATERIAL) 
    {
      material = &model->materials[group->material];

      //float green[4] = {0,2,0, 1};
      //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, green);
      //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, green);
      //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, green);
      //printf("amb: %G %G %G %G\n", material->ambient[0], material->ambient[1], material->ambient[2],  material->ambient[3]);
      //printf("dif: %G %G %G %G\n", material->diffuse[0], material->diffuse[1], material->diffuse[2],  material->diffuse[3]);
      //printf("spe: %G %G %G %G\n", material->specular[0], material->specular[1], material->specular[2],  material->specular[3]);

      //material->ambient[0] = 0;
      //material->ambient[1] = 0;
      //material->ambient[2] = 0;

      //material->diffuse[0] = 0;
      //material->diffuse[1] = 2;
      //material->diffuse[2] = 0;

      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
    }

	if(mode & GLM_TEXTURE)
		material = &model->materials[1];

    if ((mode & GLM_TEXTURE) && (material->textureData != NULL))
    {
      GLuint tname = material->textureName;
      glBindTexture(GL_TEXTURE_2D,tname);
      if (model->textureMode == GLM_REPLACE)
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      if (model->textureMode == GLM_MODULATE)
      {
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      }
      glEnable(GL_TEXTURE_2D);
    }
 
    if (mode & GLM_COLOR) 
    {
      glColor3fv(material->diffuse);
    }

    //for(int j=0;j<9; j++)
      //printf("%f ", model->vertices[3 * model->numvertices-6+j]);
    //printf("\n");

    //printf("gnum: %u\n", group->numtriangles);
    glBegin(GL_TRIANGLES);
    for (i = 0; i < group->numtriangles; i++) 
	  {
      triangle = &T(group->triangles[i]);
      
      //glVertex3fv(&model->vertices[3 * triangle->vindices[0]]);
      //glVertex3fv(&model->vertices[3 * triangle->vindices[1]]);
      //glVertex3fv(&model->vertices[3 * triangle->vindices[2]]);

/*
      for(int j=0; j<3; j++)
      {
        if ((triangle->vindices[j] < 1) || (triangle->vindices[j] > model->numvertices))
          printf("****** Error: i:%d j:%d: vindex: %d\n", i,j, triangle->vindices[j]);
      }
*/
      /*
      if (triangle->vindices[0] > model->numvertices - 10)
          continue;
      if (triangle->vindices[1] > model->numvertices - 10)
          continue;
      if (triangle->vindices[2] > model->numvertices - 10)
          continue;
*/

      //glNormal3f(1,0,0);
      //glVertex3fv(&model->vertices[3 * model->numvertices-0]);
      //glVertex3fv(&model->vertices[3 * model->numvertices-3]);
      //glVertex3fv(&model->vertices[3 * model->numvertices-6]);

      //glVertex3fv(&model->vertices[3 * model->numvertices-9]);
      //glVertex3fv(&model->vertices[3 * model->numvertices-12]);
      //glVertex3fv(&model->vertices[3 * model->numvertices-15]);

       
      if (mode & GLM_FLAT)
        glNormal3fv(&model->facetnorms[3 * triangle->findex]);
          
      // vertex 0
      
      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * triangle->nindices[0]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2 * triangle->tindices[0]]);
        
      glVertex3fv(&model->vertices[3 * triangle->vindices[0]]);

      // vertex 1
      
      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * triangle->nindices[1]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2 * triangle->tindices[1]]);
        
      glVertex3fv(&model->vertices[3 * triangle->vindices[1]]);
          
      // vertex 2
      
      if (mode & GLM_SMOOTH)
        glNormal3fv(&model->normals[3 * triangle->nindices[2]]);
      if (mode & GLM_TEXTURE)
        glTexCoord2fv(&model->texcoords[2 * triangle->tindices[2]]);
        
      glVertex3fv(&model->vertices[3 * triangle->vindices[2]]);

	    //glArrayElement(triangle->vindices[0]);
		  //glArrayElement(triangle->vindices[1]);
		  //glArrayElement(triangle->vindices[2]);
    }
    glEnd();
        
    group = group->next;
  }
}


/* glmList: Generates and returns a display list for the model using
 * the mode specified.
 *
 * model - initialized GLMmodel structure
 * mode  - a bitwise OR of values describing what is to be rendered.
 *             GLM_NONE     -  render with only vertices
 *             GLM_FLAT     -  render with facet normals
 *             GLM_SMOOTH   -  render with vertex normals
 *             GLM_TEXTURE  -  render with texture coords
 *             GLM_COLOR    -  render with colors (color material)
 *             GLM_MATERIAL -  render with materials
 *             GLM_COLOR and GLM_MATERIAL should not both be specified.  
 * GLM_FLAT and GLM_SMOOTH should not both be specified.  
 */
GLuint glmList(GLMmodel* model, unsigned int mode)
{
    GLuint list;
    list = glGenLists(1);
    glNewList(list, GL_COMPILE);
      glmDraw(model, mode);
    glEndList();
    return list;
}

GLuint glmListEdges(GLMmodel* model)
{
    GLuint list;
    
    list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glmDrawEdges(model);
    glEndList();
    
    return list;
}

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
 * each other.
 *
 * model   - initialized GLMmodel structure
 * epsilon     - maximum difference between vertices
 *               ( 0.00001 is a good start for a unitized model)
 *
 */
GLvoid
glmWeld(GLMmodel* model, GLfloat epsilon)
{
    GLfloat* vectors;
    GLfloat* copies;
    GLuint   numvectors;
    GLuint   i;
    
    /* vertices */
    numvectors = model->numvertices;
    vectors  = model->vertices;
    copies = glmWeldVectors(vectors, &numvectors, epsilon);
    
#if 0
    printf("glmWeld(): %d redundant vertices.\n", 
        model->numvertices - numvectors - 1);
#endif
    
    for (i = 0; i < model->numtriangles; i++) {
        T(i).vindices[0] = (GLuint)vectors[3 * T(i).vindices[0] + 0];
        T(i).vindices[1] = (GLuint)vectors[3 * T(i).vindices[1] + 0];
        T(i).vindices[2] = (GLuint)vectors[3 * T(i).vindices[2] + 0];
    }
    
    /* free space for old vertices */
    free(vectors);
    
    /* allocate space for the new vertices */
    model->numvertices = numvectors;
    model->vertices = (GLfloat*)malloc(sizeof(GLfloat) * 
        3 * (model->numvertices + 1));
    
    /* copy the optimized vertices into the actual vertex list */
    for (i = 1; i <= model->numvertices; i++) {
        model->vertices[3 * i + 0] = copies[3 * i + 0];
        model->vertices[3 * i + 1] = copies[3 * i + 1];
        model->vertices[3 * i + 2] = copies[3 * i + 2];
    }
    
    free(copies);
}

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
GLubyte* glmReadPPM(char* filename, int* width, int* height)
{
    FILE* fp;
    int i, w, h, d;
    unsigned char* image;
    char head[70];          /* max line <= 70 in PPM (per spec). */
    
    fp = fopen(filename, "rb");
    if (!fp) {
        perror(filename);
        return NULL;
    }
    
    /* grab first two chars of the file and make sure that it has the
       correct magic cookie for a raw PPM file. */
    fgets(head, 70, fp);
    if (strncmp(head, "P6", 2)) {
        fprintf(stderr, "%s: Not a raw PPM file\n", filename);
        return NULL;
    }
    
    /* grab the three elements in the header (width, height, maxval). */
    i = 0;
    while(i < 3) {
        fgets(head, 70, fp);
        if (head[0] == '#')     /* skip comments. */
            continue;
        if (i == 0)
            i += sscanf(head, "%d %d %d", &w, &h, &d);
        else if (i == 1)
            i += sscanf(head, "%d %d", &h, &d);
        else if (i == 2)
            i += sscanf(head, "%d", &d);
    }
    
    /* grab all the image data in one fell swoop. */
    image = (unsigned char*)malloc(sizeof(unsigned char)*w*h*3);
    if ((int)fread(image, sizeof(unsigned char), w*h*3, fp) < 3*w*h)
    {
      printf("Error accessing PPM file.\n");
      exit(1);
    }
    fclose(fp);
    
    *width = w;
    *height = h;
    return image;
}

GLvoid glmApplyDeformation(GLMmodel* model, float * U, float * q, int n, int r)
{
  for (unsigned int i=0; i < 3*model->numvertices; i++)
  {
    model->vertices[i+3] = model->verticesRest[i+3];
	for (int j=0; j < r; j++)
      model->vertices[i+3] += q[j] * U[3*n*j + i];
  }
}



GLvoid glmApplyDeformation(GLMmodel * model, float * u)
{
  for (unsigned int i=0; i < 3*model->numvertices; i++)
  {
    model->vertices[i+3] = model->verticesRest[i+3] + u[i];
  }
}

GLvoid glmApplyDeformation(GLMmodel * model, double * u)
{
  for (unsigned int i=0; i < 3*model->numvertices; i++)
  {
    model->vertices[i+3] = model->verticesRest[i+3] + u[i];
  }
}

GLvoid glmApplyDeformation(GLMmodel * model, float * u, int * mask)
{
  for (unsigned int i=0; i < model->numvertices; i++)
  {
    model->vertices[3*i+3] = model->verticesRest[3*i+3] + u[3*mask[i]+0];
    model->vertices[3*i+4] = model->verticesRest[3*i+4] + u[3*mask[i]+1];
    model->vertices[3*i+5] = model->verticesRest[3*i+5] + u[3*mask[i]+2];
  }
}


GLvoid glmApplyDeformation(GLMmodel * model, double * u, int * mask)
{
  for (unsigned int i=0; i < model->numvertices; i++)
  {
    model->vertices[3*i+3] = model->verticesRest[3*i+3] + u[3*mask[i]+0];
    model->vertices[3*i+4] = model->verticesRest[3*i+4] + u[3*mask[i]+1];
    model->vertices[3*i+5] = model->verticesRest[3*i+5] + u[3*mask[i]+2];
  }
}

GLvoid glmDrawPoints(GLMmodel * model)
{
  glBegin(GL_POINTS);
	
  for (unsigned int i=1; i <= model->numvertices; i++)
    glVertex3fv(&model->vertices[3*i]);

  glEnd();
}

GLvoid glmDrawPoints(GLMmodel * model, int * selectedVertices, int numSelectedVertices)
{
  glBegin(GL_POINTS);
                                                                                                                                                             
  for (int i=0; i < numSelectedVertices; i++)
    glVertex3fv(&model->vertices[3*selectedVertices[i]+3]);
                                                                                                                                                             
  glEnd();
}

GLvoid glmDrawSelectedPoints(GLMmodel * model, int * selectedVertices, int numSelectedVertices, int oneIndexed)
{
  glBegin(GL_POINTS);
    for (int i=0; i < numSelectedVertices; i++)
      glVertex3fv(&model->vertices[3*selectedVertices[i] + 3 - 3 * oneIndexed]);
  glEnd();
}

GLvoid glmDrawUnselectedPoints(GLMmodel * model, int * selectionArray)
{
  glBegin(GL_POINTS);

  for (unsigned int i=1; i <= model->numvertices; i++)
  {
	if (selectionArray[i] == 0)
      glVertex3fv(&model->vertices[3*i]);
  }

  glEnd();
}

GLvoid glmDrawPoints(GLMmodel * model, int start, int end)
{
  glBegin(GL_POINTS);
	
  for (int i=start; i < end; i++)
    glVertex3fv(&model->vertices[3*i]);

  glEnd();
}

GLvoid glmDrawPointsSelection(GLMmodel * model)
{
  for (unsigned int i=1; i <= model->numvertices; i++)
  {
	glLoadName(i);
	glBegin(GL_POINTS);
      glVertex3fv(&model->vertices[3*i]);
	glEnd();
  }
}

GLvoid glmDrawNormals(GLMmodel * model)
{
    static GLuint i;
    static GLMgroup* group;
    static GLMtriangle* triangle;
    //static GLMmaterial* material;

    group = model->groups;
    while (group) {

        for (i = 0; i < group->numtriangles; i++) {
            triangle = &T(group->triangles[i]);
            
			glBegin(GL_LINES);

			  for (int i=0; i<3; i++)
			  {
                int vertexID = triangle->vindices[i];

			    glVertex3fv(&model->vertices[3 * vertexID]);

                float nx,ny,nz;
				int normalID = triangle->nindices[i];
			    nx = model->normals[3 * normalID + 0];
			    ny = model->normals[3 * normalID + 1];
			    nz = model->normals[3 * normalID + 2];

			    float x,y,z;
	            x = model->vertices[3*vertexID+0] + 0.1 * nx;
	            y = model->vertices[3*vertexID+1] + 0.1 * ny;
	            z = model->vertices[3*vertexID+2] + 0.1 * nz;
	            glVertex3f(x,y,z);
              }

			glEnd();
        }
        
        group = group->next;
    }

}

int glmCopyVertex(GLMmodel * model, int i)
{

  model->numvertices++;
  model->vertices[3 * model->numvertices + 0] = model->vertices[3 * i + 0];            
  model->vertices[3 * model->numvertices + 1] = model->vertices[3 * i + 1];            
  model->vertices[3 * model->numvertices + 2] = model->vertices[3 * i + 2];            

  model->verticesRest[3 * model->numvertices + 0] = model->verticesRest[3 * i + 0];
  model->verticesRest[3 * model->numvertices + 1] = model->verticesRest[3 * i + 1];
  model->verticesRest[3 * model->numvertices + 2] = model->verticesRest[3 * i + 2];

  return model->numvertices;
}



int glmAddVertex(GLMmodel * model, GLfloat x, GLfloat y, GLfloat z)
{

  model->numvertices++;
  model->vertices[3 * model->numvertices + 0] = x;            
  model->vertices[3 * model->numvertices + 1] = y;            
  model->vertices[3 * model->numvertices + 2] = z;            

  model->verticesRest[3 * model->numvertices + 0] = x;
  model->verticesRest[3 * model->numvertices + 1] = y;
  model->verticesRest[3 * model->numvertices + 2] = z;

  return model->numvertices;
}


int glmAddNormal(GLMmodel * model, GLfloat x, GLfloat y, GLfloat z)
{
  model->numnormals++;
  model->normals[3 * model->numnormals + 0] = x;            
  model->normals[3 * model->numnormals + 1] = y;            
  model->normals[3 * model->numnormals + 2] = z;            

  return model->numnormals;

}

int glmAddTexCoord(GLMmodel * model, GLfloat u, GLfloat v)
{
  model->numtexcoords++;
  model->texcoords[2 * model->numtexcoords + 0] = u;            
  model->texcoords[2 * model->numtexcoords + 1] = v;            

  return model->numtexcoords;
}

// adds a triangle spanned on the given indices/normals/texcoords
void glmAddTriangle(GLMmodel * model, GLMgroup * group, int v1, int v2, int v3, int n1, int n2, int n3, int t1, int t2, int t3)
{
    int numtriangles = model->numtriangles;

	T(numtriangles).vindices[0] = v1;
	T(numtriangles).tindices[0] = t1;
	T(numtriangles).nindices[0] = n1;

	T(numtriangles).vindices[1] = v2;
	T(numtriangles).tindices[1] = t2;
	T(numtriangles).nindices[1] = n2;

	T(numtriangles).vindices[2] = v3;
	T(numtriangles).tindices[2] = t3;
	T(numtriangles).nindices[2] = n3;
	group->triangles[group->numtriangles++] = numtriangles;

	model->numtriangles++;
}

void glmAddTriangle(GLMmodel * model, GLMgroup * group, int v1, int v2, int v3, int n1, int n2, int n3)
{
  glmAddTriangle(model, group, v1, v2, v3, n1, n2, n3, -1, -1, -1);
}

void glmAddTriangle(GLMmodel * model, GLMgroup * group, int v1, int v2, int v3)
{
  glmAddTriangle(model, group, v1, v2, v3, -1, -1, -1, -1, -1, -1);
}

void glmGetTriangle(GLMmodel * model, int triangleID, int & i1, int & i2, int & i3)
{
  i1 = T(triangleID).vindices[0];
  i2 = T(triangleID).vindices[1];
  i3 = T(triangleID).vindices[2];
}

void glmSetTriangle(GLMmodel * model, int triangleID, int pos, int newValue)
{
  T(triangleID).vindices[pos] = newValue;
}

void glmGetVertex(GLMmodel * model, int index, float & v1, float & v2, float & v3)
{
  v1 = model->vertices[3 * index + 0];
  v2 = model->vertices[3 * index + 1];
  v3 = model->vertices[3 * index + 2];
}

void glmSetVertex(GLMmodel * model, int index, int pos, float value)
{
  model->vertices[3 * index + pos] = value;
}

void glmPermuteVertices(GLMmodel * model, int * permutation)
{
  // renumber the vertices
  unsigned int i;

  // first, make a copy
  float * copy = (float*) malloc (sizeof(float) * model->numvertices * 3);

  for (i=1; i <= model->numvertices; i++)
  {
	float x,y,z;
    glmGetVertex(model,i,x,y,z);
	copy[3*(i-1)+0] = x;
	copy[3*(i-1)+1] = y;
	copy[3*(i-1)+2] = z;
  }

  // write vertices in place
  for (i=1; i <= model->numvertices; i++)
  {
    glmSetVertex(model,permutation[i],0,copy[3*(i-1)+0]);
	glmSetVertex(model,permutation[i],1,copy[3*(i-1)+1]);
	glmSetVertex(model,permutation[i],2,copy[3*(i-1)+2]);
  }

  free(copy);

  // renumber the faces
  for(i=0; i < model->numtriangles; i++)
  {
    int i1 = T(i).vindices[0];
    int i2 = T(i).vindices[1];
    int i3 = T(i).vindices[2];
    
	T(i).vindices[0] = permutation[i1];
	T(i).vindices[1] = permutation[i2];
	T(i).vindices[2] = permutation[i3];
  }
}

/* Jernej Barbic */
// will compute the averaged normals for the model, and store them into normals
// assumed normals has been pre-allocated, to the size of 3 * sizeof(float) * model->numvertices
GLvoid glmVertexAveragedNormals(GLMmodel* model, float * normals)
{
    GLMnode*    node;
    GLMnode*    tail;
    GLMnode** members;
    GLuint  numnormals;
    GLfloat average[3];
    GLuint  i, avg;
    
    assert(model);
    assert(model->facetnorms);

    /* allocate a structure that will hold a linked list of triangle
    indices for each vertex */
    members = (GLMnode**)malloc(sizeof(GLMnode*) * (model->numvertices + 1));
    for (i = 1; i <= model->numvertices; i++)
      members[i] = NULL;
    
    /* for every triangle, create a node for each vertex in it */
    for (i = 0; i < model->numtriangles; i++) {
        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[0]];
        members[T(i).vindices[0]] = node;
        
        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[1]];
        members[T(i).vindices[1]] = node;
        
        node = (GLMnode*)malloc(sizeof(GLMnode));
        node->index = i;
        node->next  = members[T(i).vindices[2]];
        members[T(i).vindices[2]] = node;
    }
    
    // for each vertex i:
    //   traverse the list of all faces that contain the vertex i
    //   if angle among any face and the FIRST face on the list is greater than the treshold, do the averaging
    //   traverse all triangles:
    //     if a triangle contains vertex i:
    //       if i was averaged, register the average normal, as the normal for that vertex in that triangle
    //       else, register the triangle (face) normal as the normal for that vertex in that triangle

    // stores normals in an independent, separate array

    /* calculate the average normal for each vertex */
    numnormals = 0;
    for (i = 1; i <= model->numvertices; i++) 
	{
        /* calculate an average normal for this vertex by averaging the
        facet normal of every triangle this vertex is in */
        node = members[i];  // list of triangles containing node i
        //if (!node)
            //printf("glmVertexNormals(): vertex w/o a triangle\n");
        average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
        avg = 0;
        while (node) { // average the normals
            node->averaged = GL_TRUE;
            average[0] += model->facetnorms[3 * T(node->index).findex + 0];
            average[1] += model->facetnorms[3 * T(node->index).findex + 1];
            average[2] += model->facetnorms[3 * T(node->index).findex + 2];
            avg = 1;            /* we averaged at least one normal! */
            node = node->next;
        }
        
        if (avg) 
		{ 
		  // vertex had at least one triangle
          // normalize the averaged normal
          glmNormalize(average);
            
          /* add the normal to the vertex normals list */
          normals[3 * numnormals + 0] = average[0];
          normals[3 * numnormals + 1] = average[1];
          normals[3 * numnormals + 2] = average[2];
          avg = numnormals;
        }
        else
        {
          //printf("glmVertexNormals(): Setting zero normal for a vertex not belonging to any triangle\n");
          normals[3 * numnormals + 0] = 0;
          normals[3 * numnormals + 1] = 0;
          normals[3 * numnormals + 2] = 0;
        }

        numnormals++;
    }
    
    /* free the member information */
    for (i = 1; i <= model->numvertices; i++) {
        node = members[i];
        while (node) {
            tail = node;
            node = node->next;
            free(tail);
        }
    }
    free(members);

}
/*
// converts (most of) the geometry data from the model into a binary representation
GLvoid binarizeModelData(GLMmodel * model, int & numVertices, int & numTriangles,
							int & numGroups, )
{

  // read out num triangles
  fread(&numTriangles,sizeof(int),1,fin);

  vertices = (float*) malloc (sizeof(float) * 9 * numTriangles);
  verticesRest = (float*) malloc (sizeof(float) * 9 * numTriangles);
  normals = (float*) malloc (sizeof(float) * 9 * numTriangles);

  // read all vertices, three per triangle
  fread(vertices,sizeof(float),9 * numTriangles,fin);
  memcpy(verticesRest,vertices,sizeof(float) * 9 * numTriangles);

  // read original indices for each vertex
  originalIndices = (int*) malloc (sizeof(int) * 3 * numTriangles);
  fread(originalIndices,sizeof(int),3 * numTriangles,fin);

  // read num groups
  fread(&numGroups,sizeof(int),1,fin);

  Ka = (float*) malloc (sizeof(float) * 3 * numGroups);
  Kd = (float*) malloc (sizeof(float) * 3 * numGroups);
  Ks = (float*) malloc (sizeof(float) * 3 * numGroups);
  shininess = (float*) malloc (sizeof(float) * numGroups);
  groupSize = (int *) malloc (sizeof(int) * numGroups);

  // read ka, kd, ks material data for each group
  for(i=0; i < numGroups; i++)
  {
	fread(&groupSize[i],sizeof(int),1,fin);
    fread(&Ka[3*i],sizeof(float),3,fin);
	fread(&Kd[3*i],sizeof(float),3,fin);
	fread(&Ks[3*i],sizeof(float),3,fin);
	fread(&shininess[i],sizeof(float),1,fin);
  }
}
*/

// saves the model to a "flat" format, to be used for fast ARB vertex rendering
GLvoid saveToFlatFormat(GLMmodel* model, char * filename)
{
  unsigned int i;
  FILE * fout;
  fout = fopen(filename,"wb");
  if (!fout)
  {
    printf("Error: couldn't open filename %s .\n",filename);
	return;
  }
/*
  if ((int)(fread(&nVertices_,sizeof(int),1,fin)) < 1)
    return 1;
*/

  // write out num triangles
  fwrite(&model->numtriangles,sizeof(int),1,fout);
  fwrite(&model->numvertices,sizeof(int),1,fout);

  // write out all vertices, three per triangle
  GLMtriangle * triangle;
  GLMgroup * group;
  GLMmaterial * material;

  group = model->groups;
  while (group) 
  {
    for (i = 0; i < group->numtriangles; i++) 
	  {
      triangle = &T(group->triangles[i]);
      fwrite(&model->vertices[3 * triangle->vindices[0]],sizeof(float),3,fout);
	    fwrite(&model->vertices[3 * triangle->vindices[1]],sizeof(float),3,fout);
	    fwrite(&model->vertices[3 * triangle->vindices[2]],sizeof(float),3,fout);
	  } 
	  group = group->next;
  }
  
  // write out original indices for each vertex
  group = model->groups;
  while (group) 
  {
    for (i = 0; i < group->numtriangles; i++) 
	{
      triangle = &T(group->triangles[i]);

	  int i1 = triangle->vindices[0]-1;
      fwrite(&i1,sizeof(int),1,fout);

	  int i2 = triangle->vindices[1]-1;
	  fwrite(&i2,sizeof(int),1,fout);

	  int i3 = triangle->vindices[2]-1;
	  fwrite(&i3,sizeof(int),1,fout);
	} 
	group = group->next;
  }
 
  // write out num groups
  fwrite(&model->numgroups,sizeof(int),1,fout);

  // write out ka, kd, ks material data for each group
  group = model->groups;
  while (group) 
  {
    fwrite(&group->numtriangles,sizeof(int),1,fout);

    material = &model->materials[group->material];
    fwrite(&material->ambient,sizeof(float),3,fout);
    fwrite(&material->diffuse,sizeof(float),3,fout);
    fwrite(&material->specular,sizeof(float),3,fout);
    fwrite(&material->shininess,sizeof(float),1,fout);
    group = group->next;
  }    

  fclose(fout);
}

GLvoid glmPrint_bitmap_string(float x,float y, float z, char* s)
{
   glRasterPos3f(x,y,z);
   if (s && strlen(s)) {
      while (*s) {
         glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *s);
         s++;
      }
   }
}

GLvoid glmPrint_bitmap_integer(float x,float y, float z,long i)
{
  char s[50];

  sprintf(s,"%ld",i);

  glmPrint_bitmap_string(x,y,z,s);

}

int glmClosestVertex(GLMmodel * model, double queryPosX, double queryPosY, double queryPosZ)
{
  double closestDist2 = DBL_MAX;
  int closestVertex=0;

  for (int i=1; i <= (int)model->numvertices; i++)
  {
    double posX = model->vertices[3 * i + 0];
	double posY = model->vertices[3 * i + 1];
	double posZ = model->vertices[3 * i + 2];
	double d2 = (queryPosX-posX)*(queryPosX-posX)+
		        (queryPosY-posY)*(queryPosY-posY)+
		        (queryPosZ-posZ)*(queryPosZ-posZ);

	if (d2 < closestDist2)
	{
	  closestDist2 = d2;
	  closestVertex = i-1;
	}
  }

  return closestVertex;
}






// shows all point labels, 1-indexing
GLvoid glmShowPointLabels(GLMmodel* model)
{
  glmShowPointLabels(model,1,(int)(model->numvertices));
}

// shows point labels from [k to l], 1-indexing
GLvoid glmShowPointLabels(GLMmodel* model, int k, int l)
{
  
  glColor3f(0,0,0);

  // show point labels
  // labels are printed out in the range 1... , not 0...
  for (int i=k; i<=l; i++)
    glmPrint_bitmap_integer(model->vertices[3*i],model->vertices[3*i+1],model->vertices[3*i+2],i);
}

GLvoid glmMeshGeometricParameters(GLMmodel * mesh, double * centerX, double * centerY, double * centerZ, double * radius)
{
  unsigned int i;
                                                                                                                                                             
  *centerX = 0.0;
  *centerY = 0.0;
  *centerZ = 0.0;
                                                                                                                                                             
  for(i=0; i < mesh->numvertices ; i++)
  {
    *centerX += mesh->vertices[3*i+3];
        *centerY += mesh->vertices[3*i+4];
        *centerZ += mesh->vertices[3*i+5];
  }
                                                                                                                                                             
  *centerX /= mesh->numvertices;
  *centerY /= mesh->numvertices;
  *centerZ /= mesh->numvertices;
                                                                                                                                                             
  glmMeshRadius(mesh,*centerX,*centerY,*centerZ,radius);
}
                                                                                                                                                             
                                                                                                                                                             
GLvoid glmMeshRadius(GLMmodel * mesh, double centerX, double centerY,double centerZ, double * radius)
{
  double radius2 = 0.0;
  unsigned int i;
  for(i=0; i < mesh->numvertices ; i++)
  {
    double dist2 = (centerX-mesh->vertices[3*i+3])*(centerX-mesh->vertices[3*i+3]) +
                           (centerY-mesh->vertices[3*i+4])*(centerY-mesh->vertices[3*i+4]) +
                                   (centerZ-mesh->vertices[3*i+5])*(centerZ-mesh->vertices[3*i+5]);
                                                                                                                                                             
        if (dist2 > radius2)
          radius2 = dist2;
  }
                                                                                                                                                             
  *radius = sqrt(radius2);
                                                                                                                                                             
}

GLvoid glmMeshData(GLMmodel * mesh, int * numVertices, double ** vertices, int * numTriangles, int ** triangles)
{
  *numVertices = mesh->numvertices;
  *numTriangles = mesh->numtriangles;

  *triangles = (int*) malloc (sizeof(int)*3*(*numTriangles));
  int pos = 0;
                                                                                                                                                             
  GLMgroup * group;
  group = mesh->groups;
  while (group)
  {
    for (unsigned ii = 0; ii < group->numtriangles; ii++)
    {
      GLMtriangle * triangle = &GLM_TRIANGLE(mesh,group->triangles[ii]);
      (*triangles)[pos] = triangle->vindices[0]-1; pos++;
      (*triangles)[pos] = triangle->vindices[1]-1; pos++;
      (*triangles)[pos] = triangle->vindices[2]-1; pos++;
    }
    group = group->next;
  }
                                                                                                                                                             
  *vertices = (double*) malloc (sizeof(double) * 3 * (*numVertices));
                                                                                                                                                             
  for(int i=0; i<3*(*numVertices); i++)
    (*vertices)[i] = mesh->vertices[i+3];
}

