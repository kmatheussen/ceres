#include "ceres.h"
#include "play.h"

#include <sndlib.h>


/* Only call once per process. */
Boolean InitPlay(void){
  //printf("sndlib init: %d %d\n",mus_sound_initialize(),MUS_NO_ERROR);
  mus_sound_initialize();
  return TRUE;
}

struct Sndplay{
  int outport;
};

void *OpenPlay(struct FFTSound *fftsound){
  struct Sndplay *sndplay;
  sndplay=malloc(sizeof(struct Sndplay));
  sndplay->outport = mus_audio_open_output(MUS_AUDIO_DEFAULT, fftsound->R, fftsound->samps_per_frame, MUS_COMPATIBLE_FORMAT, 1024 * fftsound->samps_per_frame);

  if(sndplay->outport==-1){
    free(sndplay);
    fprintf(stderr,"Could not open sndlib outport.\n");
    return NULL;
  }

  return sndplay;
}

void WritePlay(
		      struct FFTSound *fftsound,
		      void *port,double **samples,int num_samples
		      )
{
  struct Sndplay *sndplay=(struct Sndplay *)port;
  int i,ch;
  int16_t framebuff[fftsound->Nw*2*MAX_SAMPS_PER_FRAME];

  for (i=0; i<num_samples; i++) {
    for (ch=0; ch<fftsound->samps_per_frame; ch++)
      *(framebuff+i*fftsound->samps_per_frame+ch)=(short)(samples[ch][i]*32767.);
  }
  
  mus_audio_write(sndplay->outport,(char*)framebuff,num_samples*2*fftsound->samps_per_frame);

}


void ClosePlay(void *port){
  struct Sndplay *sndplay=(struct Sndplay *)port;
  mus_audio_close(sndplay->outport);
  free(sndplay);
}


