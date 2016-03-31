/*
 * =====================================================================================
 *
 *       Filename:  IMRC_rand_gen.c
 *
 *    Description:  Various random number generators.
 *
 *        Version:  1.0
 *        Created:  07.07.2014 14:00:41
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aleksei Ponomarenko-Timofeev (), alexeyponomarenko(at)gmail(dot)com
 *   Organization:  SELF
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <math.h>
#include "IMRC_rand_gen.h"

/* Gaussian distribution generator, Box-Muller method */
#ifndef DEBUG
inline
#endif
float genGauss(void){
  float U1 = 0.0f, U2 = 0.0f, V1 = 0.0f, V2 = 0.0f, S = 0.0f;

  do{
    U1= fabs((float)rand()/(float)RAND_MAX);            /* U1=[0    ,1] */
    U2= fabs((float)rand()/(float)RAND_MAX);            /* U2=[0    ,1] */
    V1= 2.0f * U1 - 1.0f;            /* V1=[-1,1] */
    V2= 2.0f * U2 - 1.0f;           /* V2=[-1,1] */
    S = V1 * V1 + V2 * V2;
  }
  while(S >= 1.0f);
  
  return sqrt(-1.0f * log(S) / S) * V1;
}

/* Exponential distribution generator, Box-Muller method */
#ifndef DEBUG
inline
#endif
float genExp(void){
  return -log((float)rand()/(float)RAND_MAX);
}
