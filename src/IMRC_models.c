#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include "IMRC_models.h"
#include "IMRC_aux.h"
#include "IMRC_gl.h"
#include "IMRC_ver.h"
#define NOT_MAIN
#include "IMRC_types.h"

/* !!!WARNING!!! This is for simplified model of suburban ONLY! */
#define DHM 10.5
#define X 15.0
#define DIST 80.0
#define DHB 25.0

/* ALSO: Remember that distance_euclid returns distance in meters. */
extern float *gA, percentY, percentX, *modRecievers, maxWidthNow, maxHeightNow, probDieNow, probSpawnNow;
extern unsigned int nRecieversNow, nSendersNow, gASize, useGraph, randSeed;
extern unsigned char modelNow, sendersChanged, runningNow, bindMode;
extern unsigned char nThreadsNow, verification;
extern RECIEVER *pRecieversNow;
extern SENDER *pSendersNow;

typedef struct{
#ifdef DEBUG
  float avgSNR;
#endif
  unsigned int nSenders;
  unsigned int steps;
  unsigned int W;
  unsigned int model;
  unsigned int threadNum;
  RECIEVER *pRecvrs;
  SENDER *pSenders;
}THREAD_PARAMS;

/* Global model parameters */
unsigned char *pUnlocked = NULL;
unsigned int shutdown = 0, firstRun = 1;/* Running for the first time, should we stop now?  */
/*pthread_attr_t threadAttr; Thread attributes, will be removed soon */
pthread_cond_t *pThreadConds; /* Condition variables for threads, signalled when they finished */
pthread_cond_t mainCond; /* Main condition variable, all theads will wait on it */
pthread_mutex_t *pThreadMutexes = NULL; /* Mutexes for threads, I dunno */
pthread_mutex_t *pMainMutexes = NULL;
pthread_t *pThreads = NULL; /* Thread pointers */
THREAD_PARAMS *pParams = NULL; /* Thread parameters, change drastically during run */

/* Declare the function before actual use to prevent blahblah undeclared errors */
void *threadPowerCalc(void *args);

/* Prepare threads for further use and keep them up all the time */
#ifndef DEBUG
inline
#endif
void initThreads(unsigned int nThrds){
#ifdef DEBUG
  (void)printf("DEBUG: Trying to create %d threads.\n", nThrds);
#endif
  firstRun = 1;
  shutdown = 0;
  if(nThreadsNow == 0){
    pThreads = calloc(nThrds, sizeof(pthread_t));
    pParams = calloc(nThrds, sizeof(THREAD_PARAMS));
    pThreadConds = calloc(nThrds, sizeof(pthread_cond_t));
    pMainMutexes = calloc(nThrds, sizeof(pthread_mutex_t));
    pThreadMutexes = calloc(nThrds, sizeof(pthread_mutex_t));
    pUnlocked = calloc(nThrds, sizeof(unsigned char));
    nThreadsNow = nThrds;
  }else{
    (void)puts("Not creating any new threads, delete old first!");
  }
}

/* Continue all threads, initially they will be locked. */
#ifndef DEBUG
inline
#endif
void contThreads(void){
  unsigned int i = 0;
  for(;i < nThreadsNow; i++){
    while(*(pUnlocked + i) == 0){
      pthread_cond_broadcast(&mainCond);
      (void)usleep(20);
    }
  }
#ifdef DEBUG
  (void)puts("DEBUG: Continuing threads.");
#endif
}

/* Start all threads, call this ONCE. */
#ifndef DEBUG
inline
#endif
void startThreads(void){
  unsigned char i = 0;

  if(firstRun == 0){
    return;
  }

  firstRun = 0;

  if(nThreadsNow == 0 || pThreads == NULL || pParams == NULL){
    (void)puts("Error, initialize threads first, then start them.");
    return;
  }

  (void)pthread_cond_init(&mainCond, NULL);

  for(;i < nThreadsNow; i++){
    (void)pthread_cond_init(pThreadConds + i, NULL);
    (void)pthread_cond_init(&mainCond, NULL);
    (void)pthread_mutex_init(pThreadMutexes + i, NULL);
    (void)pthread_mutex_init(pMainMutexes + i, NULL);
    (pParams + i)->threadNum = i;
    if(pthread_create(pThreads + i, NULL, threadPowerCalc, pParams + i) != 0){
      (void)puts("Warning, failed to create thread, retrying");
      i--;
    }
  }
#ifdef DEBUG
  (void)puts("DEBUG: Started threads.");
#endif
}

/* Load thread parameters in. */
#ifndef DEBUG
inline
#endif
void loadParams(THREAD_PARAMS *pParms, unsigned char threadNumber){
  if(!pParms){
    (void)puts("Error, recieved NULL in loadParams.");
    return;
  }

  if(!pParms->pSenders || !pParms->pRecvrs){
    (void)puts("Error, recieved NULL in loadParams struct.");
    return;
  }

  if(threadNumber >= nThreadsNow || threadNumber < 0){
    (void)puts("Error, invalid thread index provided.");
    return;
  }

  (pParams + threadNumber)->pSenders = pParms->pSenders;
  (pParams + threadNumber)->pRecvrs = pParms->pRecvrs;
  (pParams + threadNumber)->W = pParms->W;
  (pParams + threadNumber)->steps = pParms->steps;
  (pParams + threadNumber)->model = pParms->model;
  (pParams + threadNumber)->nSenders = pParms->nSenders;
}

