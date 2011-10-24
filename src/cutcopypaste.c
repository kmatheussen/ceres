
#include "ceres.h"
#include "cutcopypaste.h"
#include "cachememhandler.h"
#include "clip.h"

struct Clip *CCP_clip=NULL;

bool CCP_copy(struct FFTSound *fftsound,int area_start,int area_end){
  if(CCP_clip!=NULL) CLIP_delete(CCP_clip);
  CCP_clip=CLIP_new(fftsound,area_start,area_end,areaf1,areaf2);
  if(CCP_clip==NULL) return false;
  return true;
}


void CCP_cut(struct FFTSound *fftsound,int area_start,int area_end){
  long point;
  int i, j, ch;

  if(area_end<=area_start) return;
  if(CCP_copy(fftsound,area_start,area_end)==false) return;
  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=area_start*fftsound->numchannels + ch*fftsound->numchannels*fftsound->horiz_num;
    for (i=area_start; i<area_end; i++) {
      for (j=areaf1; j<areaf2; j++){
	MEGAMP_PUT(point+j,0.0f);
	MEGFREQ_PUT(point+j,0.0f);
      }
      point+=fftsound->numchannels;
    }
  }
}

void CCP_paste(struct FFTSound *fftsound,int area_start,int area_end){
  if(CCP_clip==NULL) return;

  CLIP_paste(CCP_clip,fftsound,area_start,area_end);
}
