
#include "../transform.h"

void LTAS(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  int i, j, ch, start;
  double middleamp, middlefreq;

  DA_convertToChannel(fftsound);

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    start=ch*fftsound->horiz_num*fftsound->numchannels;
    for (j=0; j<fftsound->numchannels; j++) {
      middleamp=middlefreq=0.;
      for (i=0; i<fftsound->horiz_num; i++) {
        middleamp+=MEGAMP_GET_C(i*fftsound->numchannels+j+start);
	middlefreq+=MEGFREQ_GET_C(i*fftsound->numchannels+j+start);
      }
      middleamp/=fftsound->horiz_num;
      middlefreq/=fftsound->horiz_num;
      for (i=0; i<fftsound->horiz_num; i++) {
	MEGAMP_PUT_C(i*fftsound->numchannels+j+start,middleamp);
	MEGFREQ_PUT_C(i*fftsound->numchannels+j+start,middlefreq);
      }

      if(GUI_UpdateProgress(ctransform,0,j,fftsound->numchannels)==false) goto end;
    }

  }

 end:
  DA_convertToPoint(fftsound);  
  return;
}

void create_transform_ltas(struct FFTSound *fftsound){
  struct VarInput *sm;

  sm=GUI_MakeMenuButton(fftsound,NULL,transMenu,"Average",0,200);

  GUI_CloseMenuButtonOkCancelPreview(sm,LTAS,true);

}

