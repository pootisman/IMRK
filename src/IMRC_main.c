#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <math.h>
#include "IMRC_models.h"
#include "IMRC_types.h"
#include "IMRC_gl.h"
#include "IMRC_aux.h"
#include "IMRC_ver.h"

/* Macros, Gaaaah! */
#define VALID_PROB(prob1, prob2) ((prob1 >= 0.0) && (prob1 <= 1.0) && (prob2 >= 0.0) && (prob2 <= 1.0) && ((prob1 + prob2) == 1))

/* Help string */
#define HELP "Help for IMRC:\n===================================\n-W maximum x coordinates.\n-H maximum y coordinates.\n-R amount of recievers to spawn.\n-S amount of transmitters to spawn.\n-F file to read initial condition from.\n-O file for SNR output.\n-T number of threads to run.\n-G use graphics.\n-M model selection\n-D Die probability\n-N Spawn probability\n-I Iterations to run\n-h This message\n===================================\nINFO FOR GRAPHICS MODE:\nBLUE - BS.\n[RED..GREEN] - luser.\nCompiled at %s"

#define VALID_ARGS "t:W:H:R:S:F:O:T:M:D:N:I:VBGh" /* Valid arguments for the shell */
#define DEF_WIDTH 255 /* Default width of our silencing matrice */
#define DEF_HEIGHT 255 /* Default height of our silencing matrice */
#define DEF_THREADS 0 /* Default number of threads to use */
#define DEF_SENDERS 2 /* Default number of senders to spawn */
#define DEF_RECIEVERS 2 /* Default number of recievers to spawn */
#define DEF_MODEL 1 /* Default model type */
#define DEF_PROB_NEW 0.99 /* Default probability of the new reciever spawn */
#define DEF_PROB_DIE 0.1 /* Default probability of the reciever death (disconnect) */
#define DEF_ITERATIONS 1 /* Default number of iterations */
#define DEF_DELAY 500000000 /* Default delay in nanoseconds */

int main(int argc, char *argv[]){
  unsigned int maxHeight = DEF_HEIGHT, maxWidth = DEF_WIDTH, nRecievers = DEF_RECIEVERS, nSenders = DEF_SENDERS, nIterations = DEF_ITERATIONS, model = DEF_MODEL, nanoDelay = DEF_DELAY;
  int opt = 0, useGL = 0, fileo = 0, filei = 0, nThreads = DEF_THREADS, i = 1;
  FILE *I = NULL, *O = NULL;
  float probSpawn = DEF_PROB_NEW, probDie = DEF_PROB_DIE;
  unsigned short modeset = NEAR;

  /* Parse program arguments. */
  while((opt = getopt(argc, argv, VALID_ARGS)) != -1){
    switch(opt){
      case('V'):{
        setVerMode(1);
        break;
      }
      case('B'):{
	modeset = NEAR;
	break;
      }
      case('D'):{
        if(argv[i + 1]){
	  probDie = atof(argv[i + 1]);
	  ++i;
	}
	break;
      }
      case('N'):{
      	if(argv[i + 1]){
	  probSpawn = atof(argv[i + 1]);
	  ++i;
	}
	break;
      }
      case('I'):{
	if(argv[i + 1]){
          nIterations = atoi(argv[i + 1]);
	  ++i;
	}
	break;
      }
      case('G'):{
	useGL = 1;
	break;
      }
      case('H'):{
        if(argv[i + 1]){
          maxHeight = atoi(argv[i + 1]);
	  ++i;
	}
        break;
      }
      case('W'):{
        if(argv[i + 1]){
	  maxWidth = atoi(argv[i + 1]);
	  ++i;
	}
	break;
      }
      case('T'):{
	if(argv[i + 1]){
	  nThreads = atoi(argv[i + 1]);
	  ++i;
	}
	break;
      }
      case('S'):{
	if(argv[i + 1]){
	  nSenders = atoi(argv[i + 1]);
	  ++i;
	}
	break;
      }
      case('R'):{
        if(argv[i + 1]){
	  nRecievers = atoi(argv[i + 1]);
	  ++i;
	}
	break;
      }
      case('M'):{
        if(argv[i + 1]){
	  model = atoi(argv[i + 1]);
	  ++i;
	}
	break;
      }
      case('F'):{
	if(argv[i + 1]){
	  I = fopen(argv[i + 1], "r");
	  if(!I){
	    (void)puts("Error, can't open file, terminating.");
	    if(O){
	      (void)fclose(O);
	    }
	    return EXIT_FAILURE;
	  }
	  ++i;
	}
	filei = 1;
	break;
      }
      case('O'):{
	if(argv[i + 1]){
	  O = fopen( argv[i + 1],"w");
	  if(!O){
	    (void)puts("Error, can't open file, terminating.");
	    if(I){
	      (void)fclose(I);
	    }
	    return EXIT_FAILURE;
	  }
	  ++i;
	}
	fileo = 1;
	break;
      }
      case('t'):{
	if(argv[i + 1]){
	  nanoDelay = atoi(argv[i + 1]);
	  ++i;
	}
	break;
      }
      case('h'):{
        (void)printf(HELP, __TIMESTAMP__);
	return EXIT_SUCCESS;
      }
      default:{
	break;
      }
    }
    ++i;
  }
 
  setConnBehaviour(modeset);

  initModel(maxWidth, maxHeight, model, nRecievers, nSenders, nThreads, I, useGL, probSpawn, probDie);
  
  modelLoop(O, nIterations, nanoDelay);

  stopModel();

  if(fileo){
    (void)fclose(O);
  }
  if(filei){
    (void)fclose(I);
  }

  return EXIT_SUCCESS;
}
