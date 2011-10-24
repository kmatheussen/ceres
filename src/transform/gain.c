
#include "../transform.h"


struct Vars{
  struct FFTSound *fftsound;

  double vol;
  bool funcsel;
};


void Gain(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  struct Vars *sv=(struct Vars*)pointer;

  long point;
  int i, j, ch;

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=area_start*fftsound->numchannels + ch*fftsound->numchannels*fftsound->horiz_num;
    for (i=area_start; i<area_end; i++) {
      double val;
      if (sv->funcsel){
	val=getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
      }else{
	val=sv->vol;
      }
      for (j=areaf1; j<areaf2; j++){
        MEGAMP_PUT_MUL(point+j,val);
      }
      point+=fftsound->numchannels;

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }

 end:
  return;
}

void create_transform_gain(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Gain",0,240);

  GUI_MakeInputVarLog(sm,"Gain factor:",&sv->vol,0.0,300.0,0);
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Gain",&sv->funcsel,false,1,0.0f,1.0f);

  GUI_CloseMenuButtonOkCancelPreview(sm,Gain,true);

}
