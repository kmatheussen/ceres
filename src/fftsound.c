
#include "ceres.h"
#include "fftsound.h"



void newFFTSound(struct FFTSound *fftsound){
  
}


/* Does not fill in data. */
struct FFTSound *makeFFTSoundCopy(struct FFTSound *fftsound,int start,int end){
  struct FFTSound *ret=malloc(sizeof(struct FFTSound));

  memcpy(ret,fftsound,sizeof(struct FFTSound));

  ret->horiz_num=end-start;


  ret->megfreq.dac.s[0].mem=malloc(sizeof(float)*(ret->horiz_num*fftsound->total_channels));
  ret->megfreq.dac.s[0].start=0;
  ret->megfreq.dac.s[0].end=ret->horiz_num*fftsound->total_channels;

  ret->megamp.dac.s[0].mem=malloc(sizeof(float)*(ret->horiz_num*fftsound->total_channels));
  ret->megamp.dac.s[0].start=0;
  ret->megamp.dac.s[0].end=ret->horiz_num*fftsound->total_channels;

  ret->megamp.dac.file=NULL;

  ret->usemem=true;

  ret->dontfreeme=false;

  //  pthread_mutex_init(&ret->mutex,NULL); Not necesarry, because usemem==true.

  return ret;
}


/* Both from and to must have the same number of channels and samps_per_frame. */
void copyFFTSound(struct FFTSound *from,struct FFTSound *to,int fromstart){

  size_t len=min(from->horiz_num-fromstart,to->horiz_num);

#if 1

  int ch;
  int point_from;
  int point_to;

  for(ch=0;ch<to->samps_per_frame;ch++){
    point_from=fromstart*from->numchannels + ch*from->numchannels*from->horiz_num;
    point_to=ch*to->numchannels*to->horiz_num;

    DA_copy(
	    &from->megfreq,
	    &to->megfreq,
	    point_from,
	    point_to,
	    len*to->numchannels
	    );
  }
  for(ch=0;ch<to->samps_per_frame;ch++){
    point_from=fromstart*from->numchannels + ch*from->numchannels*from->horiz_num;
    point_to=ch*to->numchannels*to->horiz_num;
    DA_copy(
	    &from->megamp,
	    &to->megamp,
	    point_from,
	    point_to,
	    len*to->numchannels
	    );
  }

#else
  memcpy(to->megfreq,from->megfreq+(fromstart*fftsound->total_channels),sizeof(float)*len*fftsound->total_channels);
  memcpy(to->megamp,from->megamp+(fromstart*fftsound->total_channels),sizeof(float)*len*fftsound->total_channels);
#endif
}

void freeFFTSound(struct FFTSound *fftsound){

  free(fftsound->megamp.dac.s[0].mem);
  free(fftsound->megfreq.dac.s[0].mem);

  free(fftsound);
}

