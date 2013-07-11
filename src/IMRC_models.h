#ifndef _IMRC_MODELS_
#define _IMRC_MODELS_

#include "IMRC_types.h"

void calcPower(RECIEVER *pRecvrs, const SENDER *pSenders, const unsigned int nSends, const unsigned int nRecvs, unsigned int W, int Threads, unsigned int model);
void spawnTransmitters(SENDER *pSenders, RECIEVER *pRecievers, const unsigned int nSenders, const unsigned int nRecievers, const unsigned int maxW, const unsigned int maxH);
void spawnRecievers(RECIEVER *pRecievers, const unsigned int nRecievers, const unsigned int maxW, const unsigned int maxH);
float *prepareSilencing(unsigned int W, unsigned int H);
void stopModel(void);

#endif
