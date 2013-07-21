#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include "IMRC_models.h"
#include "IMRC_types.h"
#include "IMRC_aux.h"

#define M_PI 3.14159265358979323846264338327

extern float *gA, percentY, percentX;
extern unsigned int nRecieversNow, nSendersNow, gASize;
extern unsigned char lineWidth, spotSize;
extern RECIEVER *pRecieversNow;
extern SENDER *pSendersNow;

typedef struct{
  unsigned int nSenders;
  unsigned int steps;
  unsigned int W;
  unsigned int model;
  RECIEVER *pRecvrs;
  SENDER *pSenders;
}THREAD_PARAMS;

#ifndef DEBUG
/* Check whether we transmit data to the pReciever or not */
inline char isUseful(const RECIEVER *pReciever, const SENDER *pSender){
  RECIEVERS_LLIST *pTempR = NULL;

  if(!pReciever || !pSender){
    (void)puts("Error, recieved NULL in isUseful.");
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
  RECIEVERS_LLIST *pTempR = NULL;

  if(!pReciever || !pSender){
    (void)puts("Error, recieved NULL in isUseful.");
    return -1;
  }

  pTempR = pSender->pRecepients;

  for(; pTempR; pTempR = pTempR->pNext){
    if(pTempR->pTarget == pReciever){
#ifdef DEBUG
      (void)puts("DEBUG: The transmitter is useful.");
#endif
      return 1;
    }
  }
#ifdef DEBUG
  (void)puts("DEBUG: The transmiter is useless.");
#endif
  return 0;
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

#ifdef DEBUG
  (void)puts("DEBUG: Successfully generated gaussian value.");
#endif

  return sqrt(-1.0f * log(S) / S) * V1;
}

/* Calculate distance with Pythagoras theorem */
float distance_euclid(const RECIEVER *pRecvr,const SENDER *pSender){
  float dist = sqrt((pRecvr->x - pSender->x)*(pRecvr->x - pSender->x) + (pRecvr->y - pSender->y)*(pRecvr->y - pSender->y));
  if(dist < 1.0){
    dist = 1.0;
  }

#ifdef DEBUG
  (void)puts("DEBUG: Successfully calculated distance.");
#endif

  return dist;
}

/* Standart model power calculation. */
float power_simple(RECIEVER *pRecvr, const SENDER *pSender, const float amp){
  return ((isUseful(pRecvr, pSender)) ? (1) : (-1)) * pSender->power * (100.0)/(distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender) + amp);

#ifdef DEBUG
  (void)puts("DEBUG: Successfully calculated signal power using simple model.");
#endif
}

/* Slightly more realistic model power calculation */
float power_complex(RECIEVER *pRecvr, const SENDER *pSender, const float amp, const float Ht, const float Hr, const float freq){
  return (distance_euclid(pRecvr, pSender) == 0) ? (0) : (isUseful(pRecvr, pSender) ? (1) : (-1))*(20.0*log10(4.0*M_PI/(3e8/freq)) - 2.0 * Hr + 40.0*log10(distance_euclid(pRecvr, pSender)));

#ifdef DEBUG
  (void)puts("DEBUG: Successfully calculated signal power using complex model.");
#endif
}
#endif
/* Function for threaded calculations */
void *threadPowerCalc(void *args){
  THREAD_PARAMS *task = (THREAD_PARAMS *)args;
  unsigned int i = 0, j = 0;
  float buffer = 0.0;
  RECIEVER *pReciever = NULL;
  SENDER *pSender = NULL;

#ifdef DEBUG
  (void)puts("DEBUG: Thread started.");
#endif

  if(!task){
    (void)puts("Error, NULL passed to thread.");
    (void)pthread_exit(NULL);
  }

  pReciever = task->pRecvrs;

  for(i = 0; i < task->steps; i++){
    pSender = task->pSenders;
    for(j = 0; j < task->nSenders; j++){
      switch(task->model){
        case(1):{
          buffer = power_simple(pReciever, pSender, *(gA + (unsigned int)floor(pReciever->x) + (unsigned int)(task->W*floor(pReciever->y))));
          break;
	}
	case(2):{
	  buffer = power_complex(pReciever, pSender, *(gA + (unsigned int)floor(pReciever->x) + (unsigned int)(task->W*floor(pReciever->y))), 1.5, 1.5, 2.4e9);
	  break;
	}
	default:{
	  (void)puts("Error, mode not suppoted.");
	  pthread_exit(NULL); 
	}
      }
      if(buffer > 0.0){
        pReciever->signal += buffer;
      }else{
	pReciever->waste -= buffer;
      }
      pSender = pSender->pNext;
    }
    pReciever->SNRLin = pReciever->signal/pReciever->waste;
    pReciever = pReciever->pNext;
  }

#ifdef DEBUG
  (void)puts("DEBUG: Thread stopped.");
#endif
  (void)pthread_exit(NULL);
}

/* Unreal mode */
float *prepareSilencing(unsigned int W, unsigned int H){
  unsigned int i = 0, j = 0;

  gA = calloc(W*H, sizeof(float));

  gASize = W*H;

  for(j = 0; j < H; j++){
    for(i = 0; i < W; i++){
      *(gA + i + j*W) = genGauss()*6.0;
    }
  }

#ifdef DEBUG
  (void)puts("DEBUG: Silencing matrice ready.");
#endif
  return gA;
}

