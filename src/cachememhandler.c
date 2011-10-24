

#include "ceres.h"


void CH_clear(struct FFTSound *fftsound){
  fftsound->cachemem.pos=0;
}

int CH_getSize(struct FFTSound *fftsound){
  return fftsound->cachemem.size;
}

void CH_free(struct FFTSound *fftsound){
  free(fftsound->cachemem.mem);
  fftsound->cachemem.mem=NULL;
}

static void CH_create(struct FFTSound *fftsound){
  fftsound->cachemem.mem=malloc(sizeof(float)*fftsound->cachemem.size);
}

float *CH_malloc(struct FFTSound *fftsound,int size){
  float *ret;

  if(fftsound->cachemem.mem==NULL) CH_create(fftsound);

  ret=fftsound->cachemem.mem+fftsound->cachemem.pos;
  fftsound->cachemem.pos+=size;
  return ret;
}

float *CH_calloc(struct FFTSound *fftsound,int size){
  float *ret=CH_malloc(fftsound,size);
  memset(ret,0,size*sizeof(float));
  return ret;
}



