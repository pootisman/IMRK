#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include "IMRC_types.h"
#include "IMRC_gl.h"

float percentX = 0.0, percentY = 0.0;
unsigned int nRecievers = 0, nSenders = 0;
unsigned char lineWidth = 1, spotSize = 2;
RECIEVER *pRecievers = NULL;
SENDER *pSenders = NULL;

void draw(void){
  unsigned int i = 0;

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

  glColor3f(1.0, 1.0, 0.0);
  
  glBegin(GL_POINTS);
  for(i = 0; i < nRecievers; i++){
    glVertex2f((pRecievers + i)->x, (pRecievers + i)->y);
  }
  glEnd();

  glColor3f(0.4, 0.4, 0.4);
  glLineWidth(lineWidth);

  glBegin(GL_LINES);
  for(i = 0; i < nSenders; i++){
    glVertex2f((pSenders + i)->x, (pSenders + i)->y);
    glVertex2f((pSenders + i)->pRecepient->x, (pSenders + i)->pRecepient->y);
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
  for(i = 0; i < nSenders; i++){
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
