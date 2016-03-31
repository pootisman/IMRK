#include <stdio.h>
#include <sys/time.h>
#include "IMRC_pretty_output.h"

struct timeval tVal;
struct timeval cVal;
struct timezone tz;

void initStart(){
  (void)gettimeofday(&tVal, NULL); 
}

void printd(const char *str, const char *pFName, unsigned int LINE){
#ifdef DEBUG
  (void)gettimeofday(&cVal, NULL);
  if(str == NULL){
    printe("IMPOSSIBUL!", NULL, 0);
    return;
  }else if(pFName != NULL){
    (void)printf("\033[32;1mDEBUG[%ld]: %s in %s at %d.\033[0m\n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), str, pFName, LINE);
    (void)fflush(stdout);
  }else{
    (void)printf("\033[32;1mDEBUG[%ld]: %s.\033[0m \n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), str);
    (void)fflush(stdout);
  }
#endif
}

void printe(const char *str, const char *pFName, unsigned int LINE){
  (void)gettimeofday(&cVal, NULL);
  if(str == NULL){
    printe("IMPOSSIBUL!", NULL, 0);
    return;
  }else if(pFName != NULL){
    (void)printf("\033[31;1mERROR[%ld]: %s in %s at %d.\033[0m\n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), str, pFName, LINE);
    (void)fflush(stdout);
  }else{
    (void)printf("\033[31;1mERROR[%ld]: %s.\033[0m \n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), str);
    (void)fflush(stdout);
  }
}

void printw(const char *str, const char *pFName, unsigned int LINE){
   (void)gettimeofday(&cVal, NULL);
   if(str == NULL){
    printe("IMPOSSIBUL!", NULL, 0);
    return;
  }else if(pFName != NULL){
    (void)printf("\033[33;1mWARNING[%ld]: %s in %s at %d.\033[0m\n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), str, pFName, LINE);
    (void)fflush(stdout);
  }else{
    (void)printf("\033[33;1mWARNING[%ld]: %s.\033[0m \n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), str);
    (void)fflush(stdout);
  }
}

void printdt(const char *str, const char *pFName, unsigned int LINE, unsigned int threadNum){
#ifdef DEBUG
  (void)gettimeofday(&cVal, NULL);
  if(str == NULL){
    printe("IMPOSSIBUL!", NULL, 0);
    return;
  }else if(pFName != NULL){
    (void)printf("\033[32;1mDEBUG[%ld][%d]: %s in %s at %d.\033[0m\n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), threadNum, str, pFName, LINE);
    (void)fflush(stdout);
  }else{
    (void)printf("\033[32;1mDEBUG[%ld][%d]: %s.\033[0m \n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), threadNum, str);
    (void)fflush(stdout);
  }
#endif
}

void printet(const char *str, const char *pFName, unsigned int LINE, unsigned int threadNum){
  (void)gettimeofday(&cVal, NULL);
  if(str == NULL){
    printe("IMPOSSIBUL!", NULL, 0);
    return;
  }else if(pFName != NULL){
    (void)printf("\033[31;1mERROR[%ld][%d]: %s in %s at %d.\033[0m\n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), threadNum, str, pFName, LINE);
    (void)fflush(stdout);
  }else{
    (void)printf("\033[31;1mERROR[%ld][%d]: %s.\033[0m \n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), threadNum, str);
    (void)fflush(stdout);
  }
}

void printwt(const char *str, const char *pFName, unsigned int LINE, unsigned int threadNum){
   (void)gettimeofday(&cVal, NULL);
   if(str == NULL){
    printe("IMPOSSIBUL!", NULL, 0);
    return;
  }else if(pFName != NULL){
    (void)printf("\033[33;1mWARNING[%ld][%d]: %s in %s at %d.\033[0m\n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), threadNum, str, pFName, LINE);
    (void)fflush(stdout);
  }else{
    (void)printf("\033[33;1mWARNING[%ld][%d]: %s.\033[0m \n", (unsigned long int)(cVal.tv_usec - tVal.tv_usec), threadNum, str);
    (void)fflush(stdout);
  }
}
