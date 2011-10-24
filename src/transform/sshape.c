
/*
 * The spectrum shape transform routine is ported from ceres3. (Seems to be a bit half-finished...?)
 */

#include "../transform.h"

struct Vars {
  bool funcsel3;
  bool funcsel2;
  bool funcsel;
};


void
Sshape(struct CTransform *ctransform,
       void *pointer, struct FFTSound *fftsound, int area_start,
       int area_end,
       int funcstartdiff
       )
{
  struct Vars *sv = (struct Vars *) pointer;

    float h[fftsound->numchannels];
    float s[fftsound->numchannels];
    float d[fftsound->horiz_num];
    float val, temp;
    long point;
    bool funcsel3, funcsel2, funcsel;
    int i, j, art1, art2, arf1, arf2;
    int ch;

    //    areafind(&art1, &art2, &arf1, &arf2);
    art1 = area_start;
    art2 = area_end;
    arf1 = areaf1;
    arf2 = areaf2;

    funcsel3 = sv->funcsel3;
    funcsel = sv->funcsel;
    funcsel2 = sv->funcsel2;
    funcpt3 = numsquare3;
    funcpt = 1;
    funcpt2 = 1;


    if (!funcsel3){
      // && !funcsel2 && !funcsel) {
      printf("Error: Average spectrum must for some reason be selected.\n");
      return;
    }

    if (funcsel2){
      for (i = 0; i < fftsound->horiz_num; i++) {
	d[i] = 2. * getfuncval2(fftsound,i+funcstartdiff) / (float) (fftsound->N / 2);
      }
    }else{
      for (i = 0; i < fftsound->horiz_num; i++) {
	d[i] = 1.0;
      }
    }


    if (funcsel3) {
      for (j = arf1; j < arf2; j++) {
	h[j] = (400. - getfuncval3(fftsound,j)) / 100000000.;
      }
    }else{
      memset(h,0,sizeof(float)*fftsound->numchannels);
    }


    DA_convertToChannel(fftsound);

    for (ch=0; ch<fftsound->samps_per_frame; ch++) {
      point = art1 * fftsound->numchannels + ch*fftsound->horiz_num*fftsound->numchannels;

      /*compute RMS spectrum */
      memset(s,0,sizeof(float)*fftsound->numchannels);
      for (j = arf1; j < arf2; j++) {
	for (i = art1; i < art2; i++) {
	  temp = .5 * MEGAMP_GET_C(point + j + fftsound->numchannels * (i - art1));
	  temp *= temp;
	  s[j] += temp;
	}
	s[j] = sqrt(s[j]) / sqrt(art2 - art1);
	if (s[j] == 0.)
	  s[j] = pow(10., -10.);
	if(GUI_UpdateProgress(ctransform,arf1,j,arf2)==false) goto end;
      }
      val = 1.;

      for (j = arf1; j < arf2; j++) {
	funcpt = 1;

	for (i = art1; i < art2; i++) {
	  float temp2=MEGAMP_GET_C(point + j + fftsound->numchannels * (i - art1));
	  temp=temp2/s[j];
	  
	  if (funcsel) {
	    val = getfuncval(fftsound,i+funcstartdiff) / (float) (fftsound->N / 2);
	    if (val < 0.)
	      val = 0.;
	  }
	  
	  MEGAMP_PUT_C(
		     point + j + fftsound->numchannels * (i - art1),
		     val * pow(temp,d[i])* h[j] 
		     + (1. - val) * temp2
		     );
	}
	if(GUI_UpdateProgress(ctransform,arf1,j,arf2)==false) goto end;      
      }
    }
    //    amptrack(art1, art2, arf1, arf2);

 end:
  DA_convertToPoint(fftsound);  

}

void create_transform_sshape(struct FFTSound *fftsound)
{
  struct VarInput *sm;
  struct Vars *sv = malloc(sizeof(struct Vars));


  sm = GUI_MakeMenuButton(fftsound, sv, transMenu, "Spectrum Shape", 0, 240);

  GUI_MakeInputVarToggle_graph(sm,"Control function 3 -> Average spectrum",&sv->funcsel3,true,3,0.0f,thewidth);
  GUI_MakeInputVarToggle_graph(sm,"Control function 2 -> Spectral dynamics",&sv->funcsel2,false,2,0.0f,theheight);
  GUI_MakeInputVarToggle_graph(sm,"Control function 1 -> Process depth",&sv->funcsel,false,1,0.0f,theheight);

  GUI_CloseMenuButtonOkCancelPreview(sm, Sshape, true);

}
