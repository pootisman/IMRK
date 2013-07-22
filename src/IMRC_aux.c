#include <stdlib.h>
#include <stdio.h>
#define NOT_MAIN
#include "IMRC_types.h"

extern float *gA, percentY, percentX;
extern unsigned int nRecieversNow, nSendersNow, gASize;
extern unsigned char lineWidth, spotSize;
extern RECIEVER *pRecieversNow;
extern SENDER *pSendersNow;

/* Address of the sender at index */
SENDER *sndrAtIndex(unsigned int index){
  SENDER *pTempS = pSendersNow;
  
  if(pSendersNow){
    for(;index > 0; index--){
      if(pTempS){
        pTempS = pTempS->pNext;
      }else{
        (void)puts("Error, index out of bounds");
        return NULL;
      }
    }
    return pTempS;
  }else{
    (void)puts("Error, recieved NULL in sndrAtIndex");
    return NULL;
  }
}

/* Address of the reciever at index */
RECIEVER *rcvrAtIndex(unsigned int index){
  RECIEVER *pTempR = pRecieversNow;

  if(pRecieversNow){
    for(;index > 0; index--){
      if(pTempR){
        pTempR = pTempR->pNext;
      }else{
        (void)puts("Error, index out of bounds");
	return NULL;
      }
    }
    return pTempR;
  }else{
    (void)puts("Error, recieved NULL in rcvrAtIndex");
    return NULL;
  }
}

/* Bind transmitter to specific reciever */
void bindToReciever(RECIEVER *pReciever, SENDER *pSender){
  RECIEVERS_LLIST *pTemp = NULL;

  if(!pReciever || !pSender){
    (void)puts("Error in bindToReciever, got NULL");
    return;
  }

  if(!(pSender->pRecepients)){
    pSender->pRecepients = calloc(1, sizeof(RECIEVERS_LLIST));
    pSender->nRecepients = 1;
    pSender->pRecepients->pTarget = pReciever;
  }else{
    pTemp = pSender->pRecepients;
    for(;pTemp->pNext; pTemp = pTemp->pNext);
    pTemp->pNext = calloc(1, sizeof(RECIEVERS_LLIST));
    pTemp->pNext->pTarget = pReciever;
    pTemp->pNext->pPrev = pTemp;
    pSender->nRecepients++;
  }

  pReciever->pOwner = pSender;

#ifdef DEBUG
  (void)printf("DEBUG: Successfull bind:[%f:%f]\n", pReciever->x, pReciever->y);
#endif
}

/* Unbind reciever for transmitter */
void unbindReciever(RECIEVER *pReciever){
  SENDER *pTempS = pSendersNow;
  RECIEVERS_LLIST *pTempR = NULL;
  unsigned int i = 0;

  if(!pTempS){
    (void)puts("Error, failed to unbind from NULL");
    return;
  }

  for(;pTempS; pTempS = pTempS->pNext){
    pTempR = pTempS->pRecepients;
    for(i = 0; i < pTempS->nRecepients; ++i){
      if(pTempR->pTarget == pReciever){
        if(pTempR->pPrev){
          pTempR->pPrev->pNext = pTempR->pNext;
	}
	if(pTempR->pNext)
	 pTempR->pNext->pPrev = pTempR->pPrev;
        }
      }
    }
 
#ifdef DEBUG
  (void)printf("DEBUG: Successfull unbind:[%f:%f]\n", pReciever->x, pReciever->y);
#endif

}

/* Write current situation to disk */
void dumpToFile(FILE *output){
  unsigned int i = 0;
  RECIEVER *pTempR = pRecieversNow;

  if(!pRecieversNow || nRecieversNow == 0 || !output){
    (void)puts("Error, failed to dump data to file.");
    return;
  }

  for(;pTempR; pTempR = pTempR->pNext){
    (void)fprintf(output, "%d\t%f\t%f\t%f\n", i, pTempR->x,  pTempR->y, pTempR->SNRLin);
  }
#ifdef DEBUG
  (void)puts("DEBUG: Successfully written data to file.");
#endif
}

