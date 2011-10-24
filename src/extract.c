
#include "ceres.h"
#include "config.h"
#include "play.h"

static Widget amplitude,pitch,centroid;
static Widget extractButton, extractMenu;

void Amplitude(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  double amp;
  int j, lastx=0, lasty=0, antnod=1, x, lastxx=-1, dnod;

  dnod=thewidth/CONFIG_getInt("min_no_extract_nodes");

  PlayStopHard();

  for (j=0; j<fftsound->horiz_num; j++) {
    x=(int)(j*(double)thewidth/fftsound->horiz_num)+50;
    if (x!=lastxx) {
      amp=fftsound->largamp[j];
      if (amp>1.) amp=1.;
      amp = (amp/2.)+0.25;
      amp = (1.-amp)*theheight+10.;

      if (j==0) {
	lastx=50; lasty=(int)amp;
	xpos[1]=50; ypos[1]=(int)amp;
      }else{
	if (
	    (antnod<MAXNOD)
	    && ((fabs(amp-lasty)>50.) || (x-lastx>dnod))
	    )
	  {
	  antnod++;
	  lastx=x; lasty=(int)amp;
	  xpos[antnod]=x; ypos[antnod]=(int)amp;
	}
      }
      lastxx=x;
    }
  }
  xpos[antnod]=thewidth+50;
  numsquare=antnod;
  RedrawWin(fftsound);
}


void Pitch(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  double maxamp, maxamp2, freq, freq2;
  int i, j, point=0, lastx=0, lasty=0, antnod=1, x, lastxx=-1, maxbin, maxbin2, dnod;

  dnod=thewidth/CONFIG_getInt("min_no_extract_nodes"); 

  PlayStopHard();

  for (j=0; j<fftsound->horiz_num; j++) {
    x=(int)(j*(double)thewidth/fftsound->horiz_num)+50;
    if (x!=lastxx) {

      maxamp=0.; maxbin=1;
      for (i=2; i<40; i++) if (MEGAMP_GET(point+i)>maxamp) {
        maxamp=MEGAMP_GET(point+i); maxbin=i;
      }
      freq=MEGFREQ_GET(maxbin+point);
      freq2=0.;
      if (freq<60.) freq=0.; else {
        maxamp2=0.; maxbin2=1;
        for (i=2; i<maxbin-1; i++) if (MEGAMP_GET(point+i)>maxamp2) {
          maxamp2=MEGAMP_GET(point+i); maxbin2=i;
        }
        freq2=MEGFREQ_GET(maxbin2+point);
        if ((fabs(freq/freq2-2.)<0.1) && (maxamp/maxamp2<16.)) freq=freq2;
        else if ((fabs(freq/freq2-3.)<0.1) && (maxamp/maxamp2<16.)) freq=freq2;
        else if ((fabs(freq/freq2-1.5)<0.1) && (maxamp/maxamp2<16.)) freq=freq2/2.;
      }

      freq=freq/1000.;
      if (freq>1.) freq=1.;
      freq = (1.-freq)*theheight+10.;

      if (j==0) {
	lastxx=50; lasty=(int)freq; xpos[1]=50; ypos[1]=(int)freq;
      }else{
	if (
	    (antnod<MAXNOD)
	    && ((fabs(freq-lasty)>50.) || (x-lastx>dnod))
	    )
	  {
	    antnod++;
	    lastx=x; lasty=(int)freq;
	    xpos[antnod]=x; ypos[antnod]=(int)freq;
	  }
      }
      lastxx=x;
    }
    point+=fftsound->numchannels;
  }
  xpos[antnod]=thewidth+50;
  numsquare=antnod;
  RedrawWin(fftsound);
}


void Centroid(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  double centro, norm;
  int i, j, point=0, lastx=0, lasty=0, antnod=1, x, lastxx=-1, dnod;

  dnod=thewidth/CONFIG_getInt("min_no_extract_nodes"); 

  PlayStopHard();

  for (j=0; j<fftsound->horiz_num; j++) {
    x=(int)(j*(double)thewidth/fftsound->horiz_num)+50;
    if (x!=lastxx) {
      centro=0.; norm=0.;
      for (i=1; i<fftsound->numchannels-1; i++) {
        centro+=MEGAMP_GET(point+i)*MEGAMP_GET(point+i);
        norm+=MEGAMP_GET(point+i);
      }
      centro=(norm<=0.?0.:centro/norm);
      /* if (max<0.01) centro=0.; */
      centro/=5000.;
      if (centro>1.) centro=1.;
      centro = (centro/2.)+0.25;
      centro = (1.-centro)*theheight+10.;

      if (j==0) { lastxx=50; lasty=(int)centro; xpos[1]=50; ypos[1]=(int)centro; }
      else if ((antnod<MAXNOD) && ((fabs(centro-lasty)>50.) || (x-lastx>dnod))) {
        antnod++;
        lastx=x; lasty=(int)centro;
        xpos[antnod]=x; ypos[antnod]=(int)centro;
     }
      lastxx=x;
    }
    point+=fftsound->numchannels;
  }
  xpos[antnod]=thewidth+50;
  numsquare=antnod;
  RedrawWin(fftsound);

}


void create_extract(struct FFTSound *fftsound){
  XmString text;

  /* CREATE EXTRACT MENU */
  extractButton=XtVaCreateManagedWidget(
    "extractButton",xmCascadeButtonWidgetClass,menuBar,NULL);
  text=XmStringCreateLocalized("Extract");
  XtVaSetValues(extractButton,XmNlabelString,text,NULL);
  extractMenu=XmCreatePulldownMenu(menuBar,"extractMenu",NULL,0);
  XtVaSetValues(extractButton,XmNsubMenuId,extractMenu,NULL);

  /* CREATE AMPLITUDE BUTTON */
  amplitude=XtVaCreateManagedWidget(
    "amplitude",xmPushButtonWidgetClass,extractMenu,NULL);
  text=XmStringCreateLocalized("Amplitude");
  XtVaSetValues(amplitude,XmNlabelString,text,NULL);
  XtAddCallback(amplitude,XmNactivateCallback,Amplitude,fftsound);

  /* CREATE PITCH BUTTON */
  pitch=XtVaCreateManagedWidget(
    "pitch",xmPushButtonWidgetClass,extractMenu,NULL);
  text=XmStringCreateLocalized("Pitch");
  XtVaSetValues(pitch,XmNlabelString,text,NULL);
  XtAddCallback(pitch,XmNactivateCallback,Pitch,fftsound);

  /* CREATE CENTROID BUTTON */
  centroid=XtVaCreateManagedWidget(
    "centroid",xmPushButtonWidgetClass,extractMenu,NULL);
  text=XmStringCreateLocalized("Centroid");
  XtVaSetValues(centroid,XmNlabelString,text,NULL);
  XtAddCallback(centroid,XmNactivateCallback,Centroid,fftsound);

  
}