/* Free threads and parameters */
#ifndef DEBUG
inline
#endif
void freeThreads(void){
  if(pThreads == NULL || nThreadsNow == 0){
    (void)puts("Error, there are no threads to free.");
    return;
  }

  (void)free(pUnlocked);
  (void)free(pMainMutexes);
  (void)free(pThreadMutexes);
  (void)free(pThreadConds);
  (void)free(pThreads);
  (void)free(pParams);
  nThreadsNow = 0;
#ifdef DEBUG
  (void)puts("DEBUG: All threads freed successfully.");
#endif
}

/* Check whether we transmit data to the pReciever or not */
#ifndef DEBUG
inline
#endif
char isUseful(const RECIEVER *pReciever, const SENDER *pSender){
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

#ifndef DEBUG
inline
#endif
void waitForThreads(){
  unsigned int i = 0;

#ifdef DEBUG
  (void)puts("DEBUG: Waiting for threads.");
#endif

  for(;i < nThreadsNow; i++){
    (void)pthread_mutex_lock(pThreadMutexes + i);
    (void)pthread_cond_wait((pThreadConds + i), (pThreadMutexes + i));
#ifdef DEBUG
    (void)puts("DEBUG: Thread signalled.");
#endif
    *(pUnlocked + i) = 0;
    (void)pthread_mutex_unlock(pThreadMutexes + i);
  }
#ifdef DEBUG
  (void)puts("DEBUG: Threads are done, continuing.");
#endif
}

/* Check whether the user is within the sector */
#ifndef DEBUG
inline
#endif
char isInView(SENDER *pOrigin, float startDegs, float stopDegs, RECIEVER *pUser){
  float angle = 0.0;

  if(!pOrigin || !pUser){
    (void)printf("Error, recieved NULL in isInView.");
    return -1;
  }

  angle = atan((pOrigin->x - pUser->x)/(pOrigin->y - pUser->y));

  if(angle > startDegs && angle < stopDegs){
#ifdef DEBUG
    (void)puts("DEBUG: Location in sector.");
#endif
    return 1;
  }

#ifdef DEBUG
  (void)puts("DEBUG: Location out of sector.");
#endif

  return 0;
}

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

/* Function for threaded calculations */
void *threadPowerCalc(void *args){
  THREAD_PARAMS *task = NULL;
  unsigned int i = 0, j = 0;
  float buffer = 0.0;
  RECIEVER *pReciever = NULL;
  SENDER *pSender = NULL;

  if(args == NULL){
    (void)puts("Error, NULL passed to thread.");
    (void)pthread_exit(NULL);
  }

  task = (THREAD_PARAMS *)args;

  while(shutdown == 0){
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
	      buffer = power_complex(pReciever, pSender, 1.5, 1.5, 2.4e9);
	      break;
	    }
	    case(3):{
	      buffer = power_urban_simple(pReciever, pSender);
	      break;
	    }
	    case(4):{
	      buffer = power_urban_complex(pReciever, pSender);
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
   
        if(task->model != 3 && task->model != 4){
          pReciever->SNRLin = 10.0*log10(pReciever->signal/pReciever->waste);
        }else{
          pReciever->SNRLin = pReciever->signal - pReciever->waste;
        }
        pReciever->recalc = 0;
      }
      pReciever = pReciever->pNext;
    }

    (void)pthread_mutex_lock(pMainMutexes + task->threadNum);
    *(pUnlocked + task->threadNum) = 1;
    do{
      (void)usleep(20);
      (void)pthread_cond_signal(pThreadConds + task->threadNum);
    }while(*(pUnlocked + task->threadNum) == 1);

    (void)pthread_cond_wait(&mainCond, pMainMutexes + task->threadNum);

    *(pUnlocked + task->threadNum) = 1;
#ifdef DEBUG
    (void)puts("Main thread is done, resuming.");
#endif
    (void)pthread_mutex_unlock(pMainMutexes + task->threadNum);
  }

  (void)pthread_exit(NULL);
}

/* Unreal mode */
#ifndef DEBUG
inline
#endif
float *prepareSilencing(unsigned int W, unsigned int H){
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
      *(gA + i + j*W) = genGauss();
    }
  }

  return gA;
}

