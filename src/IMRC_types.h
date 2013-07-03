#ifndef _IRMC_TYPES_
#define _IRMC_TYPES_

typedef struct{
  float x, y; /* Position */
  float signal; /* Total power of useful signal */
  float waste; /* Total power of bad signals */
  float SNRLin; /* SNR in linear scale */
  unsigned int nTrans; /* Number of useful transmitters */
}RECIEVER;

typedef struct{
  float x, y; /* Position */
  float power; /* Strength of the transmitter */
  RECIEVER *pRecepient; /* The guy who will recieve data from us */
}SENDER;

#endif
