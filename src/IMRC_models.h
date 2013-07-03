#ifndef _IMRC_MODELS_
#define _IMRC_MODELS_

#include "IMRC_types.h"

float genGauss(void);
float power_simple(RECIEVER *pRecvr, const SENDER *pSender, const float amp);
void calcPower(RECIEVER *pRecvrs, const SENDER *pSenders, const unsigned int nSends, const unsigned int nRecvs, const float amp);
float distance_euclid(const RECIEVER *pRecvr, const SENDER *pSender);

#endif
