#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include "IMRC_gl.h"
#include "IMRC_types.h"

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

typedef struct CLRVCTR{
  float R,G,B;
}CLRVCTR;

extern long double *gA, percentY, percentX, maxWidthNow, maxHeightNow, maxDistNow;
extern unsigned int nRecieversNow, nSendersNow, gASize;
extern unsigned char lineWidth, spotSize, runningNow;
extern RECIEVER *pRecieversNow;
extern SENDER *pSendersNow;

GLFWwindow *pWView = NULL;

#ifndef DEBUG

/* Prints(Draws) one specified digit. */
inline void drawNumber(unsigned char digit, long double x, long double y){
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

/* Prints(Draws) the whole number to openGL screen. */
inline void printNumber(unsigned int number, long double x, long double y){
  char buffer[32] = {""};
  unsigned int i = 0;

  (void)sprintf(&(buffer[0]), "%d", number);

  i = strlen(&(buffer[0]));

  for(; i > 0; i--){
    drawNumber(buffer[i-1] - 48, x + 8*i*percentX/10.0, y);
  }
}

/* Calculate color of the reciever depending on the SNR */
inline CLRVCTR calcColor(float dB){
  float a, Vdec, Hi, Rs, Gs, Bs;
  CLRVCTR result;

  Hi = floor((120.0*((dB + 70.0)/140.0))/60.0);
 
  a = 100.0*((unsigned int)round(120.0*((dB + 70.0)/140.0))%60)/60.0;

  Vdec = 100.0 - a;

  if(Hi == 0.0){
    Rs = 100.0;
    Gs = a;
    Bs = 0.0;
  }else if(Hi == 1.0){
    Rs = Vdec;
    Gs = 100.0;
    Bs = 0.0;
  }else if(Hi == 2.0){
    Rs = 0.0;
    Gs = 100.0;
    Bs = a;
  }else if(Hi == 3.0){
    Rs = 0.0;
    Gs = Vdec;
    Bs = 100.0;
  }else if(Hi == 4.0){
    Rs = a;
    Gs = 0.0;
    Bs = 100.0;
  }else{
    Rs = 100.0;
    Gs = 0.0;
    Bs = Vdec;
  }

  Rs /= 100.0;
  Gs /= 100.0;
  Bs /= 100.0;

  result.R = Rs;
  result.G = Gs;
  result.B = Bs;

#ifdef DEBUG
  if(result.R == 1.0){
    (void)printf("Bad signal %Lf with color [%f, %f, %f]", dB, Rs, Gs, Bs);
  }
#endif

  return result;
}

#else

/* Prints(Draws) one specific digit. */
void drawDigit(unsigned char digit, long double x, long double y){
  unsigned int i = 0;

  if(digit > 10 || digit < 0){
    (void)printf("Digit %d too big, ignoring.\n", digit);
    return;
  }

  (void)printf("DEBUG: Drawing number %d at %Lf, %Lf.\n", digit, x, y);

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

/* Prints(Draws) the whole number to openGL screen. */
void printNumber(unsigned int number, long double x, long double y){
  char buffer[32] = {""};
  unsigned int i = 0;

  (void)sprintf(&(buffer[0]), "%d", number);

  i = strlen(&(buffer[0]));

  for(; i > 0; i--){
    drawDigit(buffer[i-1] - 48, x + 8*i*percentX/10.0, y);
  }
}

/* Calculate color of the reciever depending on the SNR */
CLRVCTR calcColor(float dB){
  float a, Vdec, Hi, Rs, Gs, Bs;
  CLRVCTR result;

  Hi = floor((120.0*((dB + 70.0)/140.0))/60.0);
 
  a = 100.0*((unsigned int)round(120.0*((dB + 70.0)/140.0))%60)/60.0;

  Vdec = 100.0 - a;

  if(Hi == 0.0){
    Rs = 100.0;
    Gs = a;
    Bs = 0.0;
  }else if(Hi == 1.0){
    Rs = Vdec;
    Gs = 100.0;
    Bs = 0.0;
  }else if(Hi == 2.0){
    Rs = 0.0;
    Gs = 100.0;
    Bs = a;
  }else if(Hi == 3.0){
    Rs = 0.0;
    Gs = Vdec;
    Bs = 100.0;
  }else if(Hi == 4.0){
    Rs = a;
    Gs = 0.0;
    Bs = 100.0;
  }else{
    Rs = 100.0;
    Gs = 0.0;
    Bs = Vdec;
  }

  Rs /= 100.0;
  Gs /= 100.0;
  Bs /= 100.0;

  result.R = Rs;
  result.G = Gs;
  result.B = Bs;

  return result;
}

#endif

/* Render the situation */
void render(void){
  unsigned int i = 0;
  RECIEVERS_LLIST *temp = NULL;
  SENDER *pTempS = pSendersNow;
  RECIEVER *pTempR = pRecieversNow;
  CLRVCTR colorVect;

  if(!pSendersNow || !pRecieversNow || nRecieversNow == 0 || nSendersNow == 0){
    (void)puts("Error, initialise model first!");
    return;
  }

  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(0.0, 0.0, 1.0);
  glPointSize(spotSize);
  
  glBegin(GL_POINTS);
  for(;pTempS; pTempS = pTempS->pNext){
    glVertex2f(pTempS->x, pTempS->y);
  }
  glEnd();

  pTempS = pSendersNow;

  for(i = 0; pTempS; pTempS = pTempS->pNext){
    printNumber(i, pTempS->x, pTempS->y);
    ++i;
  }

  glBegin(GL_POINTS);
  for(i = 0; pTempR; pTempR = pTempR->pNext, i++){
    colorVect = calcColor((float)pTempR->SNRLin);
    glColor3f(colorVect.R, colorVect.G, colorVect.B);
    glVertex2f(pTempR->x, pTempR->y);
    printNumber(i, pTempR->x, pTempR->y);
  }
  glEnd();

  pTempR = pRecieversNow;

  glColor3f(0.4, 0.4, 0.4);
  glLineWidth(lineWidth);

  pTempS = pSendersNow;

  glBegin(GL_LINES);
  for(;pTempS; pTempS = pTempS->pNext){
    temp = pTempS->pRecepients;
    for(;temp; temp = temp->pNext){
      glVertex2f(pTempS->x, pTempS->y);
      glVertex2f(temp->pTarget->x, temp->pTarget->y);
    }
  }
  glEnd();

  pTempS = pSendersNow;
  pTempR = pRecieversNow;

  glColor3f(0.0, 0.0, 1.0);
  glBegin(GL_LINES);
  for(; pTempS; pTempS = pTempS->pNext){
    glVertex2f(pTempS->x, pTempS->y);
    glVertex2f(pTempS->x, pTempS->y + percentY);

    glVertex2f(pTempS->x, pTempS->y + percentY/1.5);
    glVertex2f(pTempS->x - percentX/3.5, pTempS->y + percentY);    
    glVertex2f(pTempS->x, pTempS->y + percentY/1.5);
    glVertex2f(pTempS->x + percentX/3.5, pTempS->y + percentY);
    
    glVertex2f(pTempS->x,(long double)pTempS->y + percentY/3.0);
    glVertex2f((long double)pTempS->x - percentX/3.0, pTempS->y);
    glVertex2f(pTempS->x,(long double)pTempS->y + percentY/3.0);
    glVertex2f((long double)pTempS->x + percentX/3.0, pTempS->y);
    
  }
  glEnd();

  glBegin(GL_LINES);
  for(; pTempR; pTempR = pTempR->pNext){
    colorVect = calcColor((float)pTempR->SNRLin);
    glColor3f(colorVect.R, colorVect.G, colorVect.B);
    glVertex2f(pTempR->x, pTempR->y);
    glVertex2f(pTempR->x, pTempR->y + percentY);

    glVertex2f(pTempR->x, pTempR->y + percentY/1.5);
    glVertex2f(pTempR->x + percentX/3.5, pTempR->y + percentY);
    glVertex2f(pTempR->x, pTempR->y + percentY/1.5);
    glVertex2f(pTempR->x - percentX/3.5, pTempR->y + percentY);

    glVertex2f(pTempR->x, pTempR->y);
    glVertex2f(pTempR->x - percentX/3.0, pTempR->y + percentY/3.0);    
    glVertex2f(pTempR->x, pTempR->y);
    glVertex2f(pTempR->x + percentX/3.0, pTempR->y + percentY/3.0);
  }
  glEnd();

  glBegin(GL_LINES);
  for(i = 0; i < 120; i++){
    colorVect = calcColor(i - 60.0);
    glColor3f(colorVect.R, colorVect.G, colorVect.B);
    glVertex2f(maxWidthNow + 0.025*maxWidthNow, (float)i/(float)120.0*maxHeightNow);
    glVertex2f(maxWidthNow + 0.05*maxWidthNow, (float)i/(float)120.0*maxHeightNow);
  }
  glEnd();

  glFinish();
  runningNow = !glfwWindowShouldClose(pWView);
  glfwSwapBuffers(pWView);
  glfwPollEvents();
}

void resizeCallBk(GLFWwindow *wind, int w, int h){
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void initGraphics(){
  percentX = maxWidthNow/100.0;
  percentY = maxHeightNow/100.0;

  glfwInit();
  if(!(pWView = glfwCreateWindow(640, 480, "IMRC", NULL, NULL))){
    (void)puts("Error while initializing graphics!");
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(pWView);
  glfwSetWindowSizeCallback(pWView, resizeCallBk);

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, maxWidthNow + 0.05*maxWidthNow, 0.0, maxHeightNow, -1.0, 1.0);
#ifdef DEBUG
  (void)puts("DEBUG: Graphics initialised.");
#endif
}

void killWindow(void){
  if(!pWView){
    (void)puts("Error, no window to stop.");
    return;
  }

  glfwDestroyWindow(pWView);
  glfwTerminate();
#ifdef DEBUG
  (void)puts("DEBUG: Stopped window.");
#endif
}
