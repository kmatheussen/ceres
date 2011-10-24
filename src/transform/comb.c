
#include "../transform.h"


struct Vars{
  struct FFTSound *fftsound;
  double mult_pr_horiz;

  double combval2;
  bool funcsel;
};


void Comb(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  struct Vars *sv=(struct Vars*)pointer;

  int i, j, ch, basebin;
//  int bin;
  long point;
  double combval;

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=ch*fftsound->numchannels*fftsound->horiz_num+area_start*fftsound->numchannels;
    funcpt=1; combval=sv->combval2;
    for (i=area_start; i<area_end; i++) {
      for (j=areaf1; j<areaf2; j++) {
        if (sv->funcsel) combval=1000.*getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
        basebin=(int)(combval/fftsound->binfreq+0.5);
        if (basebin>=fftsound->numchannels) basebin=fftsound->numchannels-1;
        if (basebin) {
          if (j%basebin){
	    MEGAMP_PUT(point,0.);
	  }
        }
        combval*=sv->mult_pr_horiz;
	point++;
      }
      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }
 end:
  return;
}


void create_transform_comb(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Comb",0,300);

  GUI_MakeInputVarFreq(sm,"Fundamental frequency (Hz):",&sv->combval2,0.0,20000.0,200.0);
  GUI_MakeInputVarDouble_func(sm,"Multiplication pr. sec.:",GUI_generalMult_pr_horiz,0.0f,20.0,1.0);
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Fundamental",&sv->funcsel,false,1,0.0f,1000.0f);

  GUI_CloseMenuButtonOkCancelPreview(sm,Comb,true);

}
