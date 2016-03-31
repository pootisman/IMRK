#ifndef _IMRC_PO_
#define _IMRC_PO_

#define EOUTBOUNDS "Index out of bounds."
#define ENULLPOINT "Got NULL pointer."
#define ENOMEMLEFT "Ran out of memory."

void initStart();
void printd(const char *str, const char *pFName, unsigned int LINE);
void printe(const char *str, const char *pFName, unsigned int LINE);
void printw(const char *str, const char *pFName, unsigned int LINE);

void printdt(const char *str, const char *pFName, unsigned int LINE, unsigned int threadNum);
void printet(const char *str, const char *pFName, unsigned int LINE, unsigned int threadNum);
void printwt(const char *str, const char *pFName, unsigned int LINE, unsigned int threadNum);

#endif
