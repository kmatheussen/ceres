
#include "../transform.h"


struct Vars{
  struct FFTSound *fftsound;
  double mult_pr_horiz;

  double shval2;
  bool funcsel;
};


void Sshift(
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
  double shval;

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point= area_start*fftsound->numchannels + ch*fftsound->horiz_num*fftsound->numchannels;
    funcpt=1;
    shval=sv->shval2;
    for (i=area_start; i<area_end; i++) {
      if (sv->funcsel) shval=1000.*(getfuncval(fftsound,i+funcstartdiff)/(double)theheight)-500.;
      for (j=areaf1; j<areaf2; j++) {
	float t1=MEGFREQ_GET(point+j)+shval;
	MEGFREQ_PUT(point+j,t1);
        if (t1<20.){
	  MEGAMP_PUT_MUL(point+j,t1/20.);
	}
      }
      point+=fftsound->numchannels;
      shval*=sv->mult_pr_horiz;

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;

    }
  }
 end:
  rightbins(fftsound);
}

void create_transform_sshift(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Spectrum shift",0,240);

  GUI_MakeInputVarDouble(sm,"Shift value (hz):",&sv->shval2,-1000.0,1000.0,50.0);
  GUI_MakeInputVarDouble_func(sm,"Multiplication pr. sec.:",GUI_generalMult_pr_horiz,0.0,20.0,1.0);
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Shift",&sv->funcsel,false,1,-500.0f,500.0f);

  GUI_CloseMenuButtonOkCancelPreview(sm,Sshift,true);

}



