/*
 * =====================================================================================
 *
 *       Filename:  IMRC_ploss_models.c
 *
 *    Description:  Pathloss models descriptions, these models are quite apparoximate
 *
 *        Version:  1.0
 *        Created:  07.07.2014 13:24:29
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
#include "IMRC_ploss_models.h"
#include "IMRC_rand_gen.h"
#include "IMRC_pretty_output.h"
#define NOT_MAIN
#include "IMRC_types.h"

/* !!!WARNING!!! This is for simplified model of suburban ONLY! */
#define DHM 10.5
#define X 15.0
#define DIST 80.0
#define DHB 25.0

extern float *gA, maxWidthNow, maxHeightNow;
extern unsigned int gASize;
/* Unreal mode */
#ifndef DEBUG
inline
#endif
float *prepareSilencing(unsigned int W, unsigned int H){
  unsigned int i = 0, j = 0;

  if(W == 0 || H == 0){
    printe("Can't initialize 0x0 field", __FILE__, __LINE__);
    return NULL;
  }

  maxWidthNow = W;
  maxHeightNow = H;

  gA = calloc(W*H, sizeof(float));

  gASize = W*H;

  for(j = 0; j < H; j++){
    for(i = 0; i < W; i++){
      *(gA + i + j*W) = genGauss();
    }
  }

  return gA;
}

/* Check whether we transmit data to the pReciever or not */
#ifndef DEBUG
inline
#endif
char isUseful(const RECIEVER *pReciever, const SENDER *pSender){
  RECIEVERS_LLIST *pTempR = NULL;

  if(!pReciever || !pSender){
    printe("Recieved NULL in isUseful", __FILE__, __LINE__);
    return -1;
  }

  pTempR = pSender->pRecepients;

  for(;pTempR; pTempR = pTempR->pNext){
    if(pTempR->pTarget == pReciever){
      return 1;
    }
  }
  return 0;
}

/* Calculate distance with Pythagoras theorem */
#ifndef DEBUG
inline
#endif
float distance_euclid(const RECIEVER *pRecvr,const SENDER *pSender){
  float dist = sqrt((pRecvr->x - pSender->x)*(pRecvr->x - pSender->x) + (pRecvr->y - pSender->y)*(pRecvr->y - pSender->y));
  if(dist < 1.0){
    dist = 1.0;
  }

  return dist;
}

/* Standart model power calculation. */
#ifndef DEBUG
inline
#endif
float power_simple(RECIEVER *pRecvr, const SENDER *pSender, const float amp){
  return ((isUseful(pRecvr, pSender)) ? (1.0) : (-1.0)) * pSender->power * (100.0)/(distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender) + amp);
}

/* Slightly more realistic model power calculation */
#ifndef DEBUG
inline
#endif
float power_complex(RECIEVER *pRecvr, const SENDER *pSender, const float Ht, const float Hr, const float freq){
  return (distance_euclid(pRecvr, pSender) == 0) ? (0) : (isUseful(pRecvr, pSender) ? (1.0) : (-1.0))*(pSender->power - 20.0*log10(4.0*M_PI/(3e8/pSender->freq)) + 2.0 * Hr - 40.0*log10(distance_euclid(pRecvr, pSender)));
}

/* Model for urban/suburban environments, simplified */
#ifndef DEBUG
inline
#endif
float power_urban_simple(RECIEVER *pRecvr, const SENDER *pSender){
  return ((isUseful(pRecvr, pSender)) ? (1.0) : (-1.0)) * (pSender->power - 40.0*(1.0 - 4.0*0.001*DHB)*log10(distance_euclid(pRecvr, pSender)/1000.0) - 21.0*log10(pSender->freq/1000000.0) - 49.0 + 18.0*log10(DHB));
}

/* Model for urban/suburban environments, complex */
#ifndef DEBUG
inline
#endif
float power_urban_complex(RECIEVER *pRecvr, const SENDER *pSender){
  return ((isUseful(pRecvr, pSender) ? (1.0) : (-1.0)) * (pSender->power + 10*log10(((3e16/pSender->freq)/(4.0*M_PI*distance_euclid(pRecvr, pSender)/1000.0))*((3e8/pSender->freq)/(4.0*M_PI*distance_euclid(pRecvr, pSender)/1000.0))) + 10.0*log10( (3e8/(2.0*M_PI*M_PI*sqrt(/* hm^2 */ DHM*DHM + /* x^2 */ X*X))) * (tan(abs(/* hm */DHM)/ /* x */X) - 1.0/(2.0*M_PI + pow(tan(abs(/* hm */DHM)/ /* x */X), -1.0))) * (tan(abs(/* hm */DHM)/ /* x */X) - 1.0/(2.0*M_PI + pow(tan(abs(/* hm */DHM)/ /* x */X), -1.0))) ) + 10.0*log10(5.52*pow((DHM/distance_euclid(pRecvr, pSender)/1000.0)*sqrt(DIST/(3e8/pSender->freq)), 1.8))  ));
}
