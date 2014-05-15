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
#include "IMRC_pretty_output.h"
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
  unsigned int steps;
  unsigned int W;
  unsigned int model;
  unsigned int threadNum;
}THREAD_PARAMS;

/* Global model parameters */
unsigned char *pThreadBeginCheck = NULL, *pThreadFinishedCheck = NULL, *pMainReadyCheck = NULL, startedListen = 0, *pThreadExited = NULL;
unsigned int shutdown = 0, firstRun = 1;/* Running for the first time, should we stop now?  */
pthread_mutex_t pStartThread = PTHREAD_MUTEX_INITIALIZER; /* Mutex for starting thread. */

pthread_mutex_t *pThreadBeginMutexes = NULL;
pthread_mutex_t *pThreadFinishedMutexes = NULL;
pthread_mutex_t *pMainReadyMutexes = NULL;

pthread_cond_t *pThreadBeginConditions = NULL; /* Condition variable for starting child threads. */
pthread_cond_t *pThreadFinishedConditions = NULL; /* Condition for thread to continue. */
pthread_cond_t *pMainReadyConditions = NULL; /* Condition if thread finished calculations, used by parent. */

pthread_t *pThreads = NULL;

THREAD_PARAMS *pParams = NULL; /* Thread parameters, change drastically during run. */

/* Declare the function before actual use to prevent blahblah undeclared errors */
void *threadPowerCalc(void *args);

/* Prepare threads for further use and keep them up all the time */
#ifndef DEBUG
inline
#endif
void initThreads(unsigned int nThrds){
  printd("Trying to create threads", __FILE__, __LINE__);
  firstRun = 1;
  shutdown = 0;
  if(nThreadsNow == 0){
    pThreads = calloc(nThrds, sizeof(pthread_t));
    pParams = calloc(nThrds, sizeof(THREAD_PARAMS));

    pThreadBeginConditions = calloc(nThrds, sizeof(pthread_cond_t));
    pThreadFinishedConditions = calloc(nThrds, sizeof(pthread_cond_t));
    pMainReadyConditions = calloc(nThrds, sizeof(pthread_cond_t));

    pThreadBeginMutexes = calloc(nThrds, sizeof(pthread_mutex_t));
    pThreadFinishedMutexes = calloc(nThrds, sizeof(pthread_mutex_t));
    pMainReadyMutexes = calloc(nThrds, sizeof(pthread_mutex_t));
    
    pThreadBeginCheck = calloc(nThrds, sizeof(unsigned char));
    pThreadFinishedCheck = calloc(nThrds, sizeof(unsigned char));
    pMainReadyCheck = calloc(nThrds, sizeof(unsigned char));
    pThreadExited = calloc(nThrds, sizeof(unsigned char));

    nThreadsNow = nThrds;
  }else{
    printw("Not creating any new threads, delete old first", __FILE__, __LINE__);
  }
}

/* Continue all threads, initially they will be locked. */
#ifndef DEBUG
inline
#endif
void contThreads(void){
  unsigned int i = 0;
  printd("Continuing threads", __FILE__, __LINE__);
  for(i = 0; i < nThreadsNow; i++){
    (void)pthread_cond_signal(pMainReadyConditions + i);
    *(pMainReadyCheck + i) = 1;
  }
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
    printe("Initialize threads first, then start them", __FILE__, __LINE__);
    return;
  }

  for(i = 0; i < nThreadsNow; i++){
    (void)pthread_cond_init(pThreadFinishedConditions + i, NULL);
    (void)pthread_cond_init(pThreadBeginConditions + i, NULL);
    (void)pthread_cond_init(pMainReadyConditions + i, NULL);
    (void)pthread_mutex_init(pThreadFinishedMutexes + i, NULL);
    (void)pthread_mutex_init(pThreadBeginMutexes + i, NULL);
    (void)pthread_mutex_init(pMainReadyMutexes + i, NULL);
    (pParams + i)->threadNum = i;
    *(pThreadFinishedCheck + i) = 0;
    *(pThreadBeginCheck + i) = 0;
    *(pMainReadyCheck + i) = 0;
    *(pThreadExited + i) = 0;
    if(pthread_create(pThreads + i, NULL, threadPowerCalc, pParams + i) != 0){
      printw("Failed to create thread, retrying", __FILE__, __LINE__);
      i--;
    }
  }
  printd("Started threads.", __FILE__, __LINE__);
}

