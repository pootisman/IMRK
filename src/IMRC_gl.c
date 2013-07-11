#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include "IMRC_types.h"
#include "IMRC_gl.h"

#define LIN 1
#define PNT 2
#define END 0

float percentX = 0.0, percentY = 0.0;
unsigned int nRecievers = 0, nSenders = 0;
unsigned char lineWidth = 1, spotSize = 2;
RECIEVER *pRecievers = NULL;
SENDER *pSenders = NULL;

/* Struct to store data how to display numbers. */
typedef struct numElem{
  float xs,ys,xe,ye;
  unsigned int type;
}numElem;

/* Data for diplaying numbers. */
const numElem numbers[10][8] = {{{1,0,5,0,LIN}, {0,1,0,9,LIN}, {1,10,5,10,LIN}, {5,1,5,9,LIN}, {0,0,0,0,END}}, /* Zero */
			  {{0,0,6,0,LIN}, {2,1,2,10,LIN}, {2,9,0,7,LIN}, {0,0,0,0,END}}, /* One */
                          {{0,0,6,0,LIN}, {1,1,6,6, LIN}, {6,7,0,0,PNT}, {6,8,4,10,LIN}, {3,10,0,0,PNT}, {2,10,0,7,LIN}, {0,7,0,0,PNT}, {0,0,0,0,END}}, /* Two */
			  {{0,1,0,0,PNT}, {1,0,6,0,LIN}, {6,1,6,4,LIN}, {0,5,6,5,LIN}, {6,5,6,9,LIN}, {6,10,1,10,LIN}, {0,9,0,0,PNT}, {0,0,0,0,END}}, /* Three */
			  {{6,0,6,10,LIN}, {1,5,5,5,LIN}, {0,6,0,10,LIN}, {0,0,0,0,END}}, /* Four */
			  {{0,2,0,0,LIN}, {1,0,6,0,LIN}, {6,1,6,5,LIN}, {6,5,0,5,LIN}, {0,6,0,10,LIN}, {1,10,6,10,LIN}, {0,0,0,0,END}}, /* Five */
			  {{1,0,5,0,LIN}, {6,1,6,4,LIN}, {5,5,1,5,LIN}, {0,1,0,9,LIN}, {1,10,5,10,LIN}, {0,0,0,0,END}}, /* Six */
			  {{0,10,6,10,LIN}, {6,9,0,0,LIN}, {0,0,0,0,END}}, /* Seven */
			  {{0,1,0,9,LIN}, {6,1,6,9,LIN}, {1,10,5,10,LIN}, {1,0,5,0,LIN}, {1,5,5,5,LIN}, {0,0,0,0,END}}, /* Eight */
			  {{0,2,0,1,LIN}, {1,0,5,0,LIN}, {6,1,6,9,LIN}, {1,10,5,10,LIN}, {0,6,0,9,LIN}, {1,5,5,5,LIN}, {0,0,0,0,END}}, /* Nine */
			  };

#ifndef DEBUG
inline void drawNumber(unsigned char digit, float x, float y){
  unsigned int i = 0;

  if(digit > 10 || digit < 0){
    (void)printf("Digit %d too big, ignoring.\n", digit);
    return;
  }

  glBegin(GL_LINES);
  while(1){
    switch(numbers[digit][i].type){
      case(LIN):{
      	glVertex2f(x + numbers[digit][i].xs*percentX/10.0 + percentX, y + numbers[digit][i].ys*percentY/10.0);
	glVertex2f(x + numbers[digit][i].xe*percentX/10.0 + percentX, y + numbers[digit][i].ye*percentY/10.0);
	break;
      }
      case(PNT):{
      	glEnd();
	glBegin(GL_POINTS);
	glVertex2f(x + numbers[digit][i].xs*percentX/10.0 + percentX, y + numbers[digit][i].ys*percentY/10.0);
	glEnd();
	glBegin(GL_LINES);
	break;
      }
      case(END):{
        glEnd();
	return;
      }
    }
  i++;
  }
}

inline void printNumber(unsigned int number, float x, float y){
  char buffer[32] = {""};
  unsigned int i = 0;

  (void)sprintf(&(buffer[0]), "%d", number);

  i = strlen(&(buffer[0]));

  for(; i > 0; i--){
    drawNumber(buffer[i-1] - 48, x + 8*i*percentX/10.0, y);
  }
}
#else
void drawNumber(unsigned char digit, float x, float y){
  unsigned int i = 0;

  if(digit > 10 || digit < 0){
    (void)printf("Digit %d too big, ignoring.\n", digit);
    return;
  }

  (void)printf("Drawing number %d at %f, %f.\n", digit, x, y);

  glBegin(GL_LINES);
  while(1){
    switch(numbers[digit][i].type){
      case(LIN):{
      	glVertex2f(x + numbers[digit][i].xs*percentX/10.0 + percentX, y + numbers[digit][i].ys*percentY/10.0);
	glVertex2f(x + numbers[digit][i].xe*percentX/10.0 + percentX, y + numbers[digit][i].ye*percentY/10.0);
	break;
      }
      case(PNT):{
      	glEnd();
	glBegin(GL_POINTS);
	glVertex2f(x + numbers[digit][i].xs*percentX/10.0 + percentX, y + numbers[digit][i].ys*percentY/10.0);
	glEnd();
	glBegin(GL_LINES);
	break;
      }
      case(END):{
        glEnd();
	return;
      }
    }
  i++;
  }
}

