#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define NOT_MAIN
#include "IMRC_types.h"
#include "IMRC_pretty_output.h"
#include "IMRC_aux.h"

extern float *gA, percentY, percentX, maxWidthNow, maxHeightNow;
extern unsigned int nRecieversNow, nSendersNow, gASize, randSeed;
extern unsigned char lineWidth, spotSize;
extern RECIEVER *pRecieversNow;
extern SENDER *pSendersNow;

/* Address of the sender at index */
SENDER *sndrAtIndex(unsigned int index){
  SENDER *pTempS = pSendersNow;
  
  if(pTempS != NULL){
    for(;index > 0; index--){
      if(pTempS != NULL){
        pTempS = pTempS->pNext;
      }else{
        printw("Index out of bounds", __FILE__, __LINE__);
        return NULL;
      }
    }
    return pTempS;
  }else{
    printe("Error, no senders in sndrAtIndex", __FILE__, __LINE__);
    return NULL;
  }
}

/* Address of the reciever at index */
RECIEVER *rcvrAtIndex(unsigned int index){
  RECIEVER *pTempR = pRecieversNow;

  if(pTempR != NULL){
    for(;index > 0; index--){
      if(pTempR != NULL){
        pTempR = pTempR->pNext;
      }else{
        printw("Index out of bounds", __FILE__, __LINE__);
	return NULL;
      }
    }
    return pTempR;
  }else{
    printe("No recievers in rcvrAtIndex", __FILE__, __LINE__);
    return NULL;
  }
}

/* Bind transmitter to specific reciever */
void bindToReciever(RECIEVER *pReciever, SENDER *pSender){
  RECIEVERS_LLIST *pTemp = NULL;

  if(pReciever == NULL || pSender == NULL){
    printe(ENULLPOINT, __FILE__, __LINE__);
    return;
  }

  if(pReciever->pOwner){
    printw("Reciever already occupied", __FILE__, __LINE__);
    return;
  }

  if(!(pSender->pRecepients)){
    pSender->pRecepients = calloc(1, sizeof(RECIEVERS_LLIST));
    pSender->nRecepients = 1;
    pSender->pRecepients->pTarget = pReciever;
  }else{
    pTemp = pSender->pRecepients;
    for(;pTemp->pNext != NULL; pTemp = pTemp->pNext);
    pTemp->pNext = calloc(1, sizeof(RECIEVERS_LLIST));
    pTemp->pNext->pTarget = pReciever;
    pTemp->pNext->pPrev = pTemp;
    pSender->nRecepients++;
  }

  pReciever->recalc = 1;
  pReciever->pOwner = pSender;
}

/* Unbind reciever for transmitter */
void unbindReciever(RECIEVER *pReciever){
  SENDER *pTempS = pSendersNow;
  RECIEVERS_LLIST *pTempR = NULL;
  unsigned int i = 0;

  if(!pTempS){
    printe("Failed to unbind from NULL", __FILE__, __LINE__);
    return;
  }

  for(;pTempS != NULL; pTempS = pTempS->pNext){
    pTempR = pTempS->pRecepients;
    for(i = 0; i < pTempS->nRecepients; i++){
      if(pTempR->pTarget == pReciever){
        
	if(pTempR->pPrev){
          pTempR->pPrev->pNext = pTempR->pNext;
	}
	
	if(pTempR->pNext){
	  pTempR->pNext->pPrev = pTempR->pPrev;
        }
        
        if(pTempR == pTempS->pRecepients){
          pTempS->pRecepients = pTempR->pNext;
	}

	pTempS->nRecepients--;
        (void)free(pTempR);
	return;
      }
      pTempR = pTempR->pNext;
    }
  }
}

/* Write current situation to disk */
void dumpToFile(FILE *output, unsigned int step){
  unsigned int i = 0;
  RECIEVER *pTempR = pRecieversNow;

  if(pRecieversNow == NULL || nRecieversNow == 0 || output == NULL){
    printe("Failed to dump data to file", __FILE__, __LINE__);
    return;
  }

  (void)fprintf(output, "%d\n", step);

  for(;pTempR != NULL; pTempR = pTempR->pNext, i++){
    (void)fprintf(output, "%d\t%f\t%f\t%f\n", i, pTempR->x,  pTempR->y, pTempR->SNRLin);
  }
  printd("Successfully written data to file", __FILE__, __LINE__);
}

