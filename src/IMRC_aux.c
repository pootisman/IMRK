#include <stdlib.h>
#include <stdio.h>
#include "IMRC_types.h"

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

void initRand(void){
  FILE *I = fopen("/dev/urandom", "rb");
  int seed = 0;

  (void)fread(&seed, 1, sizeof(int), I);

  (void)srand(seed);

  (void)fclose(I);
}
