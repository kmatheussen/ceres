#include "../transform.h"


struct Vars{
  struct FFTSound *fftsound;
  double mult_pr_horiz;

  double shval2;
  bool funcsel;
};


void Shear(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  int i, j, ch;
  long delay, moveto, start;
  double shval, fact;
  struct Vars *sv=(struct Vars*)pointer;

  DA_convertToChannel(fftsound);

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    funcpt=1;
    start=ch*fftsound->numchannels*fftsound->horiz_num;
    if (sv->shval2<0) for (i=area_start; i<area_end; i++) {
      shval=-sv->shval2*pow(sv->mult_pr_horiz,i);
      if (sv->funcsel) shval=getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
      fact=fftsound->R*shval/(1000.*fftsound->Dn);
      for (j=areaf1; j<areaf2; j++) {
        delay=(long)(fact*MEGFREQ_GET_C(i*fftsound->numchannels+j+start));
        moveto=i-delay; if (moveto>=fftsound->horiz_num) moveto=fftsound->horiz_num-1;
        if (moveto<0) moveto=0;
	MEGFREQ_COPY_C(moveto*fftsound->numchannels+j+start,i*fftsound->numchannels+j+start);
        MEGAMP_COPY_C(moveto*fftsound->numchannels+j+start,i*fftsound->numchannels+j+start);
        MEGFREQ_PUT_C(i*fftsound->numchannels+j+start,0.);
	MEGAMP_PUT_C(i*fftsound->numchannels+j+start,0.);
      }

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;

    } else for (i=area_end-1; i>=area_start; i--) {
      shval=sv->shval2*pow(sv->mult_pr_horiz,i-area_start);
      if (sv->funcsel) shval=getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
      fact=fftsound->R*shval/(1000.*fftsound->Dn);
      for (j=areaf1; j<areaf2; j++) {
        delay=(long)(fact*MEGFREQ_GET_C(i*fftsound->numchannels+j+start));
        moveto=i+delay;
	if (moveto>=fftsound->horiz_num){
	  moveto=fftsound->horiz_num-1;
	}
        if (moveto<0){
	  moveto=0;
	}
        MEGFREQ_COPY_C(moveto*fftsound->numchannels+j+start,i*fftsound->numchannels+j+start);
        MEGAMP_COPY_C(moveto*fftsound->numchannels+j+start,i*fftsound->numchannels+j+start);
        MEGFREQ_PUT_C(i*fftsound->numchannels+j+start,0.);
	MEGAMP_PUT_C(i*fftsound->numchannels+j+start,0.);
      }

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }

 end:

  DA_convertToPoint(fftsound);


}

void create_transform_shear(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Shear",0,240);

  GUI_MakeInputVarDouble(sm,"Seconds pr. kHz: ",&sv->shval2,-20.0f,20.0,0.1);
  GUI_MakeInputVarDouble_func(sm,"Multiplication pr. sec.:",GUI_generalMult_pr_horiz,0.0f,20.0,1.0);


  GUI_CloseMenuButtonOkCancelPreview(sm,Shear,true);
}

