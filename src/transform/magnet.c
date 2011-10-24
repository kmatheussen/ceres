
#include "../transform.h"


static void gridsort(double *gridfreqs2, int left, int right)
{
  int i, j;
  float x, y;

  i=left; j=right;
  x=gridfreqs2[(left+right)/2];

  do {
    while (gridfreqs2[i]<x && i <right) i++;
    while (x<gridfreqs2[j] && j>left) j--;

    if(i<=j) {
      y=gridfreqs2[i];
      gridfreqs2[i]=gridfreqs2[j];
      gridfreqs2[j]=y;
      i++; j--;
    }
  } while (i<=j);

  if (left<j) gridsort(gridfreqs2, left, j);
  if (i<right) gridsort(gridfreqs2, i, right);
}


struct Vars{
  struct FFTSound *fftsound;
  double mult_pr_horiz;

  double proba2;

  bool funcsel;
};



void Magnet(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  struct Vars *sv=(struct Vars*)pointer;

  int i, j, k, nearest, ch, totf;
  double mindiff, f, proba;
  long point=0;
  double gridfreqs2[600];

  totf=numfreqs;
  for (i=0; i<totf; i++)
    gridfreqs2[i]=gridfreqs[i];

  gridsort(gridfreqs2, 0, totf-1);

  for (i=0; i<totf; i++) {
    while (gridfreqs2[i]==gridfreqs2[i+1]) { 
      for (j=i; j<numfreqs-1; j++) gridfreqs2[j]=gridfreqs2[j+1];
      totf--;  
    }
  }

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=area_start*fftsound->numchannels+ch*fftsound->numchannels*fftsound->horiz_num;
    funcpt=1;
    proba=sv->proba2;
    for (i=area_start; i<area_end; i++) {
      if (sv->funcsel) proba=getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
      for (j=areaf1; j<areaf2; j++) {
        if ((double)rand()/RAND_MAX<proba) {
          nearest=totf-1; mindiff=99999.;
          for (k=0; k<100; k++) {
            f=fabs(MEGFREQ_GET(point+j)-gridfreqs2[k]);
            if (f<mindiff) {
              nearest=k; mindiff=f;
            } else break;
          }
          MEGFREQ_PUT(point+j,gridfreqs2[nearest]);
          if (MEGFREQ_GET(point+j)<20.){
	    MEGAMP_PUT_MUL(point+j,MEGFREQ_GET(point+j)/20.);
	  }
        }
	if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
      }
      point+=fftsound->numchannels;
      proba*=sv->mult_pr_horiz;
    }
  }

 end:
  rightbins(fftsound);
}

void create_transform_magnet(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Move to pitch grid",0,240);

  GUI_MakeInputVarDouble(sm,"Probability (0-1):",&sv->proba2,0.0,1.0,1.0);
  GUI_MakeInputVarDouble_func(sm,"Multiplication pr. sec.:",GUI_generalMult_pr_horiz,0.0,20.0,1.0);
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Probability",&sv->funcsel,false,1,0.0f,1.0f);

  GUI_CloseMenuButtonOkCancelPreview(sm,Magnet,true);

}


