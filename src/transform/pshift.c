
#include "../transform.h"


struct Vars{
  struct FFTSound *fftsound;
  double mult_pr_horiz;

  double fact2;
  double reference;
  bool funcsel;
};


void Pshift(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  struct Vars *sv=(struct Vars*)pointer;

  int i, j, ch;
  long point;
  double fact, diff;

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=area_start*fftsound->numchannels+ch*fftsound->numchannels*fftsound->horiz_num;
    funcpt=1; fact=sv->fact2;
    for (i=area_start; i<area_end; i++) {
      if (sv->funcsel) fact=2.*getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
      for (j=areaf1; j<areaf2; j++) {
        diff=fact*(MEGFREQ_GET(point+j)-sv->reference) + sv->reference;
        MEGFREQ_PUT(point+j,diff);
	if(diff<20.0){
	  MEGAMP_PUT_MUL(point+j,diff/20.);
	}
      }
      point+=fftsound->numchannels;
      fact*=sv->mult_pr_horiz;

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }

 end:
  rightbins(fftsound);
  return;
}

void create_transform_pshift(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Pitch shift",0,240);

  GUI_MakeInputVarLog(sm,"Transposition factor:",&sv->fact2,0.0,10.0,1.0);
  GUI_MakeInputVarDouble_func(sm,"Multiplication pr. sec.:",GUI_generalMult_pr_horiz,0.0f,20.0,1.0);
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Tr. factor",&sv->funcsel,false,1,0.0f,2.0f);
  GUI_MakeInputVarFreq(sm,"Static frequency: ",&sv->reference,0.0,20000.0,0.0);

  GUI_CloseMenuButtonOkCancelPreview(sm,Pshift,true);

}
