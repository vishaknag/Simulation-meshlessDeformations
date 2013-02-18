#include "render.h"

void glm_mix(double * A, double * source, double * dest)
{
  dest[0] = A[0] * source[0] + A[1] * source[1] + A[2] * source[2];
  dest[1] = A[3] * source[0] + A[4] * source[1] + A[5] * source[2];
  dest[2] = A[6] * source[0] + A[7] * source[1] + A[8] * source[2];
}

void glm_mix2(double * A, double * source, double * dest)
{
  dest[0] = A[0] * source[0] + A[1] * source[1];
  dest[1] = A[2] * source[0] + A[3] * source[1];
}

void glmEncode(GLMmodel * mesh)
{
/*
for(unsigned int i=0; i <= ~0; i++)
{
  if (i != glm_invf(glm_f(i)))
    printf("Error at i=%d\n", i);
}
*/

  double mixingMatrix[9] = {1, 0, -1, 2, 1, 0, -1, 1, 2};
  double mixingMatrix2[4] = {1, 0, -1, 2};

  //encode vertices
  GLfloat * verticesTemp = (GLfloat*) malloc (sizeof(GLfloat) * 3 * (mesh->numvertices + 1));
  verticesTemp[0] = verticesTemp[1] = verticesTemp[2] = 0.0;

  #define VERTEX(i,j) (mesh->vertices[3*(i) + 3 + (j)])

  for(unsigned int i=0; i<mesh->numvertices; i++)
  {
    int m1 = i;
    int m2 = (i+1) % mesh->numvertices;
    int m3 = (i+2) % mesh->numvertices;

    double source[3] = {VERTEX(m1,0), VERTEX(m2, 1), VERTEX(m3, 2)};
    double dest[3];
    glm_mix(mixingMatrix, source, dest);
    verticesTemp[3 * i + 3] = dest[0];
    verticesTemp[3 * i + 4] = dest[1];
    verticesTemp[3 * i + 5] = dest[2];
  }

  memcpy(mesh->vertices, verticesTemp, sizeof(GLfloat) * 3 * (mesh->numvertices + 1));
  free(verticesTemp);  

  //encode normals
  if (mesh->normals != NULL)
  {
    GLfloat * normalsTemp = (GLfloat*) malloc (sizeof(GLfloat) * 3 * (mesh->numnormals + 1));
    normalsTemp[0] = normalsTemp[1] = normalsTemp[2] = 0.0;

    #define NORMAL(i,j) (mesh->normals[3*(i) + 3 + (j)])

    for(unsigned int i=0; i<mesh->numnormals; i++)
    {
      int m1 = i;
      int m2 = (i+1) % mesh->numnormals;
      int m3 = (i+2) % mesh->numnormals;

      double source[3] = {NORMAL(m1,0), NORMAL(m2, 1), NORMAL(m3, 2)};
      double dest[3];
      glm_mix(mixingMatrix, source, dest);
      normalsTemp[3 * i + 3] = dest[0];
      normalsTemp[3 * i + 4] = dest[1];
      normalsTemp[3 * i + 5] = dest[2];
    }

    memcpy(mesh->normals, normalsTemp, sizeof(GLfloat) * 3 * (mesh->numnormals + 1));
    free(normalsTemp);  
  }

  // encode texture coords
  if (mesh->texcoords != NULL)
  {
    GLfloat * uvTemp = (GLfloat*) malloc (sizeof(GLfloat) * 2 * (mesh->numtexcoords + 1));
    uvTemp[0] = uvTemp[1] = 0.0;

    #define UV(i,j) (mesh->texcoords[2*(i) + 2 + (j)])

    for(unsigned int i=0; i<mesh->numtexcoords; i++)
    {
      int m1 = i;
      int m2 = (i+1) % mesh->numtexcoords;

      double source[2] = {UV(m1,0), UV(m2, 1)};
      double dest[2];
      glm_mix2(mixingMatrix2, source, dest);
      uvTemp[2 * i + 2] = dest[0];
      uvTemp[2 * i + 3] = dest[1];
    }

    memcpy(mesh->texcoords, uvTemp, sizeof(GLfloat) * 2 * (mesh->numtexcoords + 1));
    free(uvTemp);  
  }

  // encode triangles
  for(unsigned int i=0; i<mesh->numtriangles; i++)
  {
    for(int j=0; j<3; j++)
    {
      unsigned int value = (mesh->triangles)[i].vindices[j];
      (mesh->triangles)[i].vindices[j] = glm_f(value);
    }

    for(int j=0; j<3; j++)
    {
      unsigned int value = (mesh->triangles)[i].nindices[j];
      (mesh->triangles)[i].nindices[j] = glm_f(value);
    }

    for(int j=0; j<3; j++)
    {
      unsigned int value = (mesh->triangles)[i].tindices[j];
      (mesh->triangles)[i].tindices[j] = glm_f(value);
    }
  }
}

