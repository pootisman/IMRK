#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "IMRC_models.h"
#include "IMRC_types.h"

/* Gaussian distribution generator, Box-Muller method */
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

/* Standart model power calculation. */
float power_simple(RECIEVER *pRecvr, const SENDER *pSender, const float amp){
  return ((pRecvr == pSender->pRecepient) ? (1) : (-1)) * pSender->power * (100.0)/(distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)) + genGauss()*amp;
}

/* Calculate total power applied to every reciever */
void calcPower(RECIEVER *pRecvrs, const SENDER *pSenders, const unsigned int nSends, const unsigned int nRecvs, const float amp){
  unsigned int i = 0, j = 0;
  float buffer = 0.0;

  if(!pRecvrs || !pSenders){
    (void)puts("Error, NULL provided!");
    return;
  }

  for(j = 0; j < nRecvs; j++){
    for(i = 0; i < nSends; i++){
      buffer = power_simple((pRecvrs + j), (pSenders + i), amp);
      if(buffer > 0.0){
        (pRecvrs + j)->signal += buffer;
      }else{
	(pRecvrs + j)->waste -= buffer;
      }
    }
    (pRecvrs + j)->SNRLin = (pRecvrs + j)->signal/(pRecvrs + j)->waste;
  }
}

/* Calculate distance with Pythagoras theorem */
float distance_euclid(const RECIEVER *pRecvr,const SENDER *pSender){
  return sqrt((pRecvr->x - pSender->x)*(pRecvr->x - pSender->x) + (pRecvr->y - pSender->y)*(pRecvr->y - pSender->y));
}
