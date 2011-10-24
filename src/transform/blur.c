#include "../transform.h"


struct Vars{
  struct FFTSound *fftsound;
  double mult_pr_horiz;

  double blurval2;
  int stepn;
  bool funcsel;
};

void Blur_stepset(double val,void *pointer){
  struct Vars *sv=(struct Vars*)pointer;
  if(val==0.0) val=1e-9;
  sv->stepn=(int)((sv->fftsound->R/(double)sv->fftsound->Dn)/val);
}

void Blur(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  struct Vars *sv=(struct Vars*)pointer;

  int i, j, p, ch;
  long start, t;
  double blurval, ya=0.0, yf=0.0;

  Blur_stepset(0.0,pointer);

  DA_convertToChannel(fftsound);

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    start=ch*fftsound->numchannels*fftsound->horiz_num;
    for (j=areaf1; j<areaf2; j++) {
      t=0;
      blurval=sv->blurval2;
      funcpt=1;
      //      printf("stepn: %d\n",sv->stepn);
      //      printf("j: %d / %d\n",j,areaf2);
      if (blurval>0.) for (i=area_start; i<area_end; i++) {
        p = i*fftsound->numchannels + j + start;
//	printf("p: %d %d %d\n",p,p/fftsound->total_channels,p%fftsound->total_channels);
        if (sv->funcsel) blurval=getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
        if (blurval>1.0) blurval=1.0;
        if (t%sv->stepn==0) {
          ya=MEGAMP_GET_C(p);
	  yf=MEGFREQ_GET_C(p);
        } else {
          ya=blurval*ya+(1.-blurval)*MEGAMP_GET_C(p);
	  MEGAMP_PUT_C(p,ya);
          yf=blurval*yf+(1.-blurval)*MEGFREQ_GET_C(p);
	  MEGFREQ_PUT_C(p,yf);
        }
        blurval*=sv->mult_pr_horiz;
	t++;
      } else for (i=area_start+1; i<area_end; i++) {
        p=i*fftsound->numchannels+j+start;
        if (sv->funcsel) blurval=getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
        if (blurval<-1.0) blurval=-1.0;
        if (t%sv->stepn==0) {
          ya=MEGAMP_GET_C(p);
	  yf=MEGFREQ_GET_C(p);
	  MEGAMP_PUT_MUL_C(p,-16.0*blurval);
        } else {
          ya=blurval*ya+(1.+blurval)*MEGAMP_GET_C(p);
	  MEGAMP_PUT_C(p,ya*-16.0*blurval);
        }
        blurval*=sv->mult_pr_horiz;
	t++;
      }

      if(GUI_UpdateProgress(ctransform,areaf1,j,areaf2)==false) goto end;
    }
  }

 end:
  DA_convertToPoint(fftsound);  

  rightbins(fftsound);


}



void create_transform_blur(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Blur",0,240);

  GUI_MakeInputVarDouble(sm,"Blur (0-1):",&sv->blurval2,-1.0f,1.0,0.9); 
  GUI_MakeInputVarDouble_func(sm,"Multiplication pr. sec.:",GUI_generalMult_pr_horiz,0.0f,20.0,1.0);
  GUI_MakeInputVarDouble_func(sm,"Re-init frequency (0-20):",Blur_stepset,0.0f,20.0,0); 
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Blur",&sv->funcsel,false,1,0.0f,1.0f);

  GUI_CloseMenuButtonOkCancelPreview(sm,Blur,true);
}