void glmDecode(GLMmodel * mesh)
{

  double invMixingMatrix[9] = {-2, 1, -1, 4, -1, 2, -3, 1, -1};
  double invMixingMatrix2[4] = {1, 0, 0.5, 0.5};

  //decode vertices
  GLfloat * verticesTemp = (GLfloat*) malloc (sizeof(GLfloat) * 3 * (mesh->numvertices + 1));
  verticesTemp[0] = verticesTemp[1] = verticesTemp[2] = 0.0;

  #define VERTEX(i,j) (mesh->vertices[3*(i) + 3 + (j)])

  for(unsigned int i=0; i<mesh->numvertices; i++)
  {
    double source[3] = {VERTEX(i,0), VERTEX(i, 1), VERTEX(i, 2)};
    double dest[3];
    glm_mix(invMixingMatrix, source, dest);

    int m1 = i;
    int m2 = (i+1) % mesh->numvertices;
    int m3 = (i+2) % mesh->numvertices;

    verticesTemp[3 * m1 + 3] = dest[0];
    verticesTemp[3 * m2 + 4] = dest[1];
    verticesTemp[3 * m3 + 5] = dest[2];
  }

  memcpy(mesh->vertices, verticesTemp, sizeof(GLfloat) * 3 * (mesh->numvertices + 1));
  memcpy(mesh->verticesRest, verticesTemp, sizeof(GLfloat) * 3 * (mesh->numvertices + 1));
  free(verticesTemp);  

  // decode normals
  if (mesh->normals != NULL)
  {
    GLfloat * normalsTemp = (GLfloat*) malloc (sizeof(GLfloat) * 3 * (mesh->numnormals + 1));
    normalsTemp[0] = normalsTemp[1] = normalsTemp[2] = 0.0;

    #define NORMAL(i,j) (mesh->normals[3*(i) + 3 + (j)])

    for(unsigned int i=0; i<mesh->numnormals; i++)
    {
      double source[3] = {NORMAL(i,0), NORMAL(i, 1), NORMAL(i, 2)};
      double dest[3];
      glm_mix(invMixingMatrix, source, dest);

      int m1 = i;
      int m2 = (i+1) % mesh->numnormals;
      int m3 = (i+2) % mesh->numnormals;

      normalsTemp[3 * m1 + 3] = dest[0];
      normalsTemp[3 * m2 + 4] = dest[1];
      normalsTemp[3 * m3 + 5] = dest[2];
    }

    memcpy(mesh->normals, normalsTemp, sizeof(GLfloat) * 3 * (mesh->numnormals + 1));
    free(normalsTemp);  
  }

  // decode texture coords
  if (mesh->texcoords != NULL)
  {
    GLfloat * uvTemp = (GLfloat*) malloc (sizeof(GLfloat) * 2 * (mesh->numtexcoords + 1));
    uvTemp[0] = uvTemp[1] = 0.0;

    #define UV(i,j) (mesh->texcoords[2*(i) + 2 + (j)])

    for(unsigned int i=0; i<mesh->numtexcoords; i++)
    {
      double source[2] = {UV(i,0), UV(i,1)};
      double dest[2];
      glm_mix2(invMixingMatrix2, source, dest);

      int m1 = i;
      int m2 = (i+1) % mesh->numtexcoords;

      uvTemp[2 * m1 + 2] = dest[0];
      uvTemp[2 * m2 + 3] = dest[1];
    }

    memcpy(mesh->texcoords, uvTemp, sizeof(GLfloat) * 2 * (mesh->numtexcoords + 1));
    free(uvTemp);
  }

  // decode triangles
  for(unsigned int i=0; i<mesh->numtriangles; i++)
  {
    for(int j=0; j<3; j++)
    {
      unsigned int value = (mesh->triangles)[i].vindices[j];
      (mesh->triangles)[i].vindices[j] = glm_invf(value);
    }

    for(int j=0; j<3; j++)
    {
      unsigned int value = (mesh->triangles)[i].nindices[j];
      (mesh->triangles)[i].nindices[j] = glm_invf(value);
    }

    for(int j=0; j<3; j++)
    {
      unsigned int value = (mesh->triangles)[i].tindices[j];
      (mesh->triangles)[i].tindices[j] = glm_invf(value);
    }
  }

  // update edges
  GLMgroup * group = mesh->groups;
  while(group)
  {
    for(unsigned int i=0; i< group->numEdges; i++)
    {
      unsigned int value;
      value = (group->edges)[2*i+0];
      (group->edges)[2*i+0] = glm_invf(value);
      value = (group->edges)[2*i+1];
      (group->edges)[2*i+1] = glm_invf(value);
    }
    group = group->next;
  }
}

