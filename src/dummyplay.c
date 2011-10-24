#include "ceres.h"

/*

*/


Boolean InitPlay(void){
  printf("No real soundlib compiled into Ceres. No play.\n");
  return FALSE;
}

void *OpenPlay(struct FFTSound *fftsound){

  return NULL;
}

void WritePlay(
	       struct FFTSound *fftsound,
	       void *port,double **buffer,int size
	       ){
}


void ClosePlay(void *port){
}
