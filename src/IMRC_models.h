#ifndef _IMRC_MODELS_
#define _IMRC_MODELS_

#include "IMRC_types.h"

void calcPower(unsigned int W, int Threads, unsigned int model);
void spawnTransmitters(const unsigned int maxW, const unsigned int maxH);
void spawnRecievers(const unsigned int maxW, const unsigned int maxH);
float *prepareSilencing(unsigned int W, unsigned int H);
void stopModel(void);

#endif
