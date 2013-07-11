#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include "IMRC_models.h"
#include "IMRC_types.h"

#define M_PI 3.14159265358979323846264338327

float *gA = NULL;
unsigned int gAsize = 0, gnSenders = 0;
SENDER *gpSenders = NULL;

typedef struct{
  unsigned int nSenders;
  unsigned int startIndex;
  unsigned int stopIndex;
  unsigned int W;
  unsigned int model;
  RECIEVER *pRecvrs;
  const SENDER *pSenders;
}THREAD_PARAMS;

#ifndef DEBUG
/* Check whether we transmit data to the pReciever or not */
inline char isUseful(const RECIEVER *pReciever, const SENDER *pSender){
  unsigned int i = 0;
  RECIEVERS_LLIST *temp = NULL;

  if(!pReciever || !pSender){
    (void)puts("Error, recieved NULL in isUseful.");
    return -1;
  }

  temp = pSender->pRecepients;

  if(temp != NULL){
    for(i = 0; i < pSender->nRecepients && temp != NULL; i++){
      if(temp->pTarget == pReciever){
        return 1;
      }
      if(temp != NULL){
        temp = temp->pNext;
      }
    }
  }

  return 0;
}

/* Bind reciever to the transmitter */
inline void bindReciever(RECIEVER *pReciever,SENDER *pSender){
  RECIEVERS_LLIST *temp = NULL;

  if(!pReciever || !pSender){
    (void)puts("Error, can't bind with NULL.");
    return;
  }

  temp = pSender->pRecepients;

  if(temp != NULL){
    while(temp->pNext != NULL){
      temp = temp->pNext;
    }
    temp->pNext = calloc(1, sizeof(RECIEVERS_LLIST));
    (temp->pNext->pTarget) = pReciever;
    pSender->nRecepients++;
  }else{
    pSender->pRecepients = calloc(1, sizeof(RECIEVERS_LLIST));
    pSender->pRecepients->pTarget = pReciever;
    pSender->nRecepients = 1;
  }
}

/* Gaussian distribution generator, Box-Muller method */
inline float genGauss(void){
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

/* Calculate distance with Pythagoras theorem */
inline float distance_euclid(const RECIEVER *pRecvr,const SENDER *pSender){
  float dist = sqrt((pRecvr->x - pSender->x)*(pRecvr->x - pSender->x) + (pRecvr->y - pSender->y)*(pRecvr->y - pSender->y));
  if(dist < 1.0){
    dist = 1.0;
  }
  return dist;
}

/* Standart model power calculation. */
inline float power_simple(RECIEVER *pRecvr, const SENDER *pSender, const float amp){
  return ((isUseful(pRecvr, pSender)) ? (1) : (-1)) * pSender->power * (100.0)/(distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender) + amp);
}

/* Slightly more realistic model power calculation */
inline float power_complex(RECIEVER *pRecvr, const SENDER *pSender, const float amp, const float Ht, const float Hr, const float freq){
  return (distance_euclid(pRecvr, pSender) == 0) ? (0) : (isUseful(pRecvr, pSender) ? (1) : (-1))*(20.0*log10(4.0*M_PI/(3e8/freq)) - 2.0 * Hr + 40.0*log10(distance_euclid(pRecvr, pSender)));
}

#else

/* Check whether we transmit data to the pReciever or not */
char isUseful(const RECIEVER *pReciever, const SENDER *pSender){
  unsigned int i = 0;
  RECIEVERS_LLIST *temp = NULL;

  if(!pReciever || !pSender){
    (void)puts("Error, recieved NULL in isUseful.");
    return -1;
  }

  temp = pSender->pRecepients;

  if(temp != NULL){
    for(i = 0; i < pSender->nRecepients && temp != NULL; i++){
      if(temp->pTarget == pReciever){
        return 1;
      }
      if(temp != NULL){
        temp = temp->pNext;
      }
    }
  }

  return 0;
}

/* Bind reciever to the transmitter */
void bindReciever(RECIEVER *pReciever, SENDER *pSender){
  RECIEVERS_LLIST *temp = NULL;

  if(!pReciever || !pSender){
    (void)puts("Error, can't bind with NULL.");
    return;
  }

  temp = pSender->pRecepients;

  if(temp != NULL){
    while(temp->pNext != NULL){
      temp = temp->pNext;
    }
    temp->pNext = calloc(1, sizeof(RECIEVERS_LLIST));
    temp->pNext->pTarget = pReciever;
    (pSender->nRecepients)++;
  }else{
    pSender->pRecepients = calloc(1, sizeof(RECIEVERS_LLIST));
    pSender->pRecepients->pTarget = pReciever;
    (pSender->nRecepients) = 1;
  }
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

/* Calculate distance with Pythagoras theorem */
float distance_euclid(const RECIEVER *pRecvr,const SENDER *pSender){
  float dist = sqrt((pRecvr->x - pSender->x)*(pRecvr->x - pSender->x) + (pRecvr->y - pSender->y)*(pRecvr->y - pSender->y));
  if(dist < 1.0){
    dist = 1.0;
  }
  return dist;
}

/* Standart model power calculation. */
float power_simple(RECIEVER *pRecvr, const SENDER *pSender, const float amp){
  return ((isUseful(pRecvr, pSender)) ? (1) : (-1)) * pSender->power * (100.0)/(distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender) + amp);
}

