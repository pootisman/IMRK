#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <math.h>
#include "IMRC_types.h"


/* Functions for verification of our models.
 * 
 * collByPopl - starts collecting data for checkByPopl.
 * checkByPopl - verifies our model from population aspect (average number of elements in the system), returns text report.
 * 
 * collByLifespan - starts collecting data for checkByLifespan.
 * checkByLifespan - verifies our model from lifespan data (growth, decrease, M, sigma), returns text report.
 *
 */

extern char verification;
extern const unsigned int nRecieversNow;
extern RECIEVER *pRecieversNow;

unsigned int iter = 0;
FILE *SNROut = NULL, *LSpanOut = NULL, *PoplOut = NULL;

void setVerMode(unsigned char verMode){
  verification = verMode;
}

void collByPopl(void){
  
}

char checkByPopl(void){
  return 1;
}

void collByLifespan(void){
  
}

char checkByLifespan(void){
  return 1;
}

void avgSNR(void){
  unsigned int i = 0;
  double res = 0.0;

  if(!SNROut){
    SNROut = fopen("avg_SNR(t).dat", "w");
    (void)fprintf(SNROut, "###TIME###|###aSNR###\n");
  }

  for(i = 0; i < nRecieversNow; i++){
    res += (pRecieversNow + i)->SNRLin;
  }

  (void)printf( "%d\t%f\n", iter, res/nRecieversNow);
}

void stopVerr(void){
  if(SNROut){
    (void)fclose(SNROut);
  }
}
