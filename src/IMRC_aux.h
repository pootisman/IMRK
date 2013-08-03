#ifndef _IMRC_AUX_
#define _IMRC_AUX_

#include "IMRC_types.h"
void readFromFile(FILE *input);
void dumpToFile(const FILE *output, unsigned int step);
void initRand(void);
RECIEVER *makeRcvrList(unsigned int nRecievers);
SENDER *makeSndrList(unsigned int nSenders);
void freeLists(void);
void addReciever(SENDER *pSender, unsigned int x, unsigned int y);
RECIEVER *rcvrAtIndex( unsigned int index);
SENDER *sndrAtIndex( unsigned int index);
void rmReciever(RECIEVER *pReciever);
void bindToReciever(RECIEVER *pReciever, SENDER *pSender);
void unbindReciever(RECIEVER *pReciever);
#endif
