#ifndef _IRMC_TYPES_
#define _IRMC_TYPES_

#define LIN 1
#define PNT 2
#define END 0

typedef struct RECIEVER{
  long double x, y; /* Position */
  long double signal; /* Total power of useful signal */
  long double waste; /* Total power of bad signals */
  long double SNRLin; /* SNR in linear scale */
  struct RECIEVER *pNext, *pPrev; /* Linked list */
  struct SENDER *pOwner; /* Our transmitter */
  char recalc; /* Should we re-calculate power for this reciever */
}RECIEVER;

typedef struct RECIEVERS_LLIST{
  RECIEVER *pTarget;
  struct RECIEVERS_LLIST *pNext, *pPrev;
}RECIEVERS_LLIST;

typedef struct SENDER{
  long double x, y; /* Position */
  long double power; /* Strength of the transmitter */
  long double freq; /* Transmitter frequency in Hz */
  RECIEVERS_LLIST *pRecepients; /* Guys who will recieve data from us */
  unsigned int nRecepients;/* Number of recepients for this node */
  struct SENDER *pNext, *pPrev; /* Linked list */
}SENDER;

/* Struct to store data how to display numbers. */
typedef struct numElem{
  long double xs,ys,xe,ye;
  unsigned int type;
}numElem;

/* Global variables of the model and representation */
#ifdef NOT_MAIN
long double *gA = NULL, percentY = 0.0, percentX = 0.0, maxWidthNow = 0.0, maxHeightNow = 0.0, *modRecievers = NULL, probDieNow = 0.0, probSpawnNow = 0.0;
unsigned int nRecieversNow = 0, nSendersNow = 0, gASize = 0, useGraph = 0, randSeed = 0;
unsigned char lineWidth = 1, spotSize = 2, modelNow = 1, sendersChanged = 0, runningNow = 1;
char nThreadsNow = 0;
RECIEVER *pRecieversNow = NULL;
SENDER *pSendersNow = NULL;
#endif
#endif