/* Slightly more realistic model power calculation */
float power_complex(RECIEVER *pRecvr, const SENDER *pSender, const float amp, const float Ht, const float Hr, const float freq){
  return (distance_euclid(pRecvr, pSender) == 0) ? (0) : (isUseful(pRecvr, pSender) ? (1) : (-1))*(20.0*log10(4.0*M_PI/(3e8/freq)) - 2.0 * Hr + 40.0*log10(distance_euclid(pRecvr, pSender)));
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
      switch(task->model){
        case(1):{
          buffer = power_simple((task->pRecvrs + i), (task->pSenders + j), *(gA + (unsigned int)floor((task->pRecvrs + i)->x) + (unsigned int)(task->W*floor((task->pRecvrs + i)->y))));
          break;
	}
	case(2):{
	  buffer = power_complex((task->pRecvrs + i), (task->pSenders + j), *(gA + (unsigned int)floor((task->pRecvrs + i)->x) + (unsigned int)(task->W*floor((task->pRecvrs + i)->y))), 1.5, 1.5, 2.4e9);
	  break;
	}
	default:{
	  (void)puts("Error, mode not suppoted.");
	  pthread_exit(NULL); 
	}
      }
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

/* Unreal mode */
float *prepareSilencing(unsigned int W, unsigned int H){
  unsigned int i = 0, j = 0;

  gA = calloc(W*H, sizeof(float));

  gAsize = W*H;

  for(j = 0; j < H; j++){
    for(i = 0; i < W; i++){
      *(gA + i + j*W) = genGauss()*6.0;
    }
  }

  return gA;
}

/* Calculate total power applied to every reciever */
void calcPower(RECIEVER *pRecvrs, SENDER *pSenders, const unsigned int nSends, const unsigned int nRecvs, unsigned int W, int Threads, unsigned int model){
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

  gpSenders = pSenders;
  gnSenders = nSends;

  if(nThreads <= 0){
    for(j = 0; j < nRecvs; j++){
      for(i = 0; i < nSends; i++){
        switch(model){
          case(1):{
            buffer = power_simple(pRecvrs + j, pSenders + i, *(gA + (unsigned int)floor((pRecvrs + j)->x) + (unsigned int)(W*floor((pRecvrs + j)->y))));
            break;
	  }
	  case(2):{
	    buffer = power_complex(pRecvrs + j, pSenders + i, *(gA + (unsigned int)floor((pRecvrs + j)->x) + (unsigned int)(W*floor((pRecvrs + j)->y))), 1.5, 1.5, 2.4e9);
	    break;
	  }
	  default:{
	    (void)puts("Error, mode not suppoted.");
  	    return;
  	  }
        }
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
      (pTParams + i)->model = model;
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
        switch(model){
          case(1):{
            buffer = power_simple(((pTParams + nThreads -1)->pRecvrs + j), ((pTParams + nThreads - 1)->pSenders + i), *(gA + (unsigned int)floor(((pTParams + nThreads - 1)->pRecvrs + j)->x) + (unsigned int)(W*floor(((pTParams + nThreads - 1)->pRecvrs + j)->y))));
            break;
	  }
	  case(2):{
	    buffer = power_complex(((pTParams + nThreads - 1)->pRecvrs + j), ((pTParams + nThreads - 1)->pSenders + i), *(gA + (unsigned int)floor(((pTParams + nThreads - 1)->pRecvrs + j)->x) + (unsigned int)(W*floor(((pTParams + nThreads - 1)->pRecvrs + j)->y))), 1.5, 1.5, 2.4e9);
	    break;
	  }
	  default:{
	    (void)puts("Error, mode not suppoted.");
  	    return;
  	  }
        }
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

/* Create transmitters within given bounds maxW,maxH and bind recievers to them */
void spawnTransmitters(SENDER *pSenders, RECIEVER *pRecievers, const unsigned int nSenders, const unsigned int nRecievers, const unsigned int maxW, const unsigned int maxH){
  unsigned int i = 0, j = 0;

  if(!pRecievers || !pSenders || nRecievers == 0 || nSenders == 0 || maxW == 0 || maxH == 0){
    (void)puts("Error, can't spawn transmitters.");
    return;
  }

  for(i = 0; i < nSenders; i++){
    (pSenders + i)->x = ((float)rand()/(float)RAND_MAX)*maxW;
    (pSenders + i)->y = ((float)rand()/(float)RAND_MAX)*maxH;
    (pSenders + i)->power = 1.0;
  }

  for(i = 0; i < nRecievers; i++){
    j = rand()%nSenders;
    bindReciever(pRecievers + i, pSenders + j);
  }

}

void stopModel(void){
  unsigned int i = 0, j = 0;
  RECIEVERS_LLIST *temp = NULL, *temp2 = NULL;

  if(!gA && !gpSenders){
    (void)puts("Error, failed to stop model, not running.");
    return;
  }

  for(i = 0; i < gnSenders; i++){
    temp = (gpSenders + i)->pRecepients;
    for(j = 0; j < (gpSenders + i)->nRecepients && temp != NULL; j++){
      temp2 = temp->pNext;
      (void)free(temp);
      temp = temp2;
    }
  }
}
