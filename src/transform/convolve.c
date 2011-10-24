
#include "../transform.h"


struct Vars{
  bool laplace;
  bool mean;
  bool timemean;
  bool readfromfile;
};


void Convolve(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  struct Vars *sv=(struct Vars*)pointer;

  static const double laplace[3][3]={
    { 0, -1,  0},
    {-1,  4, -1},
    { 0, -1,  0}};
  static const double mean[3][3]={
    { 0.11, 0.11, 0.11},
    { 0.11, 0.11, 0.11},
    { 0.11, 0.11, 0.11}};
  static const double timemean[3][3]={
    { 0., 0., 0.},
    { 0.33, 0.33, 0.33},
    { 0., 0., 0.}};
  double op[3][3];

  int i, j, j1, ch;
//  int i1;
  long point;
  double sum;
  double last[fftsound->numchannels], this[fftsound->numchannels];

  memset(this,0,fftsound->numchannels*sizeof(double));
  memset(last,0,fftsound->numchannels*sizeof(double));

  for (i=0; i<=2; i++) for (j=0; j<=2; j++) {
   if (sv->laplace) op[i][j]=laplace[j][i];
   if (sv->mean) op[i][j]=mean[j][i];
   if (sv->timemean) op[i][j]=timemean[j][i];
  }

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    for (j=0; j<fftsound->numchannels; j++) this[j]=0.;
    point=area_start*fftsound->numchannels+ch*fftsound->numchannels*fftsound->horiz_num;
    for (i=area_start; i<area_start; i++) {
      for (j=0; j<fftsound->numchannels; j++)
	last[j]=this[j];
      for (j=0; j<fftsound->numchannels; j++)
	this[j]=MEGAMP_GET(point+j);
      for (j=areaf1; j<areaf2; j++) {
        sum=0.;
        for (j1=-1; j1<=1; j1++) {
          if ((j+j1>=0) && (j+j1<fftsound->numchannels))
            sum+=op[0][j1+1]*last[j+j1];
        }
        for (j1=-1; j1<=1; j1++) {
          if ((j+j1>=0) && (j+j1<fftsound->numchannels))
            sum+=op[1][j1+1]*this[j+j1];
        }
        for (j1=-1; j1<=1; j1++) {
          if ((i+1<fftsound->horiz_num) && (j+j1>=0) && (j+j1<fftsound->numchannels))
            sum+=op[2][j1+1]*MEGAMP_GET(point+fftsound->numchannels+j+j1);
        }
        if (sum>=0.){
	  MEGAMP_PUT(point+j,sum);
	}

      }
      point+=fftsound->numchannels;

      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }

 end:
  return;
}

void create_transform_convolve(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Convolve",0,240);

  GUI_setVarInputRadioStart(sm,true);
  GUI_MakeInputVarToggle(sm,"Laplace",&sv->laplace,true);
  GUI_MakeInputVarToggle(sm,"Mean",&sv->mean,false);
  GUI_MakeInputVarToggle(sm,"Time Mean",&sv->timemean,false);
  GUI_MakeInputVarToggle(sm,"Read from file 'convolve'",&sv->readfromfile,false);
  GUI_setVarInputRadioStop(sm);

  GUI_CloseMenuButtonOkCancelPreview(sm,Convolve,true);

}
