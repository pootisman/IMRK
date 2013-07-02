#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <math.h>

#ifdef DEBUG
#include <mtrace.h>
#endif

#define VALID_ARGS "W:H:R:S:F:O:T:"
#define DEF_WIDTH 255
#define DEF_HEIGHT 255
#define DEF_THREADS 1
#define DEF_SENDERS 2
#define DEF_RECIEVERS 2

typedef struct{
  double x, y; /* Position */
  double signal; /* Total power of useful signal */
  double waste; /* Total power of bad signals */
  double SNRLin; /* SNR in linear scale */
  unsigned int nTrans; /* Number of useful transmitters */
}RECIEVER;

typedef struct{
  double x, y; /* Position */
  double power; /* Strength of the transmitter */
  RECIEVER *pRecepient; /* The guy who will recieve data from us */
}SENDER;

/* Gaussian distribution generator, Box-Muller method */
inline double genGauss(void){
  double U1 = 0.0f, U2 = 0.0f, V1 = 0.0f, V2 = 0.0f, S = 0.0f;

  do{
    U1= fabs((double)rand()/(double)RAND_MAX);            /* U1=[0    ,1] */
    U2= fabs((double)rand()/(double)RAND_MAX);            /* U2=[0    ,1] */
    V1= 2.0f * U1 - 1.0f;            /* V1=[-1,1] */
    V2= 2.0f * U2 - 1.0f;           /* V2=[-1,1] */
    S = V1 * V1 + V2 * V2;
  }
  while(S >= 1.0f);
  
  return sqrt(-1.0f * log(S) / S) * V1;
}

/* Calculate distance with Pythagoras theorem */
inline double distance(const RECIEVER *pRecvr,const SENDER *pSender){
  return sqrt((pRecvr->x - pSender->x)*(pRecvr->x - pSender->x) + (pRecvr->y - pSender->y)*(pRecvr->y - pSender->y));
}

/* Calculate the power for the range of recievers */
inline void calcPower(RECIEVER *pRecvr, const SENDER *pSender, const unsigned int nSends, const unsigned int nRecvs, const double amp){
  unsigned int i = 0, j = 0;
  
  if(!pRecvr || !pSender){
    (void)puts("Error, NULL provided!");
    return;
  }

  for(j = 0; j < nRecvs; j++){
    for(i = 0; i < nSends; i++){
      if((pSender + i)->pRecepient == (pRecvr + j)){
        (pRecvr + j)->signal += (pSender + i)->power * (100.0)/(distance((pRecvr + j), (pSender + i))*distance((pRecvr + j), (pSender + i))*distance((pRecvr + j), (pSender + i))*distance((pRecvr + j), (pSender + i))*distance((pRecvr + j), (pSender + i))) + genGauss()*amp;
      }else{
        (pRecvr + j)->waste += (pSender + i)->power * (100.0)/(distance((pRecvr + j), (pSender + i))*distance((pRecvr + j), (pSender + i))*distance((pRecvr + j), (pSender + i))*distance((pRecvr + j), (pSender + i))*distance((pRecvr + j), (pSender + i))) + genGauss()*amp;
      }
    }
    (pRecvr + j)->SNRLin = (pRecvr + j)->signal/(pRecvr + j)->waste;
  }
}

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
  (pRecList + 1)->x = 16;
  (pRecList + 1)->y = 0;

  pSendList->x = 0;
  pSendList->y = 16;
  pSendList->power = 1;
  pSendList->pRecepient = (pRecList);

  (pSendList + 1)->x = 0;
  (pSendList + 1)->y = 0;
  (pSendList + 1)->power = 1;
  (pSendList + 1)->pRecepient = (pRecList + 1);

  calcPower(pRecList, pSendList, nSenders, nRecievers, 0);

  (void)printf("LSNR for first Node: %f\nLSNR for second Node: %f\n", (pRecList)->SNRLin, (pRecList + 1)->SNRLin);

  (void)free(pRecList);
  (void)free(pSendList);

  return EXIT_SUCCESS;
}
