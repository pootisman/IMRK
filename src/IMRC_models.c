#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include "IMRC_models.h"
#include "IMRC_types.h"

float *A = NULL;
unsigned int Asize = 0;

typedef struct{
  unsigned int nSenders;
  unsigned int startIndex;
  unsigned int stopIndex;
  unsigned int W;
  RECIEVER *pRecvrs;
  const SENDER *pSenders;
}THREAD_PARAMS;

#ifndef DEBUG
/* Calculate distance with Pythagoras theorem */
inline float distance_euclid(const RECIEVER *pRecvr,const SENDER *pSender){
  float dist = sqrt((pRecvr->x - pSender->x)*(pRecvr->x - pSender->x) + (pRecvr->y - pSender->y)*(pRecvr->y - pSender->y));
  if(dist < 1.0){
    dist = 1.0;
  }
  return dist;
}
#else
/* Calculate distance with Pythagoras theorem */
float distance_euclid(const RECIEVER *pRecvr,const SENDER *pSender){
  float dist = sqrt((pRecvr->x - pSender->x)*(pRecvr->x - pSender->x) + (pRecvr->y - pSender->y)*(pRecvr->y - pSender->y));
  if(dist < 1.0){
    dist = 1.0;
  }
  return dist;
}
#endif

/* Function for threaded calculations */
void *threadPowerCalc(void *args){
  THREAD_PARAMS *task = (THREAD_PARAMS *)args;
  unsigned int i = 0, j = 0;
  float buffer = 0.0;

  if(!task){
    (void)puts("Error, NULL passed to thread.");
    (void)pthread_exit(NULL);
  }

  for(i = task->startIndex; i < task->stopIndex; i++){
    for(j = 0; j < task->nSenders; j++){
      buffer = power_simple((task->pRecvrs + i), (task->pSenders + j), *(A + (unsigned int)((task->pRecvrs + i)->x) + (unsigned int)(task->W*(task->pRecvrs + i)->y)));
      if(buffer > 0.0){
        (task->pRecvrs + i)->signal += buffer;
      }else{
	(task->pRecvrs + i)->waste -= buffer;
      }
    }
    (task->pRecvrs + i)->SNRLin = (task->pRecvrs + i)->signal/(task->pRecvrs + i)->waste;
  }

  (void)pthread_exit(NULL);
}

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

/* Unreal mode */
float *prepareSilencing(unsigned int W, unsigned int H){
  unsigned int i = 0, j = 0;

  A = calloc(W*H, sizeof(float));

  Asize = W*H;

  for(j = 0; j < H; j++){
    for(i = 0; i < W; i++){
      *(A + i + j*W) = genGauss()*6.0;
    }
  }

  return A;
}

/* Standart model power calculation. */
float power_simple(RECIEVER *pRecvr, const SENDER *pSender, const float amp){
  return ((pRecvr == pSender->pRecepient) ? (1) : (-1)) * pSender->power * (100.0)/(distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender) + genGauss()*amp);
}

