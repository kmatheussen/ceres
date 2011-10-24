
#include "../transform.h"

void Trpeaks(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  long point;
  int i, j, keep, ch;
  double tempamp[fftsound->numchannels];

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=areat1*fftsound->numchannels+ch*fftsound->horiz_num*fftsound->numchannels;
    for (i=area_start; i<area_end; i++) {
      for (j=areaf1; j<areaf2; j++)
	tempamp[j]=MEGAMP_GET(point+j);
      for (j=areaf1; j<areaf2-1; j++) {
        keep=0;
	if (j==areaf1){
	  keep=1;
	}else{
          if ((tempamp[j-1]<tempamp[j]) && (tempamp[j+1]<tempamp[j]))
            keep=1;
        }
        if ((tempamp[j]<=0.000001) || (MEGFREQ_GET(point+j)<20.) || (keep==0)){
          MEGAMP_PUT(point+j,0.);
	}
      }
      point += fftsound->numchannels;

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }

 end:
  return;
}

void create_transform_trpeaks(struct FFTSound *fftsound){
  struct VarInput *sm;

  sm=GUI_MakeMenuButton(fftsound,NULL,transMenu,"Keep peaks only",0,200);

  GUI_CloseMenuButtonOkCancelPreview(sm,Trpeaks,true);

}