void printNumber(unsigned int number, float x, float y){
  char buffer[32] = {""};
  unsigned int i = 0;

  (void)sprintf(&(buffer[0]), "%d", number);

  i = strlen(&(buffer[0]));

  for(; i > 0; i--){
    drawNumber(buffer[i-1] - 48, x + 8*i*percentX/10.0, y);
  }
}
#endif
void draw(void){
  unsigned int i = 0, j = 0;
  RECIEVERS_LLIST *temp = NULL;

  if(!pSenders || !pRecievers || nRecievers == 0 || nSenders == 0){
    (void)puts("Error, initialise graphics first!");
    return;
  }

  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(1.0, 0.0, 0.0);
  glPointSize(spotSize);
  
  glBegin(GL_POINTS);
  for(i = 0; i < nSenders; i++){
    glVertex2f((pSenders + i)->x, (pSenders + i)->y);
  }
  glEnd();

  for(i = 0; i < nSenders; i++){
    printNumber(i, (pSenders + i)->x, (pSenders + i)->y);
  }

  glColor3f(1.0, 1.0, 0.0);
  
  glBegin(GL_POINTS);
  for(i = 0; i < nRecievers; i++){
    glVertex2f((pRecievers + i)->x, (pRecievers + i)->y);
  }
  glEnd();

  for(i = 0; i < nRecievers; i++){
    printNumber(i, (pRecievers + i)->x, (pRecievers + i)->y);
  }


/*  printNumber(777,10,10);*/

  glColor3f(0.4, 0.4, 0.4);
  glLineWidth(lineWidth);

  glBegin(GL_LINES);
  for(i = 0; i < nSenders; i++){
    temp = (pSenders + i)->pRecepients;
    for(j = 0; j < (pSenders + i)->nRecepients; j++){
      glVertex2f((pSenders + i)->x, (pSenders + i)->y);
      glVertex2f(temp->pTarget->x, temp->pTarget->y);
      temp = temp->pNext;
    }
  }
  glEnd();

  glColor3f(1.0, 0.0, 0.0);
  glBegin(GL_LINES);
  for(i = 0; i < nSenders; i++){
    glVertex2f((pSenders + i)->x, (pSenders + i)->y);
    glVertex2f((pSenders + i)->x, (pSenders + i)->y + percentY);

    glVertex2f((pSenders + i)->x, (pSenders + i)->y + percentY/1.5);
    glVertex2f((pSenders + i)->x - percentX/3.5, (pSenders + i)->y + percentY);    
    glVertex2f((pSenders + i)->x, (pSenders + i)->y + percentY/1.5);
    glVertex2f((pSenders + i)->x + percentX/3.5, (pSenders + i)->y + percentY);
    
    glVertex2f((pSenders + i)->x,(float)(pSenders + i)->y + percentY/3.0);
    glVertex2f((float)(pSenders + i)->x - percentX/3.0, (pSenders + i)->y);
    glVertex2f((pSenders + i)->x,(float)(pSenders + i)->y + percentY/3.0);
    glVertex2f((float)(pSenders + i)->x + percentX/3.0, (pSenders + i)->y);
    
  }
  glEnd();

  glColor3f(1.0, 1.0, 0.0);
  glBegin(GL_LINES);
  for(i = 0; i < nRecievers; i++){
    glVertex2f((pRecievers + i)->x, (pRecievers + i)->y);
    glVertex2f((pRecievers + i)->x, (pRecievers + i)->y + percentY);

    glVertex2f((pRecievers + i)->x, (pRecievers + i)->y + percentY/1.5);
    glVertex2f((pRecievers + i)->x + percentX/3.5, (pRecievers + i)->y + percentY);
    glVertex2f((pRecievers + i)->x, (pRecievers + i)->y + percentY/1.5);
    glVertex2f((pRecievers + i)->x - percentX/3.5, (pRecievers + i)->y + percentY);

    glVertex2f((pRecievers + i)->x, (pRecievers + i)->y);
    glVertex2f((pRecievers + i)->x - percentX/3.0, (pRecievers + i)->y + percentY/3.0);    
    glVertex2f((pRecievers + i)->x, (pRecievers + i)->y);
    glVertex2f((pRecievers + i)->x + percentX/3.0, (pRecievers + i)->y + percentY/3.0);
  }
  glEnd();
  glFinish();
}

void kboard(unsigned char key, int x, int y){
  switch(key){
    case(27):{
      glutLeaveMainLoop();
      break;
    }
  }
}

void initData(RECIEVER *pReciever, SENDER *pSender, unsigned int senders, unsigned int recievers){
  pRecievers = pReciever;
  pSenders = pSender;
  nRecievers = recievers;
  nSenders = senders;
}

void initGraphics(int *argc, char *argv[], unsigned int maxW, unsigned int maxH){

  if(argc == NULL || argv == NULL){
    (void)puts("Error, can't init graphics.");
    return;
  }

  percentX = maxW/100.0;
  percentY = maxH/100.0;

  glutInit(argc, argv);
  glutInitDisplayMode( GLUT_RGB | GLUT_SINGLE );
  glutInitWindowSize(512,512);
  glutInitWindowPosition(50,50);
  glutCreateWindow("IMRC");
  glutFullScreen();
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, maxW, 0.0, maxH, -1.0, 1.0);
  glutDisplayFunc(draw);
  glutKeyboardFunc(kboard);
  glutMainLoop();
}
