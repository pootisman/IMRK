#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <math.h>
#include "IMRC_models.h"
#include "IMRC_types.h"

#ifdef DEBUG
#include <mtrace.h>
#endif

#define VALID_ARGS "W:H:R:S:F:O:T:"
#define DEF_WIDTH 255
#define DEF_HEIGHT 255
#define DEF_THREADS 1
#define DEF_SENDERS 2
#define DEF_RECIEVERS 2

int main(int argc, char *argv[]){
  unsigned int maxHeight = DEF_HEIGHT, maxWidth = DEF_WIDTH, nRecievers = DEF_RECIEVERS, nSenders = DEF_SENDERS, nThreads = DEF_THREADS, i = 2;
  int opt = 0;
  FILE *I = NULL, *O = NULL;
  RECIEVER *pRecList = NULL;
  SENDER *pSendList = NULL;

#ifdef DEBUG
  mcheck();
#endif

  /* Parse program arguments. */
  while((opt = getopt(argc, argv, VALID_ARGS)) != -1){
    switch(opt){
      case('H'):{
        if(argv[i+1] != NULL){
          maxHeight = atoi(argv[i + 1]);
	}
	i++;
        break;
      }
      case('W'):{
        if(argv[i + 1] != NULL){
	  maxWidth = atoi(argv[i + 1]);
	}
	i++;
	break;
      }
      case('T'):{
	if(argv[i + 1] != NULL){
	  nThreads = atoi(argv[i + 1]);
	}
	i++;
	break;
      }
      case('S'):{
	if(argv[i + 1] != NULL){
	  nSenders = atoi(argv[i + 1]);
	}
	i++;
	break;
      }
      case('R'):{
        if(argv[i + 1] != NULL){
	  nRecievers = atoi(argv[i + 1]);
	}
	break;
      }
      case('F'):{
	if(argv[i + 1] != NULL){
	  I = fopen(argv[i + 1], "r");

	  if(!I){
	    (void)puts("Error, can't open file, terminating.");
	    if(O){
	      (void)fclose(O);
	    }
	    return EXIT_FAILURE;
	  }
	}
	break;
      }
      case('O'):{
	if(argv[i + 1] != NULL){
	  O = fopen( argv[i + 1],"w");
	  if(!O){
	    (void)puts("Error, can't open file, terminating.");
	    if(I){
	      (void)fclose(I);
	    }
	    return EXIT_FAILURE;
	  }
	}
	break;
      }
      default:{
        (void)puts("Unknown argument.");
	break;
      }
    }
    ++i;
  }


  pRecList = calloc(nRecievers, sizeof(RECIEVER));
  pSendList = calloc(nSenders, sizeof(SENDER));

  pRecList->x = 16;
  pRecList->y = 16;
  (pRecList + 1)->x = 0;
  (pRecList + 1)->y = 0;

  pSendList->x = 0;
  pSendList->y = 0;
  pSendList->power = 1;
  pSendList->pRecepient = (pRecList);

  (pSendList + 1)->x = 16;
  (pSendList + 1)->y = 16;
  (pSendList + 1)->power = 1;
  (pSendList + 1)->pRecepient = (pRecList + 1);

  calcPower(pRecList, pSendList, nSenders, nRecievers, 0);

  (void)printf("LSNR for first Node: %f\nLSNR for second Node: %f\n", (pRecList)->SNRLin, (pRecList + 1)->SNRLin);

  (void)free(pRecList);
  (void)free(pSendList);

  return EXIT_SUCCESS;
}
