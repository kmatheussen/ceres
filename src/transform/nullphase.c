
#include "../transform.h"

void Nullphase(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{

  long point;
  int i, j, ch;

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=area_start*fftsound->numchannels+ch*fftsound->numchannels*fftsound->horiz_num;  
    for (i=area_start; i<area_end; i++) {
      for (j=areaf1; j<areaf2; j++){
        MEGFREQ_PUT(point+j,j*fftsound->binfreq);
      }
      point+=fftsound->numchannels;

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }
 end:
  return;
}

void create_transform_nullphase(struct FFTSound *fftsound){
  struct VarInput *sm;

  sm=GUI_MakeMenuButton(fftsound,NULL,transMenu,"Null phase",0,200);

  GUI_CloseMenuButtonOkCancelPreview(sm,Nullphase,true);

}