/* Calculate total power applied to every reciever */
void calcPower(RECIEVER *pRecvrs, const SENDER *pSenders, const unsigned int nSends, const unsigned int nRecvs, unsigned int W, int Threads){
  unsigned int i = 0, j = 0;
  long int nThreads = 0;
  float buffer = 0.0;
  pthread_t *pThreads = NULL;
  pthread_attr_t threadAttr;
  THREAD_PARAMS *pTParams = NULL;

  if(!pRecvrs || !pSenders){
    (void)puts("Error, NULL provided!");
    return;
  }

  nThreads = sysconf(_SC_NPROCESSORS_ONLN);

  if(Threads != 0){
    nThreads = Threads;
  }

  if(nThreads <= 0){
    (void)puts("No multi-threading will be used.");
  }

  if(nThreads <= 0){
    for(j = 0; j < nRecvs; j++){
      for(i = 0; i < nSends; i++){
        buffer = power_simple((pRecvrs + j), (pSenders + i), *(A + (unsigned int)((pRecvrs + j)->x) + (unsigned int)(W*(pRecvrs + j)->y)));
        if(buffer > 0.0){
          (pRecvrs + j)->signal += buffer;
        }else{
	  (pRecvrs + j)->waste -= buffer;
        }
      }
      (pRecvrs + j)->SNRLin = (pRecvrs + j)->signal/(pRecvrs + j)->waste;
    }
  }else{
    pTParams = calloc(nThreads, sizeof(THREAD_PARAMS));
    pThreads = calloc(nThreads, sizeof(pthread_t));
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    /* Prepare tasks for threads */
    for(i = 0; i < nThreads; i++){
      (pTParams + i)->startIndex = i*(nRecvs/nThreads);
      (pTParams + i)->stopIndex = (i+1)*(nRecvs/nThreads);
      (pTParams + i)->pSenders = pSenders;
      (pTParams + i)->pRecvrs = pRecvrs;
      (pTParams + i)->nSenders = nSends;
      (pTParams + i)->W = W;
    }
    /* Start threads, except for the last one, we are the last thread */
    for(i = 0; i < nThreads - 1; i++){
      if(pthread_create((pThreads + i), &(threadAttr), threadPowerCalc, (void *)(pTParams + i))){
        (void)puts("Error while creating thread, retrying.");
	--i;
      }
    }
    /* Start calculating our part */
    for(j = (pTParams + nThreads - 1)->startIndex; j < (pTParams + nThreads - 1)->stopIndex; j++){
      for(i = 0; i < nSends; i++){
        buffer = power_simple(((pTParams + nThreads - 1)->pRecvrs + j), ((pTParams + nThreads -1)->pSenders + i), *(A + (unsigned int)(((pTParams + nThreads - 1)->pRecvrs + j)->x) + (unsigned int)(W*((pTParams + nThreads - 1)->pRecvrs + j)->y)));
        if(buffer > 0.0){
          ((pTParams + nThreads - 1)->pRecvrs + j)->signal += buffer;
        }else{
	  ((pTParams + nThreads - 1)->pRecvrs + j)->waste -= buffer;
        }
      }
      ((pTParams + nThreads - 1)->pRecvrs + j)->SNRLin = ((pTParams + nThreads - 1)->pRecvrs + j)->signal/((pTParams + nThreads - 1)->pRecvrs + j)->waste;
    }

    pthread_attr_destroy(&threadAttr);

    for(i = 0; i < nThreads - 1; i++){
      pthread_join(*(pThreads + i), NULL);
    }

    (void)free(pTParams);
    (void)free(pThreads);
  }
}

/* Create recievers within given bounds maxW, maxH */
void spawnRecievers(RECIEVER *pRecievers, const unsigned int nRecievers, const unsigned int maxW, const unsigned int maxH){
  unsigned int i = 0;

  if(!pRecievers || nRecievers == 0 || maxW == 0 || maxH == 0){
    (void)puts("Error, can't spawn recievers.");
    return;
  }

  for(i = 0; i < nRecievers; i++){
    (pRecievers + i)->x = ((float)rand()/(float)RAND_MAX)*maxW;
    (pRecievers + i)->y = ((float)rand()/(float)RAND_MAX)*maxH;
  }
}

/* Create transmitters within given bounds maxW,maxH */
void spawnTransmitters(SENDER *pSenders, RECIEVER *pRecievers, const unsigned int nSenders, const unsigned int nRecievers, const unsigned int maxW, const unsigned int maxH){
  unsigned int i = 0, j = 0;
  unsigned char *isTaken = NULL;

  if(!pRecievers || !pSenders || nRecievers == 0 || nSenders == 0 || maxW == 0 || maxH == 0){
    (void)puts("Error, can't spawn transmitters.");
    return;
  }

  isTaken = calloc(nRecievers ,sizeof(unsigned char));

  for(i = 0; i < nSenders; i++){
    (pSenders + i)->x = ((float)rand()/(float)RAND_MAX)*maxW;
    (pSenders + i)->y = ((float)rand()/(float)RAND_MAX)*maxH;
    (pSenders + i)->power = 1.0;
    do{
      j = rand()%nRecievers;
      if(*(isTaken + j) == 0){
	*(isTaken + j) = 1;
	(pSenders + i)->pRecepient = (pRecievers + j);
	j = 1;
      }else{
        j = 0;
      }
    }while(j != 1);
  }

  (void)free(isTaken);
}
