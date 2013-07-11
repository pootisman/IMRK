#ifndef _IRMC_TYPES_
#define _IRMC_TYPES_

typedef struct{
  float x, y; /* Position */
  float signal; /* Total power of useful signal */
  float waste; /* Total power of bad signals */
  float SNRLin; /* SNR in linear scale */
}RECIEVER;

typedef struct RECIEVERS_LLIST{
  struct RECIEVERS_LLIST *pNext; /* Linked list of the recievers */
  RECIEVER *pTarget; /* The reciever that will be connected to us */
}RECIEVERS_LLIST;

typedef struct{
  float x, y; /* Position */
  float power; /* Strength of the transmitter */
  RECIEVERS_LLIST *pRecepients; /* Guys who will recieve data from us */
  unsigned int nRecepients;/* Number of recepients for this node */
}SENDER;

#endif
