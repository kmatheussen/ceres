

/*
 * The warp transform routine is ported from ceres3.
 */


#include "../transform.h"


struct Vars {
  double warpval;

  bool am;
  bool phas;
  bool prob;
  bool depth;
  bool env;
};


void
Warp(struct CTransform *ctransform,
     void *pointer, struct FFTSound *fftsound, int area_start,
     int area_end,
     int funcstartdiff
     )
{
  struct Vars *sv = (struct Vars *) pointer;

  int art1, art2, arf1, arf2, art;
  float val0, val, regen;
  long point;
  long dev;
  int i, j,ch;

  /* Oh boy! */
  long t[area_end-area_start];
  float a[area_end-area_start+1];
  float d[fftsound->horiz_num+1];
  float f[area_end-area_start+1];
  float g[fftsound->horiz_num+1];
  float h[fftsound->numchannels+1];

  val0 = sv->warpval;

  if (val0 < 0.)
    val0 = 0.;
  if (val0 > 1.)
    val0 = 1.;
     
  regen = 1.;


  //  areafind (&art1, &art2, &arf1, &arf2);
  art1 = area_start;
  art2 = area_end;
  arf1 = areaf1;
  arf2 = areaf2;

  art = art2 - art1;
  
  if (sv->prob) {
    funcpt3 = numsquare3;
    for (j = 0; j < fftsound->numchannels; j++) {
      h[j] = (400. - getfuncval3(fftsound,j)) / 400.;
      if (h[j] <= 0.)
	h[j] = 1e-9;
    }
  }

  if (sv->depth) {
    funcpt2 = 1;
    for (i = 0; i < fftsound->horiz_num; i++) {
      d[i] = getfuncval2(fftsound,i+funcstartdiff) / (float) fftsound->numchannels;
      if (d[i] <= 0.)
	d[i] = 1e-9;
      if (d[i] > 1.)
	d[i] = 1.;
    }
  }

  if (sv->env) {
    funcpt = 1;
    for (i = 0; i < fftsound->horiz_num; i++) {
      g[i] = getfuncval(fftsound,i+funcstartdiff) / (float) fftsound->numchannels;
      if (g[i] <= 0.)
	g[i] = 1e-9;
      if (g[i] > 1.)
	g[i] = 1.;
    }
  }


  DA_convertToChannel(fftsound);

  /*
   * begin algorithm 
   */
  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point = art1 * fftsound->numchannels + ch*fftsound->horiz_num*fftsound->numchannels;
    val = val0;
    for (j = arf1; j < arf2; j++) {
      if (sv->prob)
	val = val0 * h[j];
      for (i = 0; i < art; i++) {
	dev =
	  (long) (val * (art * rand() / (float) RAND_MAX - art / 2));
	if (sv->env)
	  dev *= g[i + art1];
	t[i] = i + dev;
	if (t[i] < 0)
	  t[i] *= (-1);
	if (t[i] > art)
	  t[i] = 2 * art - t[i];
	
	t[i] *= fftsound->numchannels;
	t[i] += (point + j);
	if (sv->am)
	  a[i] = MEGAMP_GET_C(t[i]);
	if (sv->phas)
	  f[i] = MEGFREQ_GET_C(t[i]);
      }
      if (sv->depth) {
	for (i = 0; i < art; i++) {
	  if (sv->am){
	    double temp=MEGAMP_GET_C(i * fftsound->numchannels + point + j);
	    MEGAMP_PUT_C(i*fftsound->numchannels + point + j , a[i]*d[i+art1] +  temp*(1.-d[i+art1]));
	  }
	  if (sv->phas){
	    double temp=MEGFREQ_GET_C(i*fftsound->numchannels + point + j);
	    MEGFREQ_PUT_C(i*fftsound->numchannels + point + j,f[i] * d[i + art1] + temp*(1.-d[i+art1]));
	  }
	}
      } else {
	for (i = 0; i < art; i++) {
	  if (sv->am){
	    MEGAMP_PUT_C(i * fftsound->numchannels + point + j,a[i]);
	  }
	  if (sv->phas){
	    MEGFREQ_PUT_C(i * fftsound->numchannels + point + j,f[i]);
	  }
	}
      }

      if(GUI_UpdateProgress(ctransform,arf1,j,arf2)==false) goto end;      
    }				// End for(j=

  } // end for(ch=

  /*
   * end algorithm 
   */


 end:
  DA_convertToPoint(fftsound);  
  rightbins(fftsound);
}

void create_transform_warp(struct FFTSound *fftsound)
{
  struct VarInput *sm;
  struct Vars *sv = malloc(sizeof(struct Vars));


  sm = GUI_MakeMenuButton(fftsound, sv, transMenu, "Warp", 0, 240);

  GUI_MakeInputVarDouble(sm,"Warp (0-1):",&sv->warpval,0.0,1.0,0.7);
  GUI_MakeInputVarToggle(sm,"Warp Amplitude Spectrum",&sv->am,false);
  GUI_MakeInputVarToggle(sm,"Warp Frequency Spectrum",&sv->phas,false);
  GUI_MakeInputVarToggle_graph(sm,"Control function 3 -> Warp",&sv->prob,false,3,0.0f,thewidth);
  GUI_MakeInputVarToggle_graph(sm,"Control function 2 -> Depth",&sv->depth,false,2,0.0f,theheight);
  GUI_MakeInputVarToggle_graph(sm,"Control function 1 -> Density",&sv->env,false,1,0.0f,theheight);

  GUI_CloseMenuButtonOkCancelPreview(sm, Warp, true);

}
