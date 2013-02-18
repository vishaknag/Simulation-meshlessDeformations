#ifndef _GLME_ENCODE_H_
#define _GLME_ENCODE_H_

#include "glme.h"

inline unsigned int glm_f(unsigned int i)
{
  i = i ^ 0xD597A3B4;
  
  unsigned int r0 = i % 256;
  unsigned int r1 = (i >> 8) % 256;
  unsigned int r2 = (i >> 16) % 256;
  unsigned int r3 = (i >> 24) % 256;

  unsigned int r0mix = r0 * 251 % 256;
  unsigned int r1mix = r1 * 251 % 256;
  unsigned int r2mix = r2 * 251 % 256;
  unsigned int r3mix = r3 * 251 % 256;

  i = (r0mix << 24) + (r1mix << 16) + (r2mix << 8) + r3mix;

  return i;
}

inline unsigned int glm_invf(unsigned int i)
{
  unsigned int r0mix = i >> 24;
  unsigned int r1mix = (i >> 16) % 256;
  unsigned int r2mix = (i >> 8) % 256;
  unsigned int r3mix = i % 256;

  unsigned int r0 = r0mix * 51 % 256;
  unsigned int r1 = r1mix * 51 % 256;
  unsigned int r2 = r2mix * 51 % 256;
  unsigned int r3 = r3mix * 51 % 256;

  i = (r3 << 24) + (r2 << 16) + (r1 << 8) + r0;
  i = i ^ 0xD597A3B4;

  return i;
}

void glm_mix(double * A, double * source, double * dest);

void glmEncode(GLMmodel * mesh);

void glmDecode(GLMmodel * mesh);

#endif
