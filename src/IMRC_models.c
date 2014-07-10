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
#include "IMRC_ploss_models.h"
#define NOT_MAIN
#include "IMRC_types.h"

/* ALSO: Remember that distance_euclid returns distance in meters. */
extern float percentY, percentX, *modRecievers, probDieNow, probSpawnNow, *gA, maxWidthNow, maxHeightNow;
extern unsigned int nRecieversNow, nSendersNow, useGraph, randSeed;
extern unsigned char modelNow, sendersChanged, runningNow, bindMode;
extern unsigned char nThreadsNow, verification;
extern RECIEVER *pRecieversNow;
extern SENDER *pSendersNow;

typedef struct{
  unsigned int steps;
  unsigned int W;
  unsigned int model;
  unsigned int threadNum;
}THREAD_PARAMS;

/* Global model parameters */
unsigned char startedListen = 0, *pThreadExited = NULL, *pThreadSigRcv0 = NULL, *pThreadSigRcv1 = NULL, *pThreadSigRcv2 = NULL;
unsigned int shutdown = 0, firstRun = 1;/* Running for the first time, should we stop now?  */

pthread_mutex_t *pMainReadyMutexes = NULL;
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

    pMainReadyMutexes = calloc(nThrds, sizeof(pthread_mutex_t));
    
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
    (void)pthread_mutex_unlock(pMainReadyMutexes + i);
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
    (void)pthread_mutex_init(pMainReadyMutexes + i, NULL);
    (void)pthread_mutex_lock(pMainReadyMutexes + i);
    (pParams + i)->threadNum = i;
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

  (void)free(pThreadExited);
  (void)free(pMainReadyMutexes);

  (void)free(pThreads);
  (void)free(pParams);
  nThreadsNow = 0;

  printd("All threads freed successfully", __FILE__, __LINE__);
}

#ifndef DEBUG
inline
#endif
void waitForThreads(){
  unsigned int i = 0;

  printd("Waiting for threads", __FILE__, __LINE__);

  /* Inform threads that we are ready to wait. */
  for(i = 0; i < nThreadsNow; i++){
    (void)pthread_mutex_unlock(pMainReadyMutexes + i);
  }

  for(i = 0; i < nThreadsNow; i++){
    (void)pthread_mutex_lock(pMainReadyMutexes + i);
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

    printdt("Thread waiting for start signal", __FILE__, __LINE__, task->threadNum);
    (void)pthread_mutex_lock(pMainReadyMutexes + task->threadNum);

    printdt("Thread recieved start signal", __FILE__, __LINE__, task->threadNum);
    pReciever = rcvrAtIndex(task->threadNum);
    for(i = 0; i < task->steps && pReciever != NULL; i++){
      pReciever = rcvrAtIndex(task->threadNum + i * (task->threadNum + 1));
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
    (void)pthread_mutex_unlock(pMainReadyMutexes + task->threadNum);
    
    printdt("Waiting for parent to finish it's job", __FILE__, __LINE__, task->threadNum);
    (void)pthread_mutex_lock(pMainReadyMutexes + task->threadNum);

    printdt("Parent finished, continuing", __FILE__, __LINE__, task->threadNum);
  }

  *(pThreadExited + task->threadNum) = 1;
  (void)pthread_exit(NULL);
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

    ++step;
  }
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
