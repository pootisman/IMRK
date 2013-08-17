#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include "IMRC_models.h"
#include "IMRC_aux.h"
#include "IMRC_gl.h"

#define NOT_MAIN
#include "IMRC_types.h"

#define M_PI 3.14159265358979323846264338327

extern float *gA, percentY, percentX, *modRecievers, maxWidthNow, maxHeightNow, probDieNow, probSpawnNow;
extern unsigned int nRecieversNow, nSendersNow, gASize, useGraph;
extern unsigned char lineWidth, spotSize, modelNow, nThreadsNow, sendersChanged;
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

/* Function for threaded calculations */
void *threadPowerCalc(void *args){
  THREAD_PARAMS *task = (THREAD_PARAMS *)args;
  unsigned int i = 0, j = 0;
  float buffer = 0.0;
  RECIEVER *pReciever = NULL;
  SENDER *pSender = NULL;

  if(!task){
    (void)puts("Error, NULL passed to thread.");
    (void)pthread_exit(NULL);
  }

  pReciever = task->pRecvrs;

  for(i = 0; i < task->steps; i++){
    if(pReciever->recalc || sendersChanged){
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
      pReciever->recalc = 0;
    }
    pReciever = pReciever->pNext;
  }

  (void)pthread_exit(NULL);
}

/* Unreal mode */
inline float *prepareSilencing(unsigned int W, unsigned int H){
  unsigned int i = 0, j = 0;

  if(!W || !H){
    (void)puts("Error, can't initialize 0x0 field.");
    return NULL;
  }

  maxWidthNow = W;
  maxHeightNow = H;

  gA = calloc(W*H, sizeof(float));

  gASize = W*H;

  for(j = 0; j < H; j++){
    for(i = 0; i < W; i++){
      *(gA + i + j*W) = genGauss()*6.0;
    }
  }

  return gA;
}

/* Calculate total power applied to every reciever */
inline void calcPower(void){
  unsigned int i = 0;
  long int nThreads = 0;
  float buffer = 0.0;
  pthread_t *pThreads = NULL;
  pthread_attr_t threadAttr;
  THREAD_PARAMS *pTParams = NULL;
  RECIEVER *pTempR = pRecieversNow;
  SENDER *pTempS = pSendersNow;

  if(!pRecieversNow || !pSendersNow){
    (void)puts("Error, NULL provided!");
    return;
  }

  nThreads = sysconf(_SC_NPROCESSORS_ONLN);

  if(nThreadsNow != 0){
    nThreads = nThreadsNow;
  }

  if(nThreads <= 0){
    (void)puts("No multi-threading will be used.");
  }

  modRecievers = calloc(nRecieversNow, sizeof(float));

  if(nThreads <= 0){
    for(; pTempR; pTempR = pTempR->pNext ){
      if(pTempR->recalc || sendersChanged){
        for(; pTempS; pTempS = pTempS->pNext){
          switch(modelNow){
            case(1):{
              buffer = power_simple(pTempR, pTempS, *(gA + (unsigned int)floor(pTempR->x) + (unsigned int)(maxWidthNow*floor(pTempR->y))));
              break;
	    }
	    case(2):{
	      buffer = power_complex(pTempR, pTempS, *(gA + (unsigned int)floor(pTempR->x) + (unsigned int)(maxWidthNow*floor(pTempR->y))), 1.5, 1.5, 2.4e9);
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

	  pTempS = pTempS->pNext;
        }
       
	pTempR->SNRLin = pTempR->signal/pTempR->waste;
	pTempR->recalc = 0;
      }
      pTempR = pTempR->pNext;
    }

    for(i = 0; i < nRecieversNow; i++){
      *(modRecievers + i) = (float)rand()/(float)RAND_MAX;
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
      (pTParams + i)->model = modelNow;
      (pTParams + i)->W = maxWidthNow;
    }
    /* Start all threads */
    for(i = 0; i < nThreads; i++){
      if(pthread_create((pThreads + i), &(threadAttr), threadPowerCalc, (void *)(pTParams + i))){
        (void)puts("Error while creating thread, retrying.");
	--i;
      }
    }

    for(i = 0; i < nRecieversNow; i++){
      *(modRecievers + i) = (float)rand()/(float)RAND_MAX;
    }

    for(i = 0; i < nThreads; i++){
      pthread_join(*(pThreads + i), NULL);
    }

    (void)free(pTParams);
    (void)free(pThreads);
  }
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
      (void)puts("DEBUG: The transmitter is useful.");
      return 1;
    }
  }
  (void)puts("DEBUG: The transmiter is useless.");
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

  return sqrt(-1.0f * log(S) / S) * V1;
}

