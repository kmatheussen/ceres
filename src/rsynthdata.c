
#include "ceres.h"


struct RSynthData *getRSynthData(struct FFTSound *fftsound){
  struct RSynthData *ret=malloc(sizeof(struct RSynthData));
  int lokke;
  int adding=0;

  int num_channels=fftsound->samps_per_frame;

  ret->num_channels=num_channels;

  ret->output=malloc(sizeof(double *)*num_channels);
  ret->lastphase=malloc(sizeof(double *)*num_channels);
  ret->lastfreq=malloc(sizeof(double *)*num_channels);
  ret->lastamp=malloc(sizeof(double *)*num_channels);
  ret->indx=malloc(sizeof(double *)*num_channels);

  ret->alldata=calloc(sizeof(double),8192*num_channels+(4097*num_channels* MAX_SAMPS_PER_FRAME ));

  for(lokke=0;lokke<num_channels;lokke++,adding+=8192){
    ret->output[lokke]=ret->alldata+adding;
  }

  for(lokke=0;lokke<num_channels;lokke++,adding+=4097){
    ret->lastphase[lokke]=ret->alldata+adding;
  }

  for(lokke=0;lokke<num_channels;lokke++,adding+=4097){
    ret->lastfreq[lokke]=ret->alldata+adding;
  }

  for(lokke=0;lokke<num_channels;lokke++,adding+=4097){
    ret->lastamp[lokke]=ret->alldata+adding;
  }

  for(lokke=0;lokke<num_channels;lokke++,adding+=4097){
    ret->indx[lokke]=ret->alldata+adding;
  }

  ret->input=calloc(sizeof(double),fftsound->Nw);
  ret->buffer=calloc(sizeof(double),fftsound->N);
  ret->channel=calloc(sizeof(double),fftsound->N+2);

  /*
  fvec(input,  fftsound->Nw);
  fvec(buffer,  fftsound->N);
  fvec(channel,  fftsound->N+2);
  */

  return ret;
}

void returnRSynthData(struct RSynthData *rsd){

  free(rsd->input);
  free(rsd->buffer);
  free(rsd->channel);

  free(rsd->alldata);

  free(rsd->indx);
  free(rsd->lastamp);
  free(rsd->lastfreq);
  free(rsd->lastphase);
  free(rsd->output);

  free(rsd);
}

