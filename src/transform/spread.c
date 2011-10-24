
#include "../transform.h"


struct Vars{
  struct FFTSound *fftsound;
  double mult_pr_horiz;

  double eval2;
  double mul;
  bool funcsel;
};


void Spread(
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
  double eval, f;

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=area_start*fftsound->numchannels+ch*fftsound->numchannels*fftsound->horiz_num;
    funcpt=1; eval=sv->eval2;
    for (i=area_start; i<area_end; i++) {
      if (sv->funcsel) eval=2.*getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
      for (j=areaf1; j<areaf2; j++) {
        f=MEGFREQ_GET(point+j);
        f*=pow(2., eval*((double)rand()/(RAND_MAX/2)-1.)*(1.-sv->mul+sv->mul*MEGAMP_GET(point+j)*10000.));
        if (f<0.) f=0.;
        if (f<20.) f*=f/20.;
        MEGFREQ_PUT(point+j,f);
      }
      point+=fftsound->numchannels;
      eval*=sv->mult_pr_horiz;

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }

 end:
  rightbins(fftsound);
}

void create_transform_spread(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Spread",0,240);

  GUI_MakeInputVarDouble(sm,"Random spread (0-2 octaves)",&sv->eval2,0.0f,2.0,0.1);
  GUI_MakeInputVarDouble_func(sm,"Multiplication pr. sec.:",GUI_generalMult_pr_horiz,0.0f,20.0,1.0);
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Spread",&sv->funcsel,false,1,0.0f,2.0f);
  GUI_MakeInputVarDouble(sm,"Amplitude sensitivity (0-1):",&sv->mul,0.0f,1.0,0.0);

  GUI_CloseMenuButtonOkCancelPreview(sm,Spread,true);

}
