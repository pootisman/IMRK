#ifndef _IMRC_GL_
#define _IMRC_GL_

#include "IMRC_types.h"

void initData(RECIEVER *pReciever, SENDER *pSender, unsigned int senders, unsigned int recievers);
void initGraphics(const unsigned int fps, const unsigned int xRes, const unsigned int yRes, int *argc, char *argv[], unsigned int maxW, unsigned int maxH);

#endif