/* Calculate distance with Pythagoras theorem */
float distance_euclid(const RECIEVER *pRecvr,const SENDER *pSender){
  float dist = sqrt((pRecvr->x - pSender->x)*(pRecvr->x - pSender->x) + (pRecvr->y - pSender->y)*(pRecvr->y - pSender->y));
  if(dist < 1.0){
    dist = 1.0;
  }

  (void)puts("DEBUG: Successfully calculated distance.");

  return dist;
}

/* Standart model power calculation. */
float power_simple(RECIEVER *pRecvr, const SENDER *pSender, const float amp){
  return ((isUseful(pRecvr, pSender)) ? (1) : (-1)) * pSender->power * (100.0)/(distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender)*distance_euclid(pRecvr, pSender) + amp);

  (void)puts("DEBUG: Successfully calculated signal power using simple model.");
}

/* Slightly more realistic model power calculation */
float power_complex(RECIEVER *pRecvr, const SENDER *pSender, const float amp, const float Ht, const float Hr, const float freq){
  return (distance_euclid(pRecvr, pSender) == 0) ? (0) : (isUseful(pRecvr, pSender) ? (1) : (-1))*(20.0*log10(4.0*M_PI/(3e8/freq)) - 2.0 * Hr + 40.0*log10(distance_euclid(pRecvr, pSender)));

  (void)puts("DEBUG: Successfully calculated signal power using complex model.");
}

/* Function for threaded calculations */
void *threadPowerCalc(void *args){
  THREAD_PARAMS *task = (THREAD_PARAMS *)args;
  unsigned int i = 0, j = 0;
  float buffer = 0.0;
  RECIEVER *pReciever = NULL;
  SENDER *pSender = NULL;

  if(!task){
    (void)puts("Error, NULL passed to thread.");
    (void)pthread_exit(NULL);
  }

  (void)puts("DEBUG: Started the thread.");

  pReciever = task->pRecvrs;

  for(i = 0; i < task->steps; i++){
    if(pReciever->recalc || sendersChanged){
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
      pReciever->recalc = 0;
    }
    pReciever = pReciever->pNext;
  }

  (void)puts("DEBUG: Thread finished.");

  (void)pthread_exit(NULL);
}

/* Unreal mode */
float *prepareSilencing(unsigned int W, unsigned int H){
  unsigned int i = 0, j = 0;

  if(!W || !H){
    (void)puts("Error, can't initialize 0x0 field.");
    return NULL;
  }

  gA = calloc(W*H, sizeof(float));

  maxWidthNow = W;
  maxHeightNow = H;

  gASize = W*H;

  for(j = 0; j < H; j++){
    for(i = 0; i < W; i++){
      *(gA + i + j*W) = genGauss()*6.0;
    }
  }
  (void)puts("DEBUG: Silencing matrice ready.");
  return gA;
}

