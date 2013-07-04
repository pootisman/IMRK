#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <math.h>
#include "IMRC_models.h"
#include "IMRC_types.h"
#include "IMRC_gl.h"
#include "IMRC_aux.h"

#ifdef DEBUG
#include <mtrace.h>
#endif

#define VALID_ARGS "W:H:R:S:F:O:T:G"
#define DEF_WIDTH 255
#define DEF_HEIGHT 255
#define DEF_THREADS 1
#define DEF_SENDERS 2
#define DEF_RECIEVERS 2

int main(int argc, char *argv[]){
  unsigned int maxHeight = DEF_HEIGHT, maxWidth = DEF_WIDTH, nRecievers = DEF_RECIEVERS, nSenders = DEF_SENDERS, nThreads = DEF_THREADS, i = 2;
  int opt = 0, useGL = 0, fileo = 0, filei = 0;
  FILE *I = NULL, *O = NULL;
  RECIEVER *pRecList = NULL;
  SENDER *pSendList = NULL;

#ifdef DEBUG
  mcheck();
#endif

  /* Parse program arguments. */
  while((opt = getopt(argc, argv, VALID_ARGS)) != -1){
    switch(opt){
      case('G'):{
	useGL = 1;
	break;
      }
      case('H'):{
        if(argv[i] != NULL){
          maxHeight = atoi(argv[i]);
	}
	++i;
        break;
      }
      case('W'):{
        if(argv[i] != NULL){
	  maxWidth = atoi(argv[i]);
	}
	++i;
	break;
      }
      case('T'):{
	if(argv[i] != NULL){
	  nThreads = atoi(argv[i]);
	}
	++i;
	break;
      }
      case('S'):{
	if(argv[i] != NULL){
	  nSenders = atoi(argv[i]);
	}
	++i;
	break;
      }
      case('R'):{
        if(argv[i] != NULL){
	  nRecievers = atoi(argv[i]);
	}
	i++;
	break;
      }
      case('F'):{
	if(argv[i] != NULL){
	  I = fopen(argv[i], "r");

	  if(!I){
	    (void)puts("Error, can't open file, terminating.");
	    if(O){
	      (void)fclose(O);
	    }
	    return EXIT_FAILURE;
	  }
	}
	filei = 1;
	++i;
	break;
      }
      case('O'):{
	if(argv[i] != NULL){
	  O = fopen( argv[i],"w");
	  if(!O){
	    (void)puts("Error, can't open file, terminating.");
	    if(I){
	      (void)fclose(I);
	    }
	    return EXIT_FAILURE;
	  }
	}
	fileo = 1;
	++i;
	break;
      }
      default:{
        (void)puts("Unknown argument.");
	break;
      }
    }
    ++i;
  }

  initRand();

  pRecList = calloc(nRecievers, sizeof(RECIEVER));
  pSendList = calloc(nSenders, sizeof(SENDER));

  spawnRecievers( pRecList, nRecievers, maxWidth, maxHeight);
  spawnTransmitters( pSendList, pRecList, nSenders, nRecievers, maxWidth, maxHeight);

  calcPower(pRecList, pSendList, nSenders, nRecievers, 0);

  if(fileo){
  dumpToFile(pRecList, nRecievers, O);
  }

  if(useGL){
    initData(pRecList, pSendList, nSenders, nRecievers);
    initGraphics(75, 1366, 768, &argc, argv, maxWidth, maxHeight);
  }

  (void)free(pRecList);
  (void)free(pSendList);

  if(fileo){
    (void)fclose(O);
  }
  if(filei){
    (void)fclose(I);
  }

  return EXIT_SUCCESS;
}