/* Calculate total power applied to every reciever */
#ifndef DEBUG
inline
#endif
void calcPower(void){
  unsigned int i = 0, accumBuffer = 1;
  THREAD_PARAMS threadParameters;

  if(!pRecieversNow || !pSendersNow){
    (void)puts("Error, NULL provided!");
    return;
  }

  if(modRecievers != NULL){
    (void)free(modRecievers);
  }

  modRecievers = calloc(nRecieversNow, sizeof(float));

  /* Prepare tasks for threads */
  for(; i < nThreadsNow; i++){
    threadParameters.steps = (i < nThreadsNow - 1) ? (round(nRecieversNow/nThreadsNow)) : (nRecieversNow - accumBuffer);
    threadParameters.pSenders = pSendersNow;
    threadParameters.pRecvrs = (i < nThreadsNow - 1) ? (rcvrAtIndex(i*round(nRecieversNow/nThreadsNow))) : (rcvrAtIndex(nRecieversNow - accumBuffer));
    threadParameters.nSenders = nSendersNow;
    threadParameters.model = modelNow;
    threadParameters.W = maxWidthNow;
    loadParams(&threadParameters, i);
    accumBuffer += threadParameters.steps;
  }
  
  /* Start all threads */
  if(firstRun == 1){
    startThreads();
  }else{
    contThreads();
  }

  waitForThreads();

  for(i = 0; i < nRecieversNow; i++){
    *(modRecievers + i) = (float)rand()/(float)RAND_MAX;
  }
}

/* Initialise model */
void initModel(unsigned int W, unsigned int H, unsigned int model, unsigned int nRecievers, unsigned int nSenders, unsigned int nThreads, FILE *I, unsigned int useGL, float probSpawn, float probDie){
  if(!W || !H || !model || !nRecievers || !nSenders){
    (void)puts("Error, can't initialize model with invalid parameters.");
    return;
  }

  modelNow = model;

  probDieNow = probDie;
  probSpawnNow = probSpawn;

  if(nThreads == 0){
    nThreads = sysconf(_SC_NPROCESSORS_ONLN); 
  }

  initThreads(nThreads);

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

#ifdef DEBUG
  (void)puts("Model initialised.");
#endif
}

/* Model loop */
void modelLoop(FILE *O, unsigned int steps, unsigned int nanoDelay){
  unsigned int step = 0, i = 0, nDeleted = 0;
  float Pr = 0.0, temp = 0.0;
  struct timespec tSpec;
#ifdef DEBUG
  float Mn = 0.0, Md = 0.0;
  FILE *Od = fopen("SNR_avg.dat","w");
#endif

  tSpec.tv_sec = 0;
  tSpec.tv_nsec = nanoDelay;

  if(!pRecieversNow){
    (void)puts("Error, got NULL in modelLoop.");
    return; 
  }

  while(step < steps && runningNow){
    nDeleted = 0;
    if(step){
      for(i = 0; i < nRecieversNow; i++){
        if(*(modRecievers + i) < probDieNow){
          rmReciever(rcvrAtIndex(i - nDeleted));
	  ++nDeleted;
        }
      }

      Pr = (float)rand()/(float)RAND_MAX;

      i = 0;
      for(temp = probSpawnNow; temp > Pr; temp *= probSpawnNow, i++){
	if(bindMode == NEAR){
	  addReciever(NULL, (float)rand()/(float)RAND_MAX*(float)maxWidthNow, (float)rand()/(float)RAND_MAX*(float)maxHeightNow);
	}else if(bindMode == RAND){
          addReciever(sndrAtIndex(rand()%nSendersNow), (float)rand()/(float)RAND_MAX*(float)maxWidthNow, (float)rand()/(float)RAND_MAX*(float)maxHeightNow);
	}
      }
#ifdef DEBUG
      (void)printf("DEBUG: Spawned %d new recievers, there are now %d.\n", i, nRecieversNow);
      Mn += i;
      Md += nDeleted;
#endif
    }

    calcPower();

    if(O && steps > 0){
      dumpToFile(O, step);
    }
    
    if(useGraph){
      render();
      (void)nanosleep(&tSpec, NULL);
    }

    if(verification){
      avgSNR();
    }

    ++step;
  }
#ifdef DEBUG
  (void)printf("Mn = %f, P = %f\n", (float)Mn/(float)steps, probSpawnNow);
  (void)printf("Md = %f, P = %f\n", (float)Md/(float)steps, probDieNow);
  (void)fclose(Od);
#endif
  stopVerr();
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
    if(modelNow > 1){
      pTempS->power = 30.0*((float)rand()/(float)RAND_MAX) - 50.0*((float)rand()/(float)RAND_MAX);
    }else{
      pTempS->power = 1;
    }
    pTempS->freq = 2.4e9;
  }

  for(; pTempR; pTempR = pTempR->pNext){
    if(bindMode == NEAR){
      bindToReciever(pTempR, getNearest(pTempR));
    }else if(bindMode == RAND){
      j = rand()%nSendersNow;
      bindToReciever(pTempR, sndrAtIndex(j));
    }else if(bindMode == MAXS){
      for(j = 0; j < nSendersNow; j++){

      }
    }
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
  (void)freeThreads();
  (void)free(modRecievers);
#ifdef DEBUG
  (void)puts("DEBUG: Stopped model.");
#endif
}
