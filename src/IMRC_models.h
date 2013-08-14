#ifndef _IMRC_MODELS_
#define _IMRC_MODELS_

#include "IMRC_types.h"

void spawnTransmitters(const unsigned int maxW, const unsigned int maxH);
void spawnRecievers(const unsigned int maxW, const unsigned int maxH);
float *prepareSilencing(unsigned int W, unsigned int H);
void stopModel(void);
void modelLoop(FILE *O, int steps);
void initModel(unsigned int W, unsigned int H, unsigned int model, unsigned int nRecievers, unsigned int nSenders, unsigned int nThreads, FILE *I, unsigned int useGL);
#endif
