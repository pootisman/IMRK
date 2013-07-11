#ifndef _IMRC_AUX_
#define _IMRC_AUX_

#include "IMRC_types.h"
void readFromFile(RECIEVER **pRecievers, SENDER **pSenders, unsigned int *nRecievers, unsigned int *nSenders, FILE *input);
void dumpToFile( const RECIEVER *pRecievers, const unsigned int nRecievers, const FILE *output);
void initRand(void);

#endif
