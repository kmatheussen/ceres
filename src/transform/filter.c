
#include "../transform.h"



struct Vars{
  struct FFTSound *fftsound;

  double filter_lower;
  double filter_upper;
  bool funcsel;

  double filterfreqs[1000];
};

static int makefilterfile(struct Vars *sv)
{
  int i;
  float f;
  FILE *fil;

  i=0; fil=fopen("frequencies","r");
  if (fil==NULL) {
    printf("Couldn't open frequencies file\n");
    return 0;
  }
  while ((!feof(fil)) && (i<100)) {
    fscanf(fil,"%f",&f);
    if (feof(fil)) break;
    sv->filterfreqs[i++]=f;
  }
  fclose(fil);
  return i;
}


void Filter(
	     struct CTransform *ctransform,
	     void *pointer,
	     struct FFTSound *fftsound,
	     int area_start,int area_end,
	     int funcstartdiff
	     )
{
  struct Vars *sv=(struct Vars*)pointer;

  long point, ch;
  int i, j, k,  keep;

  if (sv->funcsel==true) {
    int ant=makefilterfile(sv);
    for (ch=0; ch<fftsound->samps_per_frame; ch++) {
      point=areat1*fftsound->numchannels+ch*fftsound->numchannels*fftsound->horiz_num;
      if (ant>0) for (i=area_start; i<area_end; i++) {
        for (j=areaf1; j<areaf2; j++) {
          keep=0;
          for (k=0; k<ant; k++) {
	    double filter_lower,filter_upper;
            filter_lower=sv->filterfreqs[k]-fftsound->binfreq/2.;
            filter_upper=sv->filterfreqs[k]+fftsound->binfreq/2.;
            if ((MEGFREQ_GET(point+j)>=filter_lower)
              && (MEGFREQ_GET(point+j)<=filter_upper)) keep=1;
          }
          if (keep==0){
	    MEGAMP_PUT(point+j,0.);
	  }
        }
        point+=fftsound->numchannels;

	if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
      }
    } 
  } else {
    double filter_lower=sv->filter_lower;
    if (filter_lower==0.) filter_lower=-99999.;

    for (ch=0; ch<fftsound->samps_per_frame; ch++) {
      point=areat1*fftsound->numchannels+ch*fftsound->numchannels*fftsound->horiz_num;
      for (i=area_start; i<area_end; i++) {
        for (j=areaf1; j<areaf2; j++)
          if ((MEGFREQ_GET(point+j)>=filter_lower) && (MEGFREQ_GET(point+j)<=sv->filter_upper)){
            MEGAMP_PUT(point+j,0.);
	  }
        point+=fftsound->numchannels;

	if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
      }
    }
  }

 end:
  return;
}

void create_transform_filter(struct FFTSound *fftsound){
  struct VarInput *sm;
  struct Vars *sv=malloc(sizeof(struct Vars));

  sv->fftsound=fftsound;

  sm=GUI_MakeMenuButton(fftsound,sv,transMenu,"Filter",0,240);

  GUI_MakeInputVarFreq(sm,"Lower cutoff frequency: ",&sv->filter_lower,0.0,20000.0,0.0);
  GUI_MakeInputVarFreq(sm,"Upper cutoff frequency: ",&sv->filter_upper,0.0,20000.0,0.0);
  GUI_MakeInputVarToggle(sm,"Read from file 'frequencies'",&sv->funcsel,false);

  GUI_CloseMenuButtonOkCancelPreview(sm,Filter,true);

}