/* Load thread parameters in. */
#ifndef DEBUG
inline
#endif
void loadParams(THREAD_PARAMS *pParms, unsigned char threadNumber){
  if(!pParms){
    printe("Recieved NULL in loadParams.", __FILE__, __LINE__);
    return;
  }

  if(threadNumber >= nThreadsNow || threadNumber < 0){
    printe("Invalid thread index provided.", __FILE__, __LINE__);
    return;
  }

  (pParams + threadNumber)->W = pParms->W;
  (pParams + threadNumber)->steps = pParms->steps;
  (pParams + threadNumber)->model = pParms->model;
}

/* Free threads and parameters */
#ifndef DEBUG
inline
#endif
void freeThreads(void){
  unsigned int i = 0;

  if(pThreads == NULL || nThreadsNow == 0){
    printe("There are no threads to free.", __FILE__, __LINE__);
    return;
  }

  shutdown = 1;

  (void)contThreads();

  for(i = 0; i < nThreadsNow; i++){
    if(*(pThreadExited + i)){
      (void)pthread_join(*(pThreads + i), NULL);
    }
  }

  (void)free(pThreadFinishedCheck);
  (void)free(pThreadBeginCheck);
  (void)free(pMainReadyCheck);
  (void)free(pThreadExited);

  (void)free(pThreadFinishedMutexes);
  (void)free(pThreadBeginMutexes);
  (void)free(pMainReadyMutexes);
  
  (void)free(pThreadFinishedConditions);
  (void)free(pThreadBeginConditions);
  (void)free(pMainReadyConditions);

  (void)free(pThreads);
  (void)free(pParams);
  nThreadsNow = 0;

  printd("All threads freed successfully", __FILE__, __LINE__);
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

#ifndef DEBUG
inline
#endif
void waitForThreads(){
  unsigned int i = 0;

  printd("Waiting for threads", __FILE__, __LINE__);

  /* Inform threads that we are ready to wait. */
  for(i = 0; i < nThreadsNow; i++){
    (void)pthread_cond_signal(pThreadBeginConditions + i);
    *(pThreadBeginCheck + i) = 1;
  }

  for(i = 0; i < nThreadsNow; i++){
    if(*(pThreadFinishedCheck + i) == 0){
      (void)pthread_mutex_lock(pThreadFinishedMutexes + i);
      (void)pthread_cond_wait(pThreadFinishedConditions + i, pThreadFinishedMutexes + i);
      (void)pthread_mutex_unlock(pThreadFinishedMutexes + i);
    }

    *(pThreadFinishedCheck + i) = 0;

    printd("Parent thread notified", __FILE__, __LINE__);
  }
  printd("Threads are done, continuing", __FILE__, __LINE__);
}

/* Check whether the user is within the sector */
#ifndef DEBUG
inline
#endif
char isInView(SENDER *pOrigin, float startDegs, float stopDegs, RECIEVER *pUser){
  float angle = 0.0;

  if(!pOrigin || !pUser){
    printe("Recieved NULL in isInView.", __FILE__, __LINE__);
    return -1;
  }

  angle = atan((pOrigin->x - pUser->x)/(pOrigin->y - pUser->y));

  if(angle > startDegs && angle < stopDegs){
    printe("Location in sector", __FILE__, __LINE__);
    return 1;
  }

  printd("Location out of sector", __FILE__, __LINE__);

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
    printet("NULL passed to thread", __FILE__, __LINE__, task->threadNum);
    (void)pthread_exit(NULL);
  }

  task = (THREAD_PARAMS *)args;

  while(shutdown == 0){

    if(*(pThreadBeginCheck + task->threadNum) == 0){
     printdt("Thread waiting for start signal", __FILE__, __LINE__, task->threadNum);
     (void)pthread_mutex_lock(pThreadBeginMutexes + task->threadNum);
     (void)pthread_cond_wait(pThreadBeginConditions + task->threadNum, pThreadBeginMutexes + task->threadNum);
     (void)pthread_mutex_unlock(pThreadBeginMutexes + task->threadNum);
    }

    *(pThreadBeginCheck + task->threadNum) = 0;

    printdt("Thread recieved start signal", __FILE__, __LINE__, task->threadNum);
    pReciever = rcvrAtIndex(task->threadNum);
    for(i = 0; i < task->steps && pReciever != NULL; i++){
      pReciever = rcvrAtIndex(task->threadNum + i * task->threadNum);
      if(pReciever->recalc || sendersChanged){
        pSender = pSendersNow;
        for(j = 0; j < nSendersNow; j++){
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
	      printet("Mode not suppoted", __FILE__, __LINE__, task->threadNum);
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
    }

    printdt("Thread finished calculations, notifying parent", __FILE__, __LINE__, task->threadNum);
    (void)pthread_cond_signal(pThreadFinishedConditions + task->threadNum);
    *(pThreadFinishedCheck + task->threadNum) = 1;
    
    if(*(pMainReadyCheck + task->threadNum) == 0){
      printdt("Waiting for parent to finish it's job", __FILE__, __LINE__, task->threadNum);
      (void)pthread_mutex_lock(pMainReadyMutexes + task->threadNum);
      (void)pthread_cond_wait(pMainReadyConditions + task->threadNum, pMainReadyMutexes + task->threadNum);
      (void)pthread_mutex_unlock(pMainReadyMutexes + task->threadNum);
    }

    *(pMainReadyCheck + task->threadNum) = 0;

    printdt("Parent finished, continuing", __FILE__, __LINE__, task->threadNum);
  }

  *(pThreadExited + task->threadNum) = 1;
  (void)pthread_exit(NULL);
}

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

/* Calculate total power applied to every reciever */
#ifndef DEBUG
inline
#endif
void calcPower(void){
  unsigned int i = 0, accumBuffer = 1;
  THREAD_PARAMS threadParameters;

  if(!pRecieversNow || !pSendersNow){
    printe("NULL provided", __FILE__, __LINE__);
    return;
  }

  if(modRecievers != NULL){
    (void)free(modRecievers);
  }

  modRecievers = calloc(nRecieversNow, sizeof(float));

  /* Prepare tasks for threads */
  for(i = 0; i < nThreadsNow; i++){
    threadParameters.steps = (i < nThreadsNow - 1) ? (round(nRecieversNow/nThreadsNow)) : (nRecieversNow - accumBuffer);
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
    printe("Can't initialize model with invalid parameters", __FILE__, __LINE__);
    return;
  }

  initStart();

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

  printd("Model initialised", __FILE__, __LINE__);
}

/* Model loop */
void modelLoop(FILE *O, unsigned int steps, unsigned int nanoDelay){
  unsigned int step = 0, i = 0, nDeleted = 0;
  float Pr = 0.0, temp = 0.0;
  struct timespec tSpec;

  tSpec.tv_sec = 0;
  tSpec.tv_nsec = nanoDelay;

  if(!pRecieversNow){
    printe("Got NULL in modelLoop", __FILE__, __LINE__);
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
/*
#ifdef DEBUG
      (void)printf("DEBUG: Spawned %d new recievers, there are now %d.\n", i, nRecieversNow);
      Mn += i;
      Md += nDeleted;
#endif
*/
    }

    calcPower();
    (void)printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$[STEP: %d]\n", step);
    (void)fflush(stdout);
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
/* 
#ifdef DEBUG
(void)printf("Mn = %f, P = %f\n", (float)Mn/(float)steps, probSpawnNow);
  (void)printf("Md = %f, P = %f\n", (float)Md/(float)steps, probDieNow);
  (void)fclose(Od);
#endif*/
  stopVerr();
}

/* Create recievers within given bounds maxW, maxH */
void spawnRecievers( const unsigned int maxW, const unsigned int maxH){
  RECIEVER *pTempR = pRecieversNow;

  if(!pRecieversNow || nRecieversNow == 0 || maxW == 0 || maxH == 0){
    printe("Can't spawn recievers", __FILE__, __LINE__);
    return;
  }

  for(; pTempR; pTempR = pTempR->pNext){
    pTempR->x = ((float)rand()/(float)RAND_MAX)*maxW;
    pTempR->y = ((float)rand()/(float)RAND_MAX)*maxH;
  }

  printd("Spawned recievers", __FILE__, __LINE__);
}

/* Create transmitters within given bounds maxW,maxH and bind recievers to them */
void spawnTransmitters( const unsigned int maxW, const unsigned int maxH){
  unsigned int j = 0;
  SENDER *pTempS = pSendersNow;
  RECIEVER *pTempR = pRecieversNow;

  if(!pRecieversNow || !pSendersNow || nRecieversNow == 0 || nSendersNow == 0 || maxW == 0 || maxH == 0){
    printe("Can't spawn transmitters", __FILE__, __LINE__);
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

  printd("Spawned transmitters", __FILE__, __LINE__);
}

/* Free allocated memory */
void stopModel(void){
  RECIEVERS_LLIST *pTempLL = NULL;
  SENDER *pTempS = pSendersNow;

  if(!gA || !pTempS || !pRecieversNow){
    printe("Failed to stop model, not running", __FILE__, __LINE__);
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
  printd("Stopped model", __FILE__, __LINE__);
}