/* Calculate total power applied to every reciever */
void calcPower(unsigned int W, int Threads, unsigned int model){
  unsigned int i = 0;
  long int nThreads = 0;
  float buffer = 0.0;
  pthread_t *pThreads = NULL;
  pthread_attr_t threadAttr;
  THREAD_PARAMS *pTParams = NULL;
  RECIEVER *pTempR = pRecieversNow;
  SENDER *pTempS = pSendersNow;


#ifdef DEBUG
  (void)puts("DEBUG: Started power calculation.");
#endif

  if(!pRecieversNow || !pSendersNow){
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
    for(; pTempR; pTempR = pTempR->pNext ){
      for(; pTempS; pTempS = pTempS->pNext){
        switch(model){
          case(1):{
            buffer = power_simple(pTempR, pTempS, *(gA + (unsigned int)floor(pTempR->x) + (unsigned int)(W*floor(pTempR->y))));
            break;
	  }
	  case(2):{
	    buffer = power_complex(pTempR, pTempS, *(gA + (unsigned int)floor(pTempR->x) + (unsigned int)(W*floor(pTempR->y))), 1.5, 1.5, 2.4e9);
	    break;
	  }
	  default:{
	    (void)puts("Error, mode not suppoted.");
  	    return;
  	  }
        }
        if(buffer > 0.0){
          pTempR->signal += buffer;
        }else{
	  pTempR->waste -= buffer;
        }
	pTempR = pTempR->pNext;
      }
      pTempR->SNRLin = pTempR->signal/pTempR->waste;
      pTempS = pTempS->pNext;
    }
  }else{
    pTParams = calloc(nThreads, sizeof(THREAD_PARAMS));
    pThreads = calloc(nThreads, sizeof(pthread_t));
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    /* Prepare tasks for threads */
    for(i = 0; i < nThreads; i++){
      (pTParams + i)->steps = round(nRecieversNow/nThreads);
      (pTParams + i)->pSenders = pSendersNow;
      (pTParams + i)->pRecvrs = rcvrAtIndex(i*(nRecieversNow/nThreads));
      (pTParams + i)->nSenders = nSendersNow;
      (pTParams + i)->model = model;
      (pTParams + i)->W = W;
    }
    /* Start all threads */
    for(i = 0; i < nThreads; i++){
      if(pthread_create((pThreads + i), &(threadAttr), threadPowerCalc, (void *)(pTParams + i))){
        (void)puts("Error while creating thread, retrying.");
	--i;
      }
    }

    for(i = 0; i < nThreads; i++){
      pthread_join(*(pThreads + i), NULL);
    }

    (void)free(pTParams);
    (void)free(pThreads);
  }
#ifdef DEBUG
  (void)puts("DEBUG: Finished calulating power for one frame.");
#endif
}

/* Create recievers within given bounds maxW, maxH */
void spawnRecievers( const unsigned int maxW, const unsigned int maxH){
  RECIEVER *pTempR = pRecieversNow;

  if(!pRecieversNow || nRecieversNow == 0 || maxW == 0 || maxH == 0){
    (void)puts("Error, can't spawn recievers.");
    return;
  }

  for(; pTempR; pTempR = pTempR->pNext){
    pTempR->x = ((float)rand()/(float)RAND_MAX)*maxW;
    pTempR->y = ((float)rand()/(float)RAND_MAX)*maxH;
  }

#ifdef DEBUG
  (void)puts("DEBUG: Spawned recievers.");
#endif
}

/* Create transmitters within given bounds maxW,maxH and bind recievers to them */
void spawnTransmitters( const unsigned int maxW, const unsigned int maxH){
  unsigned int j = 0;
  SENDER *pTempS = pSendersNow;
  RECIEVER *pTempR = pRecieversNow;

  if(!pRecieversNow || !pSendersNow || nRecieversNow == 0 || nSendersNow == 0 || maxW == 0 || maxH == 0){
    (void)puts("Error, can't spawn transmitters.");
    return;
  }

  for(; pTempS; pTempS = pTempS->pNext){
    pTempS->x = ((float)rand()/(float)RAND_MAX)*maxW;
    pTempS->y = ((float)rand()/(float)RAND_MAX)*maxH;
    pTempS->power = 1.0;
  }

  for(; pTempR; pTempR = pTempR->pNext){
    j = rand()%nSendersNow;
    bindToReciever(pTempR, sndrAtIndex(j));
  }

#ifdef DEBUG
  (void)puts("DEBUG: Spawned transmitters.");
#endif
}

/* Free allocated memory */
void stopModel(void){
  RECIEVERS_LLIST *pTempLL = NULL;
  SENDER *pTempS = pSendersNow;

  if(!gA || !pTempS || !pRecieversNow){
    (void)puts("Error, failed to stop model, not running.");
    return;
  }

  for(; pTempS; pTempS = pTempS->pNext){
    if(pTempS->pRecepients){
      pTempLL = pTempS->pRecepients;
      for(; pTempLL->pNext;){
        pTempLL = pTempLL->pNext;
      	(void)free(pTempLL->pPrev);
      }
      (void)free(pTempLL);
    }
  }

  (void)freeLists();

#ifdef DEBUG
  (void)puts("DEBUG: Stopped model.");
#endif
}