/* Read initial data from file */
void readFromFile(FILE *input){
  unsigned int bind = 0, i = 0;
  RECIEVER *pTempR = NULL;
  SENDER *pTempS = NULL;

  if(!input){
    (void)puts("Error, can't read from file.");
    return;
  }

  (void)fscanf(input, "%u\n", &nRecieversNow);

  pRecieversNow = pTempR = calloc( 1, sizeof(RECIEVER));

  (void)fscanf(input, "%f\t%f\n", &(pTempR->x), &(pTempR->y));

  for(i = 1; i < nRecieversNow; i++){
    pTempR->pNext = calloc(1, sizeof(RECIEVER));
    pTempR->pNext->pPrev = pTempR;
    pTempR = pTempR->pNext;
    (void)fscanf(input, "%f\t%f\n", &(pTempR->x), &(pTempR->y));
  }

  (void)fscanf(input, "%u\n", &nSendersNow);

  pSendersNow = pTempS = calloc( 1, sizeof(SENDER));

  for(i = 0; i < nSendersNow; i++){
    (void)fscanf(input, "%f\t%f\t%f\t%u\n", &(pTempS->x), &(pTempS->y), &(pTempS->power), &bind);
    bindToReciever(rcvrAtIndex(bind), pTempS);
    pTempS->pNext = calloc(1, sizeof(SENDER));
    pTempS->pNext->pPrev = pTempS;
    pTempS = pTempS->pNext;
  }
  pTempS->pPrev->pNext = NULL;
  (void)free(pTempS);

#ifdef DEBUG
  (void)puts("DEBUG: Successfully read model data from file.");
#endif
}

/* Initialise random number generator */
void initRand(void){
  FILE *I = fopen("/dev/urandom", "rb");
  int seed = 0;
  (void)fread(&seed, 1, sizeof(int), I);
  (void)srand(seed);
  (void)fclose(I);
#ifdef DEBUG
  (void)puts("DEBUG: Random number generator initialized.");
#endif
}

/* Generate a linked list of the recievers */
RECIEVER *makeRcvrList(unsigned int nRecievers){
  unsigned int i = 0;
  RECIEVER *pTemp = NULL;

  nRecieversNow = nRecievers;

  if(nRecievers != 0){
    pTemp = pRecieversNow = calloc(1, sizeof(RECIEVER));
    if(!pRecieversNow){
      (void)puts("Error allocating memory for new reciever");
      return NULL;
    }
  }else{
    return NULL;
  }

  for(i = 1; i < nRecievers; i++){
    pTemp->pNext = calloc(1, sizeof(RECIEVER));
    if(!pTemp->pNext){
      (void)puts("Error allocating memory for new reciever");
      return NULL;
    }
    pTemp->pNext->pPrev = pTemp;
    pTemp = pTemp->pNext;
  }

#ifdef DEBUG
  (void)puts("DEBUG: Prepared linked list for recievers.");
#endif

  return pRecieversNow;
}

/* Generate a linked list of the senders */
SENDER *makeSndrList(unsigned int nSenders){
  unsigned int i = 0;
  SENDER *pTemp = NULL;

  nSendersNow = nSenders;

  if(nSenders != 0){
    pTemp = pSendersNow = calloc(1, sizeof(SENDER));
    if(!pSendersNow){
      (void)puts("Error allocating memory for new sender");
      return NULL;
    }
  }else{
    return NULL;
  }

  for(i = 1; i < nSenders; i++){
    pTemp->pNext = calloc(1, sizeof(SENDER));
    if(!pTemp->pNext){
      (void)puts("Error allocating memory for new sender");
      return NULL;
    }
    pTemp->pNext->pPrev = pTemp;
    pTemp = pTemp->pNext;
  }

#ifdef DEBUG
  (void)puts("DEBUG: Prepared linked list for transmitters.");
#endif

  return pSendersNow;
}

/* Deallocate all lists */
void freeLists(void){
  SENDER *pTempS = NULL, *pTempS2 = NULL;
  RECIEVER *pTempR = NULL, *pTempR2 = NULL;

  if(pSendersNow){
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
  
  if(pRecieversNow){
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

#ifdef DEBUG
  (void)puts("DEBUG: All linked lists freed.");
#endif

}

/* Add a new reciever to the list */
void addReciever(SENDER *pSender, RECIEVER *pRecievers, unsigned int x, unsigned int y){
  RECIEVER *pTemp = pRecievers;

  if(!pTemp){
    (void)puts("Failed to link to NULL.");
    return;
  }

  for(;pTemp->pNext != NULL; pTemp = pTemp->pNext);

  pTemp->pNext = calloc(1, sizeof(RECIEVER));

  nRecieversNow++;

  pTemp->pNext->x = x;
  pTemp->pNext->y = y;

  if(pSender){
    bindToReciever(pTemp->pNext, pSender); 
  }

#ifdef DEBUG
  (void)puts("DEBUG: Added new reciever to list.");
#endif

}

/* Remove the reciever from the list */
void rmReciever(RECIEVER *pReciever){
  if(!pReciever || !pRecieversNow){
    (void)puts("Error, got NULL in rmReciever()");
    return;
  }else{
    if(pReciever->pPrev){
      pReciever->pPrev->pNext = pReciever->pNext;
    }
    if(pReciever->pNext){
      pReciever->pNext->pPrev = pReciever->pPrev;
    }

    if(pSendersNow){
      unbindReciever(pReciever);
    }

    (void)free(pReciever);
    nRecieversNow--;

  }

#ifdef DEBUG
  (void)puts("DEBUG: Removed reciever from list.");
#endif
}
