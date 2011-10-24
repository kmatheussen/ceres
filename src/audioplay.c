#include "ceres.h"

#include <audio.h>

/*
  This code is losely based on code made for ceres3 by Stanko Juzbasic.

  Never tested, but it could be that it works (but probably not). -Kjetil Matheussen.
*/


struct Sgiplay{
  ALconfig out_c;
  ALport out_port;
};

Boolean InitPlay(void){
  return TRUE;
}

void *OpenPlay(struct FFTSound *fftsound){
  struct Sgiplay *sgiplay;

  sgiplay=malloc(sizeof(struct Sgiplay));

  sgiplay->out_c = alNewConfig();

  alSetQueueSize(sgiplay->out_c, _I);
  alSetSampFmt(sgiplay->out_c,AL_SAMPFMT_DOUBLE);
  alSetChannels(sgiplay->out_c, fftsound->samps_per_frame);
  sgiplay->out_port =  alOpenPort("Ceres Play", "w", sgiplay->out_c);
  if(!sgiplay->out_port){
    printf("Couldn't open output port: %s\n",alGetErrorString(oserror()));
    free(sgiplay);
    return NULL;
  }
  return sgiplay;
}

void WritePlay(
	       struct FFTSound *fftsound,
	       void *port,double **buffer,int size
	       ){
  struct Sgiplay *sgiplay=(struct Sgiplay *)port;
  alWriteFrames(sgiplay->out_port,  buffer, size);
}


void ClosePlay(void *port){
  struct Sgiplay *sgiplay=(struct Sgiplay *)port;
  type j;

  j = alClosePort(sgiplay->out_port);
  j = alFreeConfig(sgiplay->out_c);

  free(sgiplay);
}