/* Read initial data from file */
void readFromFile(FILE *input){
  unsigned int bind = 0, i = 0;
  RECIEVER *pTempR = NULL;
  SENDER *pTempS = NULL;

  if(input == NULL){
    printe("Can't read from file", __FILE__, __LINE__);
    return;
  }

  if(fscanf(input, "%u\n", &nRecieversNow) == 0){
    printe("Can't read reciever data from file", __FILE__, __LINE__);
    return;
  }

  pRecieversNow = pTempR = calloc( 1, sizeof(RECIEVER));

  pTempR->recalc = 1;

  if(fscanf(input, "%f\t%f\n", &(pTempR->x), &(pTempR->y)) == 0){
    printe("Can't read reciever data from file", __FILE__, __LINE__);
    return;
  }

  for(i = 1; i < nRecieversNow; i++){
    pTempR->pNext = calloc(1, sizeof(RECIEVER));
    pTempR->pNext->pPrev = pTempR;
    pTempR = pTempR->pNext;
    pTempR->recalc = 1;
    if(fscanf(input, "%f\t%f\n", &(pTempR->x), &(pTempR->y)) == 0){
      printe("Can't read reciever data from file", __FILE__, __LINE__);
      return;
    }
  }

  if(fscanf(input, "%u\n", &nSendersNow) == 0){
    printe("Can't read sender data from file.", __FILE__, __LINE__);
    return;
  }

  pSendersNow = pTempS = calloc( 1, sizeof(SENDER));

  if(fscanf(input, "%f\t%f\t%f\t%u\t%f\n", &(pTempS->x), &(pTempS->y), &(pTempS->power), &bind, &(pTempS->freq)) == 0){
    printe("Can't read sender data from file.", __FILE__, __LINE__);
    return;
  }

  bindToReciever(rcvrAtIndex(bind), pTempS);

  for(i = 1; i < nSendersNow; i++){
    pTempS->pNext = calloc(1, sizeof(SENDER));
    pTempS->pNext->pPrev = pTempS;
    pTempS = pTempS->pNext;
    if(fscanf(input, "%f\t%f\t%f\t%u\t%f\n", &(pTempS->x), &(pTempS->y), &(pTempS->power), &bind, &(pTempS->freq)) == 0){
      printe("Can't read sender data from file.", __FILE__, __LINE__);
      return;
    }
    bindToReciever(rcvrAtIndex(bind), pTempS);
  }

  printd("Successfully read model data from file.", __FILE__, __LINE__);
}

/* Initialise random number generator */
void initRand(void){
  FILE *I = fopen("/dev/urandom", "rb");
  if(I == NULL){
    printw("No /dev/urandom?", __FILE__, __LINE__);
    return;
  }
  if(fread(&randSeed, 1, sizeof(int), I) == 0){
    printw("Failed to init generator", __FILE__, __LINE__);
    return;
  }
  (void)srand(randSeed);
  (void)fclose(I);
#ifdef DEBUG
  printd("Random number generator initialized", __FILE__, __LINE__);
#endif
}

/* Generate a linked list of the recievers */
RECIEVER *makeRcvrList(unsigned int nRecievers){
  unsigned int i = 0;
  RECIEVER *pTemp = NULL;

  if(nRecieversNow == 0){
    nRecieversNow = nRecievers;
  }else{
    printw("WARNING: Not creating new list of recievers over existing", __FILE__, __LINE__);
    return NULL;
  }

  if(nRecievers != 0){
    pTemp = pRecieversNow = calloc(1, sizeof(RECIEVER));
    if(pRecieversNow == NULL){
      printe("Failed to allocate memory for new reciever", __FILE__, __LINE__);
      return NULL;
    }
  }else{
    return NULL;
  }

  pTemp->recalc = 1;

  for(i = 1; i < nRecieversNow; i++){
    pTemp->pNext = calloc(1, sizeof(RECIEVER));
    if(pTemp->pNext == NULL){
      printe("Failed to allocate memory for new reciever", __FILE__, __LINE__);
      return NULL;
    }
    pTemp->pNext->pPrev = pTemp;
    pTemp = pTemp->pNext;
    pTemp->recalc = 1;
  }

  pTemp->pNext = NULL;

  printd("Created a list for recievers", __FILE__, __LINE__);

  return pRecieversNow;
}

/* Generate a linked list of the senders */
SENDER *makeSndrList(unsigned int nSenders){
  unsigned int i = 0;
  SENDER *pTemp = NULL;

  if(nSendersNow == 0){
    nSendersNow = nSenders;
  }else{
    printw("Not creating new list of recievers over existing", __FILE__, __LINE__);
    return NULL;
  }

  if(nSenders != 0){
    pTemp = pSendersNow = calloc(1, sizeof(SENDER));
    if(pSendersNow == NULL){
      printe("Failed to allocate memory for new sender", __FILE__, __LINE__);
      return NULL;
    }
  }else{
    return NULL;
  }

  for(i = 1; i < nSendersNow; i++){
    pTemp->pNext = calloc(1, sizeof(SENDER));
    if(pTemp->pNext == NULL){
      printe("Failed to allocate memory for new sender", __FILE__, __LINE__);
      return NULL;
    }
    pTemp->pNext->pPrev = pTemp;
    pTemp = pTemp->pNext;
  }

  pTemp->pNext = NULL;

  printd("Prepared linked list for transmitters", __FILE__, __LINE__);

  return pSendersNow;
}

