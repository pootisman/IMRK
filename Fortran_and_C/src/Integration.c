#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[]){
  float start = 0, stop = 50, step = 0.00001;

  float funcField[(unsigned int)round((stop - start)/step)];
  float sumRes = 0.0, x = 0.0;
  

//  (void)printf("Integrating x^2 from %f to %f with step %f.\n", start, stop, step);

  while(x < stop){
    sumRes += (x*x + (x+step)*(x+step))/2.0 * step;
    x += step;
  }

//  (void)printf("The result is %f.\n", sumRes);

  return EXIT_SUCCESS;

}
