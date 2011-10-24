
#include "../transform.h"


struct Vars{
  struct FFTSound *fftsound;

  double th;
  bool funcsel;
};


void Threshold(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  struct Vars *sv=(struct Vars*)pointer;

  double val, maxamp;
  long point;
  int i, j, ch;

  val=sv->th;

  maxamp=-1.;
  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=ch*fftsound->numchannels*fftsound->horiz_num;
    for (i=0; i<fftsound->horiz_num; i++) {
      for (j=0; j<fftsound->numchannels; j++)
        if (MEGAMP_GET(point+j)>maxamp){
	  maxamp=MEGAMP_GET(point+j);
	}
      point+=fftsound->numchannels;

      if(GUI_UpdateProgress(ctransform,0,i,fftsound->horiz_num)==false) goto end;  
    }
  }

  val*=maxamp;
  if (maxamp>0.) for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=areat1*fftsound->numchannels+ch*fftsound->numchannels*fftsound->horiz_num;
    for (i=area_start; i<area_end; i++) {
      if (sv->funcsel){
	val=getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
      }else{
	val=sv->th;
      }
      for (j=areaf1; j<areaf2; j++)
        if (MEGAMP_GET(point+j)<(val*maxamp)){
	  MEGAMP_PUT(point+j,0.);
	}
      point+=fftsound->numchannels;

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }

 end:
  return;
}

void create_transform_threshold(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Threshold",0,240);

  GUI_MakeInputVarDouble(sm,"Threshold (0-1):",&sv->th,0.0,1.0,0.1);
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Threshold",&sv->funcsel,false,1,0.0f,1.0f);

  GUI_CloseMenuButtonOkCancelPreview(sm,Threshold,true);

}

