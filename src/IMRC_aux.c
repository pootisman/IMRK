#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define NOT_MAIN
#include "IMRC_types.h"

extern long double *gA, percentY, percentX;
extern unsigned int nRecieversNow, nSendersNow, gASize, randSeed;
extern unsigned char lineWidth, spotSize;
extern RECIEVER *pRecieversNow;
extern SENDER *pSendersNow;

/* Address of the sender at index */
SENDER *sndrAtIndex(unsigned int index){
  SENDER *pTempS = pSendersNow;
  
  if(pTempS){
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
    (void)puts("Error, no senders in sndrAtIndex");
    return NULL;
  }
}

/* Address of the reciever at index */
RECIEVER *rcvrAtIndex(unsigned int index){
  RECIEVER *pTempR = pRecieversNow;

  if(pTempR){
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
    (void)puts("Error, no recievers in rcvrAtIndex");
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

  if(pReciever->pOwner){
    (void)puts("Error in bindToReciever, reciever already occupied.");
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

  pReciever->recalc = 1;
  pReciever->pOwner = pSender;

#ifdef DEBUG
  (void)printf("DEBUG: Successfull bind:[%Lf:%Lf]\n", pReciever->x, pReciever->y);
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
#ifdef DEBUG
	(void)printf("DEBUG: Successfull unbind:[%Lf:%Lf]\n", pReciever->x, pReciever->y);
#endif
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

  if(!pRecieversNow || nRecieversNow == 0 || !output){
    (void)puts("Error, failed to dump data to file.");
    return;
  }

  (void)fprintf(output, "%d\n", step);

  for(;pTempR; pTempR = pTempR->pNext, i++){
    (void)fprintf(output, "%d\t%Lf\t%Lf\t%Lf\n", i, pTempR->x,  pTempR->y, pTempR->SNRLin);
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

  pTempR->recalc = 1;

  (void)fscanf(input, "%Lf\t%Lf\n", &(pTempR->x), &(pTempR->y));

  for(i = 1; i < nRecieversNow; i++){
    pTempR->pNext = calloc(1, sizeof(RECIEVER));
    pTempR->pNext->pPrev = pTempR;
    pTempR = pTempR->pNext;
    pTempR->recalc = 1;
    (void)fscanf(input, "%Lf\t%Lf\n", &(pTempR->x), &(pTempR->y));
  }

  (void)fscanf(input, "%u\n", &nSendersNow);

  pSendersNow = pTempS = calloc( 1, sizeof(SENDER));

  (void)fscanf(input, "%Lf\t%Lf\t%Lf\t%u\t%Lf\n", &(pTempS->x), &(pTempS->y), &(pTempS->power), &bind, &(pTempS->freq));

  bindToReciever(rcvrAtIndex(bind), pTempS);

  for(i = 1; i < nSendersNow; i++){
    pTempS->pNext = calloc(1, sizeof(SENDER));
    pTempS->pNext->pPrev = pTempS;
    pTempS = pTempS->pNext;
    (void)fscanf(input, "%Lf\t%Lf\t%Lf\t%u\t%Lf\n", &(pTempS->x), &(pTempS->y), &(pTempS->power), &bind, &(pTempS->freq));
    bindToReciever(rcvrAtIndex(bind), pTempS);
  }

#ifdef DEBUG
  (void)puts("DEBUG: Successfully read model data from file.");
#endif
}

/* Initialise random number generator */
void initRand(void){
  FILE *I = fopen("/dev/urandom", "rb");
  (void)fread(&randSeed, 1, sizeof(int), I);
  (void)srand(randSeed);
  (void)fclose(I);
#ifdef DEBUG
  (void)printf("DEBUG: Random number generator initialized, rand returned %d out of %d\n", rand(), RAND_MAX);
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

  pTemp->recalc = 1;

  for(i = 1; i < nRecieversNow; i++){
    pTemp->pNext = calloc(1, sizeof(RECIEVER));
    if(!pTemp->pNext){
      (void)puts("Error allocating memory for new reciever");
      return NULL;
    }
    pTemp->pNext->pPrev = pTemp;
    pTemp = pTemp->pNext;
    pTemp->recalc = 1;
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

  for(i = 1; i < nSendersNow; i++){
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
void addReciever(SENDER *pSender, unsigned int x, unsigned int y){
  RECIEVER *pTemp = pRecieversNow;

  if(!pTemp){
    (void)puts("Failed to link to NULL.");
    return;
  }

  for(;pTemp->pNext != NULL; pTemp = pTemp->pNext);

  pTemp->pNext = calloc(1, sizeof(RECIEVER));

  pTemp->pNext->pPrev = pTemp;

  pTemp->pNext->x = x;
  pTemp->pNext->y = y;

  ++nRecieversNow;

  if(pSender){
    pTemp->pNext->recalc = 1;
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
  }

  if(pReciever == pRecieversNow){
    pRecieversNow = pRecieversNow->pNext;
  }

  if(pReciever->pPrev){
    pReciever->pPrev->pNext = pReciever->pNext;
  }
  
  if(pReciever->pNext){
    pReciever->pNext->pPrev = pReciever->pPrev;
  }

  if(pSendersNow){
    unbindReciever(pReciever);
  }
  
  --nRecieversNow;
  (void)free(pReciever);

#ifdef DEBUG
  (void)puts("DEBUG: Removed reciever from list.");
#endif
}
