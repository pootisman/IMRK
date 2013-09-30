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

/* Macros, Gaaaah! */
#define VALID_PROB(prob1, prob2) ((prob1 >= 0.0) && (prob1 <= 1.0) && (prob2 >= 0.0) && (prob2 <= 1.0) && ((prob1 + prob2) == 1))

/* Help string */
#define HELP "Help for IMRC:\n===================================\n-W maximum x coordinates.\n-H maximum y coordinates.\n-R amount of recievers to spawn.\n-S amount of transmitters to spawn.\n-F file to read graph from.\n-O file for SNR output.\n-T number of threads to run.\n-G use graphics.\n-M model selection\n-D Die probability\n-N Spawn probability\n-I Iterations to run\n-h This message\n===================================\nINFO FOR GRAPHICS MODE:\nRED - transmitter.\nYELLOW - reciever."

#define VALID_ARGS "W:H:R:S:F:O:T:M:D:N:I:Gh" /* Valid arguments for the shell */
#define DEF_WIDTH 255 /* Default width of our silencing matrice */
#define DEF_HEIGHT 255 /* Default height of our silencing matrice */
#define DEF_THREADS 0 /* Default number of threads to use */
#define DEF_SENDERS 2 /* Default number of senders to spawn */
#define DEF_RECIEVERS 2 /* Default number of recievers to spawn */
#define DEF_MODEL 1 /* Default model type */
#define DEF_PROB_NEW 0.8 /* Default probability of the new reciever spawn */
#define DEF_PROB_DIE 0.1 /* Default probability of the reciever death (disconnect) */
#define DEF_ITERATIONS 1 /* Default number of iterations */

int main(int argc, char *argv[]){
  unsigned int maxHeight = DEF_HEIGHT, maxWidth = DEF_WIDTH, nRecievers = DEF_RECIEVERS, nSenders = DEF_SENDERS, nIterations = DEF_ITERATIONS, model = DEF_MODEL;
  int opt = 0, useGL = 0, fileo = 0, filei = 0, nThreads = DEF_THREADS, i = 2;
  FILE *I = NULL, *O = NULL;
  long double probSpawn = DEF_PROB_NEW, probDie = DEF_PROB_DIE;
#ifdef DEBUG
  mtrace();
#endif

  /* Parse program arguments. */
  while((opt = getopt(argc, argv, VALID_ARGS)) != -1){
    switch(opt){
      case('D'):{
        if(argv[i]){
          if(!VALID_PROB(probDie, probSpawn)){
            probDie = 1.0 - probSpawn;
	  }
	  ++i;
	}
	break;
      }
      case('N'):{
      	if(argv[i]){
	  if(!VALID_PROB(probSpawn, probDie)){
	    probSpawn = 1.0 - probDie;
	  }
	  ++i;
	}
	break;
      }
      case('I'):{
	if(argv[i]){
          nIterations = atoi(argv[i]);
	  ++i;
	}
	break;
      }
      case('G'):{
	useGL = 1;
	break;
      }
      case('H'):{
        if(argv[i]){
          maxHeight = atoi(argv[i]);
	}
	++i;
        break;
      }
      case('W'):{
        if(argv[i]){
	  maxWidth = atoi(argv[i]);
	}
	++i;
	break;
      }
      case('T'):{
	if(argv[i]){
	  nThreads = atoi(argv[i]);
	}
	++i;
	break;
      }
      case('S'):{
	if(argv[i]){
	  nSenders = atoi(argv[i]);
	}
	++i;
	break;
      }
      case('R'):{
        if(argv[i]){
	  nRecievers = atoi(argv[i]);
	}
	++i;
	break;
      }
      case('M'):{
        if(argv[i]){
	  model = atoi(argv[i]);
	}
	++i;
	break;
      }
      case('F'):{
	if(argv[i]){
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
	if(argv[i]){
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
  
  initModel(maxWidth, maxHeight, model, nRecievers, nSenders, nThreads, I, useGL, probSpawn, probDie);
  
  modelLoop(O, nIterations);

  stopModel();

  if(fileo){
    (void)fclose(O);
  }
  if(filei){
    (void)fclose(I);
  }

  return EXIT_SUCCESS;
}
