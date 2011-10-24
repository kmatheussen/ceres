

#include "ceres.h"
#include "play.h"

static Widget lpcButton, lpcMenu, lpcMult, lpcInvert;


void Lpcmult(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  int i, j, ch;
  long point, point2;
  double *coef, f;

  PlayStopHard();

  if (fftsound->numcoef2==0) return;
  SetWatchCursor(topLevel);
  fvec(coef, fftsound->numcoef2+1);
  for (ch=0; ch<lpc_samps_per_frame; ch++) {
    for (j=0; j<lpc_horiz_num; j++) {
      if (j>=fftsound->horiz_num) break;
      point=ch*(fftsound->numcoef2+2)*lpc_horiz_num+j*(fftsound->numcoef2+2);
      point2=ch*fftsound->horiz_num*fftsound->numchannels+j*fftsound->numchannels;
      for (i=0; i<fftsound->numcoef2+1; i++) coef[i]=lpc_coef[point+i];
      for (i=0; i<fftsound->numchannels; i++) {
        f=MEGFREQ_GET(point2+i)*TwoPi/fftsound->R;
	MEGAMP_PUT_MUL(point2+i,lpamp(
				      f,
				      lpc_coef[point+fftsound->numcoef2+1],
				      coef,
				      fftsound->numcoef2
				      )
		       *0.1/fftsound->numcoef2
		       );
        if (MEGAMP_GET(point2+i)<1e-20){MEGAMP_PUT(point2+i,0.);}
      }
    }
  }
  free(coef);
  RedrawWin(fftsound);
  ResetCursor(topLevel);
}

void Lpcinvert(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  int i, j, ch;
  long point, point2;
  double *coef, f;

  if (fftsound->numcoef2==0) return;
  SetWatchCursor(topLevel);

  PlayStopHard();

  fvec(coef, fftsound->numcoef2+1);
  for (ch=0; ch<lpc_samps_per_frame; ch++) {
    for (j=0; j<lpc_horiz_num; j++) {
      if (j>=fftsound->horiz_num) break;
      point=ch*(fftsound->numcoef2+2)*lpc_horiz_num+j*(fftsound->numcoef2+2);
      point2=ch*fftsound->horiz_num*fftsound->numchannels+j*fftsound->numchannels;
      for (i=0; i<fftsound->numcoef2+1; i++) coef[i]=lpc_coef[point+i];
      for (i=0; i<fftsound->numchannels; i++) {
        f=MEGFREQ_GET(point2+i)*TwoPi/fftsound->R;
	MEGAMP_PUT_DIV(point2+i,(lpamp(f, lpc_coef[point+fftsound->numcoef2+1], coef, fftsound->numcoef2)
          *0.1/fftsound->numcoef2));
	MEGAMP_PUT_MUL(point2+i,sqrt(lpc_coef[point+fftsound->numcoef2+1]));
      }
    }
  }
  free(coef);
  RedrawWin(fftsound);
  ResetCursor(topLevel);
}

void create_lpc2(struct FFTSound *fftsound){
  XmString text;

  /* CREATE LPC MENU */

  lpcButton=XtVaCreateManagedWidget(
    "lpcButton",xmCascadeButtonWidgetClass,menuBar,NULL);
  text=XmStringCreateLocalized("LPC");
  XtVaSetValues(lpcButton,XmNlabelString,text,NULL);
  lpcMenu=XmCreatePulldownMenu(menuBar,"lpcMenu",NULL,0);
  XtVaSetValues(lpcButton,XmNsubMenuId,lpcMenu,NULL);

  /* CREATE LPCMULT BUTTON */
  lpcMult=XtVaCreateManagedWidget(
    "lpcMult",xmPushButtonWidgetClass,lpcMenu,NULL);
  text=XmStringCreateLocalized("Filter current spectrum");
  XtVaSetValues(lpcMult,XmNlabelString,text,NULL);
  XtAddCallback(lpcMult,XmNactivateCallback,Lpcmult,fftsound);

  /* CREATE INVERT BUTTON */
  lpcInvert=XtVaCreateManagedWidget(
    "lpcInvert",xmPushButtonWidgetClass,lpcMenu,NULL);
  text=XmStringCreateLocalized("Inverse-filter spectrum (whiten)");
  XtVaSetValues(lpcInvert,XmNlabelString,text,NULL);
  XtAddCallback(lpcInvert,XmNactivateCallback,Lpcinvert,fftsound);

}

