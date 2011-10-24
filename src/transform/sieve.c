
#include "../transform.h"

struct SieveVars{
  struct FFTSound *fftsound;
  double mult_pr_horiz;

  int numparint2;
  double sievew;
  bool ampsel;
  bool funcsel;
  bool nullsel;
};

void setSieveTextfield2(double val,void *pointer){
  struct SieveVars *sv=(struct SieveVars*)pointer;
  sv->mult_pr_horiz=pow(val,sv->fftsound->duration/sv->fftsound->horiz_num);
  printf("Val: %f\n",val);
}

void SieveOk(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  int numparint, i, j, k, maxnum, ch;
  long point;
  double tempamp[fftsound->numchannels], megamp2[fftsound->numchannels], max, numpar, rscale;
  struct SieveVars *sv=(struct SieveVars*)pointer;

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {

    funcpt=1;
    maxnum=1;
    numparint=sv->numparint2;
    numpar=(double)numparint;

    rscale=(double)(areaf2-areaf1)/RAND_MAX;
    point=(ch*fftsound->numchannels*fftsound->horiz_num) + (area_start*fftsound->numchannels);

    for (i=area_start; i<area_end; i++) {

      if (sv->funcsel) numparint=(int)getfuncval(fftsound,i+funcstartdiff);
      if(
	 (!sv->ampsel)
	 && (numparint>fftsound->numchannels-10)
	 )
	numparint=fftsound->numchannels-10;

      for (j=0; j<fftsound->numchannels; j++) {
        if (sv->nullsel){
	  tempamp[j]=MEGAMP_GET(point+j);
	}else{
	  tempamp[j]=0.;
	}
        megamp2[j]=MEGAMP_GET(point+j)*(1.+sv->sievew*j);
      }

      if (sv->ampsel){

	for (k=0; k<numparint; k++) {
	  max=0.;
	  if (sv->ampsel) for (j=areaf1; j<areaf2; j++) {
	    if (megamp2[j]>=max) {
	      max=megamp2[j]; maxnum=j;
	    }
	  }
	  if (sv->nullsel){
	    tempamp[maxnum]=0.;
          }else{
	    tempamp[maxnum]=MEGAMP_GET(point+maxnum);
	  }
	  megamp2[maxnum]=-1.;
	}

      }else{

	for (k=0; k<numparint; k++) {
	  j=0;
	  do {
	    maxnum=(int)((double)rand()*rscale)+areaf1;
	    j++;
	  }while ((megamp2[maxnum]==-1) && (j<fftsound->numchannels));

	  if (sv->nullsel){
	    tempamp[maxnum]=0.;
          }else{
	    tempamp[maxnum]=MEGAMP_GET(point+maxnum);
	  }
	  megamp2[maxnum]=-1.;
	}

      }

      for (j=areaf1; j<areaf2; j++){
	MEGAMP_PUT(point+j,tempamp[j]);
      }
      point+=fftsound->numchannels;

      if (!sv->funcsel) {
	numpar*=sv->mult_pr_horiz;
	numparint=(int)(floor(numpar));
      }

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }
 end:
  return;
}


void create_transform_sieve(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct SieveVars *sv=malloc(sizeof(struct SieveVars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Sieve",0,240);

  GUI_MakeInputVarInt(sm,"Number of harmonics:",&sv->numparint2,0,100,10);

  GUI_MakeInputVarDouble_func(sm,"Multiplication pr. sec.:",GUI_generalMult_pr_horiz,0.0f,20.0f,1.0f);
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Num. harm.",&sv->funcsel,false,1,0.0f,theheight);
  GUI_MakeInputVarFreq(sm,"High frequency boost:",&sv->sievew,0.0f,20000.0f,0.0f);

  GUI_setVarInputRadioStart(sm,true);
  GUI_MakeInputVarToggle(sm,"Amplitude selection",&sv->ampsel,true);
  GUI_MakeInputVarToggle(sm,"Random selection",NULL,false);
  GUI_setVarInputRadioStop(sm);

  GUI_MakeInputVarToggle(sm,"Null out selected partials",&sv->nullsel,false);

  GUI_CloseMenuButtonOkCancelPreview(sm,SieveOk,true);
}

