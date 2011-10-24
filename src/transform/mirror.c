
#include "../transform.h"


struct Vars{
  struct FFTSound *fftsound;
  double mult_pr_horiz;

  double mval2;
  bool funcsel;
};


void Mirror(
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
  double mval, f;

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=area_start*fftsound->numchannels+ch*fftsound->horiz_num*fftsound->numchannels; 
    funcpt=1; 
    mval=sv->mval2;
    for (i=area_start; i<area_end; i++) {
      if (sv->funcsel) mval=fftsound->binfreq*getfuncval(fftsound,i+funcstartdiff);
      for (j=areaf1; j<areaf2; j++) {
        f=MEGFREQ_GET(point+j);
        f=mval-(f-mval);
        if (f<0.) f=0.;
        if (f<20.) f*=f/20.;
        MEGFREQ_PUT(point+j,f);
      }
      point+=fftsound->numchannels;
      mval*=sv->mult_pr_horiz;

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }

 end:
  rightbins(fftsound);

}

void create_transform_mirror(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Mirror",0,240);

  GUI_MakeInputVarFreq(sm,"Shift value (hz):",&sv->mval2,0.0,20000.0,500.0);
  GUI_MakeInputVarDouble_func(sm,"Multiplication pr. sec.:",GUI_generalMult_pr_horiz,0.0f,20.0,1.0);
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Shift",&sv->funcsel,false,1,-500.0f,500.0f);

  GUI_CloseMenuButtonOkCancelPreview(sm,Mirror,true);

}
