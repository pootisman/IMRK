#ifndef _IRMC_TYPES_
#define _IRMC_TYPES_

#define LIN 1
#define PNT 2
#define END 0

#define RAND 0x07
#define NEAR 0x0F
#define MAXS 0x70

#define SNDR "S"
#define RCVR "R"

#define TRUE 0xFF
#define FALSE 0x00

typedef struct RECIEVER{
  float x, y; /* Position */
  float signal; /* Total power of useful signal */
  float waste; /* Total power of bad signals */
  float SNRLin; /* SNR in linear scale */
  struct RECIEVER *pNext, *pPrev; /* Linked list */
  struct SENDER *pOwner; /* Our transmitter */
  char recalc; /* Should we re-calculate power for this reciever */
}RECIEVER;

typedef struct RECIEVERS_LLIST{
  RECIEVER *pTarget;
  struct RECIEVERS_LLIST *pNext, *pPrev;
}RECIEVERS_LLIST;

typedef struct SENDER{
  float x, y; /* Position */
  float power; /* Strength of the transmitter */
  float freq; /* Transmitter frequency in Hz */
  RECIEVERS_LLIST *pRecepients; /* Guys who will recieve data from us */
  unsigned int nRecepients;/* Number of recepients for this node */
  struct SENDER *pNext, *pPrev; /* Linked list */
}SENDER;

typedef struct DIRANT{
  float startAngle, stopAngle; /* Starting and stopping angles. */
  char type; /* Type of the bound object. */
  void *obj; /* Object pointer. */
}DIRANT;

typedef struct ATTPAT{
  unsigned short numEntries; /* Number of attenuation coefficients */
  float *pAtten; /* Attenuation coefficients */
}ATTPAT;

/* Struct to store data on how to display numbers. */
typedef struct numElem{
  float xs,ys,xe,ye; /* Stop and start points */
  unsigned int type; /* Type of thing to draw */
}numElem;

/* Global variables of the model and representation */
#ifdef NOT_MAIN
float *gA = NULL, percentY = 0.0, percentX = 0.0, maxWidthNow = 0.0, maxHeightNow = 0.0, *modRecievers = NULL, probDieNow = 0.0, probSpawnNow = 0.0;
unsigned int nRecieversNow = 0, nSendersNow = 0, gASize = 0, useGraph = 0, randSeed = 0;
unsigned char lineWidth = 1, spotSize = 2, modelNow = 1, sendersChanged = 0, runningNow = 1, bindMode = RAND;
unsigned char nThreadsNow = 0, verification = 0;
RECIEVER *pRecieversNow = NULL;
SENDER *pSendersNow = NULL;
#endif
#endif
