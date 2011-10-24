
#include "../transform.h"


struct Vars{
  struct FFTSound *fftsound;

  double amount;

  bool funcsel;
  bool envelopesel;
  bool despecklesel;

};


static void envelopenoise(struct FFTSound *fftsound,double *ampvec){
  int i, j;
  double max;
  
  for (i=1; i<fftsound->numchannels/8-1; i++) {
    max=0.;
    for (j=0; j<8; j++) if (ampvec[i*8+j]>max) max=ampvec[i*8+j];
    for (j=0; j<8; j++) ampvec[i*8+j]=max;
  }
}


void Noisered(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  struct Vars *sv=(struct Vars*)pointer;

  double val, tempamp[fftsound->numchannels];
  long point;
  int i, j, ch;

  val=sv->amount;

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    //1
    point=area_start*fftsound->numchannels+ch*fftsound->numchannels*fftsound->horiz_num;
    memset(tempamp,0.0,fftsound->numchannels*sizeof(double));
    //2
    for (i=area_start; i<area_end; i++) {
      for (j=0; j<fftsound->numchannels; j++)
	tempamp[j]+=MEGAMP_GET(point+j);
      point+=fftsound->numchannels;

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
    //3
    for (j=0; j<fftsound->numchannels; j++)
      tempamp[j]/=(area_end-area_start);
    //4
    if (sv->envelopesel)
      envelopenoise(fftsound,tempamp);
    //5
    point=ch*fftsound->numchannels*fftsound->horiz_num;
    //6
    for (i=0; i<fftsound->horiz_num; i++) {
      if (sv->funcsel) val=2.*getfuncval(fftsound,i+funcstartdiff)/(double)theheight;
      for (j=areaf1; j<areaf2; j++) {
        MEGAMP_PUT_SUB(point+j,val*tempamp[j]);
        if (MEGAMP_GET(point+j)<0.){
	  MEGAMP_PUT(point+j,0.);
	}
      }
      point+=fftsound->numchannels;

      if(GUI_UpdateProgress(ctransform,0,i,fftsound->horiz_num)==false) goto end;
    }
    //7

    if (sv->despecklesel) {

      point=ch*fftsound->numchannels*fftsound->horiz_num+fftsound->numchannels;
      for (i=1; i<fftsound->horiz_num-1; i++) {
        for (j=areaf1; j<areaf2; j++) {
          if (
	      (MEGAMP_GET(point+j-fftsound->numchannels)==0.)
	      && (MEGAMP_GET(point+j+fftsound->numchannels)==0.)
	      )
	    {
	      MEGAMP_PUT(point+j,0.);
	    }
        }
        point+=fftsound->numchannels;

	if(GUI_UpdateProgress(ctransform,1,i,fftsound->horiz_num-1)==false) goto end;
      }

      point=ch*fftsound->numchannels*fftsound->horiz_num;
      for (i=0; i<fftsound->horiz_num; i++) {
        for (j=areaf1+1; j<areaf2-1; j++) {
          if (
	      (MEGAMP_GET(point+j-1)==0.)
	      && (MEGAMP_GET(point+j+1)==0.)
	      )
	    {
	      MEGAMP_PUT(point+j,0.);
	    }
        }
        point+=fftsound->numchannels;

	if(GUI_UpdateProgress(ctransform,0,i,fftsound->horiz_num)==false) goto end;
      }

      point=ch*fftsound->numchannels*fftsound->horiz_num+fftsound->numchannels;
      for (i=1; i<fftsound->horiz_num-1; i++) {
        for (j=areaf1; j<areaf2; j++) {
          if (
	      (MEGAMP_GET(point+j-fftsound->numchannels)==0.)
	      && (MEGAMP_GET(point+j+fftsound->numchannels)==0.)
	      )
	    {
	      MEGAMP_PUT(point+j,0.);
	    }
        }
        point+=fftsound->numchannels;

	if(GUI_UpdateProgress(ctransform,1,i,fftsound->horiz_num-1)==false) goto end;
      }


    } // End if


  } // End for

 end:
  return;
}

void create_transform_noisered(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Spectral substraction",0,240);

  GUI_MakeInputVarDouble(sm,"Amount (0-2):",&sv->amount,0.0,2.0,0.8);
  GUI_MakeInputVarToggle_graph(sm,"Control function -> Amount",&sv->funcsel,false,1,0.0f,2.0f);
  GUI_MakeInputVarToggle(sm,"Envelope noise spectrum",&sv->envelopesel,false);
  GUI_MakeInputVarToggle(sm,"Despeckle",&sv->despecklesel,false);

  GUI_CloseMenuButtonOkCancelPreview(sm,Noisered,true);

}
