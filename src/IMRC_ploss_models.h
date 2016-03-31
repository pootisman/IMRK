/*
 * =====================================================================================
 *
 *       Filename:  IMRC_ploss_models.h
 *
 *    Description:  Pathloss models descriptions, these models are quite approximate
 *
 *        Version:  1.0
 *        Created:  07.07.2014 13:35:50
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aleksei Ponomarenko-Timofeev (), alexeyponomarenko(at)gmail(dot)com
 *   Organization:  SELF
 *
 * =====================================================================================
 */
#ifndef _IMRC_PLOSS_
#define _IMRC_PLOSS_
#include "IMRC_types.h"

float *prepareSilencing(unsigned int W, unsigned int H);
float distance_euclid(const RECIEVER *pRecvr, const SENDER *pSender);
float power_simple(RECIEVER *pRecvr, const SENDER *pSender, const float amp);
float power_complex(RECIEVER *pRecvr, const SENDER *pSender, const float Ht, const float Hr, const float freq);
float power_urban_simple(RECIEVER *pRecvr, const SENDER *pSender);
float power_urban_complex(RECIEVER *pRecvr, const SENDER *pSender);

#endif
