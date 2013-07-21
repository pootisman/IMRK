#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <math.h>
#include "IMRC_models.h"
#include "IMRC_types.h"
#include "IMRC_gl.h"
#include "IMRC_aux.h"

#ifdef DEBUG
#include <mcheck.h>
#endif

#define HELP "Help for IMRC:\n===================================\n-W maximum x coordinates.\n-H maximum y coordinates.\n-R amount of recievers to spawn.\n-S amount of transmitters to spawn.\n-F file to read graph from.\n-O file for SNR output.\n-T number of threads to run.\n-G use graphics.\n-M model selection\n-h This message\n===================================\nINFO FOR GRAPHICS MODE:\nRED - transmitter.\nYELLOW - reciever."

#define VALID_ARGS "W:H:R:S:F:O:T:GM:h"
#define DEF_WIDTH 255
#define DEF_HEIGHT 255
#define DEF_THREADS 0
#define DEF_SENDERS 2
#define DEF_RECIEVERS 2
#define DEF_MODEL 1

int main(int argc, char *argv[]){
  unsigned int maxHeight = DEF_HEIGHT, maxWidth = DEF_WIDTH, nRecievers = DEF_RECIEVERS, nSenders = DEF_SENDERS, i = 2, model = DEF_MODEL;
  int opt = 0, useGL = 0, fileo = 0, filei = 0, nThreads = DEF_THREADS;
  FILE *I = NULL, *O = NULL;
#ifdef DEBUG
  mtrace();
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
	++i;
	break;
      }
      case('M'):{
        if(argv[i] != NULL){
	  model = atoi(argv[i]);
	}
	++i;
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
      case('h'):{
        (void)puts(HELP);
	return EXIT_SUCCESS;
      }
      default:{
        (void)puts("Unknown argument.");
	break;
      }
    }
    ++i;
  }

  initRand();

  prepareSilencing(maxWidth, maxHeight);

  if(!filei){
    makeRcvrList(nRecievers);
    makeSndrList(nSenders);
    spawnRecievers(maxWidth, maxHeight);
    spawnTransmitters(maxWidth, maxHeight);
  }else{
    readFromFile(I);
  }

  calcPower(maxWidth, nThreads, model);

  if(fileo){
    dumpToFile(O);
    (void)fflush(O);
  }

  if(useGL){
    initGraphics( &argc, argv, maxWidth, maxHeight);
  }

  stopModel();

  if(fileo){
    (void)fclose(O);
  }
  if(filei){
    (void)fclose(I);
  }

  return EXIT_SUCCESS;
}
