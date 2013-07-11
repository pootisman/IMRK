#include <stdlib.h>
#include <stdio.h>
#include "IMRC_types.h"

void bindToReciever(RECIEVER *pRecievers, SENDER *pSender, unsigned int recNum){
  pSender->pRecepients = calloc(1, sizeof(RECIEVERS_LLIST));
  pSender->pRecepients->pTarget = (pRecievers + recNum);
  pSender->nRecepients = 1;
  pSender->pRecepients->pNext =  NULL;
}

void dumpToFile( const RECIEVER *pRecievers, const unsigned int nRecievers, FILE *output){
  unsigned int i = 0;

  if(!pRecievers || nRecievers == 0 || !output){
    (void)puts("Error, failed to dump data to file.");
    return;
  }

  for(i = 0; i < nRecievers; i++){
    (void)fprintf(output, "%d\t%f\t%f\t%f\n", i, (pRecievers + i)->x,  (pRecievers + i)->y, (pRecievers + i)->SNRLin);
  }
}

void readFromFile(RECIEVER **pRecievers, SENDER **pSenders, unsigned int *nRecievers, unsigned int *nSenders, FILE *input){
  unsigned int bind = 0, i = 0;
 
  if(!input || !pSenders || !pRecievers){
    (void)puts("Error, can't read from file.");
    return;
  }

  (void)fscanf(input, "%u\n", nRecievers);

  *pRecievers = calloc( *nRecievers, sizeof(RECIEVER));

  for(i = 0; i < *nRecievers; i++){
    (void)fscanf(input, "%f\t%f\n", &((*(pRecievers) + i)->x), &((*(pRecievers) + i)->y));
  }

  (void)fscanf(input, "%u\n", nSenders);

  *pSenders = calloc( *nSenders, sizeof(SENDER));

  for(i = 0; i < *nSenders; i++){
    (void)fscanf(input, "%f\t%f\t%f\t%u\n", &((*(pSenders) + i)->x), &((*(pSenders) + i)->y), &((*(pSenders) + i)->power), &bind);
    bindToReciever(*pRecievers, ((*pSenders) + i), bind);
  }
}

void initRand(void){
  FILE *I = fopen("/dev/urandom", "rb");
  int seed = 0;

  (void)fread(&seed, 1, sizeof(int), I);

  (void)srand(seed);

  (void)fclose(I);
}