/* Calculate total power applied to every reciever */
void calcPower(void){
  unsigned int i = 0;
  long int nThreads = 0;
  float buffer = 0.0;
  pthread_t *pThreads = NULL;
  pthread_attr_t threadAttr;
  THREAD_PARAMS *pTParams = NULL;
  RECIEVER *pTempR = pRecieversNow;
  SENDER *pTempS = pSendersNow;

  (void)puts("DEBUG: Started power calculation.");

  if(!pRecieversNow || !pSendersNow){
    (void)puts("Error, NULL provided!");
    return;
  }

  nThreads = sysconf(_SC_NPROCESSORS_ONLN);

  if(nThreadsNow != 0){
    nThreads = nThreadsNow;
  }

  if(nThreads <= 0){
    (void)puts("No multi-threading will be used.");
  }

  modRecievers = calloc(nRecieversNow, sizeof(float));

  if(!modRecievers){
    (void)puts("DEBUG: Error, failed to allocate memory for modRecievers.");
  }

  if(nThreads <= 0){
    for(; pTempR; pTempR = pTempR->pNext ){
      if(pTempR->recalc || sendersChanged){
        for(; pTempS; pTempS = pTempS->pNext){
          switch(modelNow){
            case(1):{
              buffer = power_simple(pTempR, pTempS, *(gA + (unsigned int)floor(pTempR->x) + (unsigned int)(maxWidthNow*floor(pTempR->y))));
              break;
	    }
	    case(2):{
	      buffer = power_complex(pTempR, pTempS, *(gA + (unsigned int)floor(pTempR->x) + (unsigned int)(maxWidthNow*floor(pTempR->y))), 1.5, 1.5, 2.4e9);
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
	pTempR->recalc = 0;
      }
      pTempS = pTempS->pNext;
    }

    for(i = 0; i < nRecieversNow; i++){
      *(modRecievers + i) = (float)rand()/(float)RAND_MAX;
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
      (pTParams + i)->model = modelNow;
      (pTParams + i)->W = maxWidthNow;
    }
    /* Start all threads */
    for(i = 0; i < nThreads; i++){
      if(pthread_create((pThreads + i), &(threadAttr), threadPowerCalc, (void *)(pTParams + i))){
        (void)puts("Error while creating thread, retrying.");
	--i;
      }
    }

    for(i = 0; i < nRecieversNow; i++){
      *(modRecievers + i) = (float)rand()/(float)RAND_MAX;
    }

    for(i = 0; i < nThreads; i++){
      pthread_join(*(pThreads + i), NULL);
    }

    (void)free(pTParams);
    (void)free(pThreads);
  }
  (void)puts("DEBUG: Finished calulating power for one frame.");
}
#endif

/* Initialise model */
void initModel(unsigned int W, unsigned int H, unsigned int model, unsigned int nRecievers, unsigned int nSenders, unsigned int nThreads, FILE *I, unsigned int useGL, float probSpawn, float probDie){
  
  if(!W || !H || !model || !nRecievers || !nSenders){
    (void)puts("Error, can't initialize model with invalid parameters.");
    return;
  }

  probDieNow = probDie;
  probSpawnNow = probSpawn;

  maxWidthNow = W;
  maxHeightNow = H;

  nThreadsNow = nThreads;

  initRand();

  (void)prepareSilencing(W, H);

  if(!I){
    (void)makeRcvrList(nRecievers);
    (void)makeSndrList(nSenders);
    spawnRecievers(maxWidthNow, maxHeightNow);
    spawnTransmitters(maxWidthNow, maxHeightNow);
  }else{
    readFromFile(I);
  }

  useGraph = useGL;

  if(useGraph){
    initGraphics();
  }

  modelNow = model;
#ifdef DEBUG
  (void)puts("Model initialised.");
#endif
}

/* Model loop */
void modelLoop(FILE *O, unsigned int steps){
  float genProb = 0.0f, probLim = 0.0f;
  char running = 1;
  unsigned int step = 0, i = 0, nDeleted;

  if(!pRecieversNow){
    (void)puts("Error, got NULL in modelLoop.");
    return; 
  }

  while(step < steps && running){
    nDeleted = 0;
    if(step){
      for(i = 0; i < nRecieversNow; i++){
#ifdef DEBUG
        (void)printf("DEBUG: Reciever %d %s\n", i, (*(modRecievers + i) < probDieNow) ? ("died.") : ("stayed."));
#endif
        if(*(modRecievers + i) < probDieNow){
          rmReciever(rcvrAtIndex(i - nDeleted));
	  ++nDeleted;
        }
      }

      probLim = (float)rand()/(float)RAND_MAX;

      for(i = 0,genProb = probSpawnNow; genProb > probLim; genProb *= genProb, i++){
        addReciever(sndrAtIndex(rand()%nSendersNow), (float)rand()/(float)RAND_MAX*(float)maxWidthNow, (float)rand()/(float)RAND_MAX*(float)maxHeightNow);
      }

#ifdef DEBUG
      (void)printf("DEBUG: Spawned %d new recievers, there are now %d.\n", i, nRecieversNow);
#endif
      (void)free(modRecievers);
    }

    calcPower();
    
    if(O && steps > 0){
      dumpToFile(O, step);
    }
    
    if(useGraph){
      render();
      (void)sleep(1);
    }

    step++;
  }
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

  if(useGraph){
    killWindow();
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

  (void)free(gA);
  (void)freeLists();
  (void)free(modRecievers);
#ifdef DEBUG
  (void)puts("DEBUG: Stopped model.");
#endif
}