/* Deallocate all lists */
void freeLists(void){
  SENDER *pTempS = NULL, *pTempS2 = NULL;
  RECIEVER *pTempR = NULL, *pTempR2 = NULL;

  if(pSendersNow != NULL){
    pTempS = pSendersNow;
    pTempS2 = pTempS->pNext;
    (void)free(pTempS);
    for(;pTempS2 != NULL;){
      pTempS = pTempS2;
      pTempS2 = pTempS2->pNext;
      (void)free(pTempS);
    }
    pSendersNow = NULL;
  }
  
  if(pRecieversNow != NULL){
    pTempR = pRecieversNow;
    pTempR2 = pTempR->pNext;
    (void)free(pTempR);
    for(;pTempR2 != NULL;){
      pTempR = pTempR2;
      pTempR2 = pTempR2->pNext;
      (void)free(pTempR);
    }
    pRecieversNow = NULL;
  }

  printd("All linked lists freed", __FILE__, __LINE__);
}

/* Add a new reciever to the list */
void addReciever(SENDER *pSender, unsigned int x, unsigned int y){
  RECIEVER *pTemp = pRecieversNow;

  if(pTemp == NULL){
    printe("Failed to link to NULL.", __FILE__, __LINE__);
    return;
  }

  for(;pTemp->pNext != NULL; pTemp = pTemp->pNext);

  pTemp->pNext = calloc(1, sizeof(RECIEVER));

  pTemp->pNext->pPrev = pTemp;

  pTemp->pNext->x = x;
  pTemp->pNext->y = y;
  pTemp->pNext->pNext = NULL;

  nRecieversNow++;

  pTemp->pNext->recalc = 1;
  
  if(bindMode == NEAR || pSender == NULL){
     bindToReciever(pTemp->pNext, getNearest(pTemp->pNext)); 
  }else if(bindMode == RAND){
    bindToReciever(pTemp->pNext, pSender);
  }else if(bindMode == MAXS){

  }

  pTemp->pNext->pNext = NULL;
}

/* Remove the reciever from the list */
void rmReciever(RECIEVER *pReciever){
  if(pReciever == NULL || pRecieversNow == NULL){
    printe("Got NULL in rmReciever()", __FILE__, __LINE__);
    return;
  }

  if(pReciever == pRecieversNow){
    pRecieversNow = pRecieversNow->pNext;
  }

  if(pReciever->pPrev != NULL){
    pReciever->pPrev->pNext = pReciever->pNext;
  }
  
  if(pReciever->pNext != NULL){
    pReciever->pNext->pPrev = pReciever->pPrev;
  }

  if(pSendersNow != NULL){
    unbindReciever(pReciever);
  }
  
  nRecieversNow--;
  (void)free(pReciever);

}

/* Get the nearest transmitter. */
SENDER *getNearest(RECIEVER *pReciever){
  unsigned int i = 0;
  float minDist = sqrt(maxWidthNow*maxWidthNow + maxHeightNow*maxHeightNow), dist = 0.0;
  SENDER *pTempS = pSendersNow, *pSelection = NULL;

  if(pReciever == NULL){
    printe("No reciever specified", __FILE__, __LINE__);
    return NULL;
  }

  if(pRecieversNow == NULL || pSendersNow == NULL){
    printe("Model not initialized", __FILE__, __LINE__);
    return NULL;
  }

  for(i = 0;i < nSendersNow; i++){
    if((dist = sqrt((pReciever->x - pTempS->x)*(pReciever->x - pTempS->x) + (pReciever->y - pTempS->y)*(pReciever->y - pTempS->y))) < minDist){
      pSelection = pTempS;
      minDist = dist;
    }
    pTempS = pTempS->pNext;
  }

  return pSelection;
}
/*
SENDER *getMaxSig(RECIEVER *pReciever){
  unsigned int i = 0;
  float minSig = -INFINITY;
  SENDER *pTempS = pSendersNow, *pSelection = NULL;

  if(!pReciever){
    (void)puts("Error, no reciever specified.");
    return NULL;
  }

  if(!pRecieversNow || !pSendersNow){
    (void)puts("Error, model now initialized.");
    return NULL;
  }

  for(i = 0; i < nSendersNow; i++){

  }
}
*/
void setConnBehaviour(unsigned char mode){
  bindMode = mode;
}
