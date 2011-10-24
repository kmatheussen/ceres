
#include "ceres.h"
#include "undo.h"


static Widget spotsButton, spotsMenu, spotsRestart, spotsKeep, spotsRemove;

extern int spotssize;

void SpotsRestart(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  numspots=0;
  RedrawWin(fftsound);
}

void SpotsKeep(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  int i, j, k, f, t, ch;
  long start;

  UNDO_addTransform(fftsound);

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    start=ch*fftsound->numchannels*fftsound->horiz_num;
    for (i=0; i<numspots; i++) {
      for (j=-spotssize; j<=spotssize; j++) {
        f=spotsf[i]+j;
        if ((f>0) && (f<fftsound->numchannels)) {
          for (k=-spotssize*fftsound->horiz_num/(double)thewidth; k<=spotssize*fftsound->horiz_num/(double)thewidth; k++) {
            t=spotst[i]+k;
            if ((t>=0) && (t<fftsound->horiz_num)){
              MEGAMP_NEG(t*fftsound->numchannels+f+start);
	    }
          }
        }
      }
    }

    for (i=0; i<fftsound->horiz_num; i++) {
      for (j=0; j<fftsound->numchannels; j++) {
        MEGAMP_PUT(start,
		   (MEGAMP_GET(start)<0?-MEGAMP_GET(start):0.)
		   );
	start++;
      }
    }
  }
  RedrawWin(fftsound);
}

void SpotsRemove(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  int i, j, k, f, t, ch;

  UNDO_addTransform(fftsound);

  for (ch=0; ch<fftsound->samps_per_frame; ch++)
    for (i=0; i<numspots; i++) {
      for (j=-spotssize; j<=spotssize; j++) {
        f=spotsf[i]+j;
        if ((f>0) && (f<fftsound->numchannels)) {
          for (k=-spotssize*fftsound->horiz_num/(double)thewidth; k<=spotssize*fftsound->horiz_num/(double)thewidth; k++) {
            t=spotst[i]+k;
            if ((t>=0) && (t<fftsound->horiz_num)){
              MEGAMP_PUT(
			 ch*fftsound->horiz_num*fftsound->numchannels+
			 t*fftsound->numchannels
			 +f
			 ,0.
			 );
	    }
          }
        }
      }
    }

  RedrawWin(fftsound);
}


void create_paint(struct FFTSound *fftsound){
  XmString text;

  /* CREATE SPOTS MENU */

  spotsButton=XtVaCreateManagedWidget(
    "spotsButton",xmCascadeButtonWidgetClass,menuBar,NULL);
  text=XmStringCreateLocalized("Paint");
  XtVaSetValues(spotsButton,XmNlabelString,text,NULL);
  spotsMenu=XmCreatePulldownMenu(menuBar,"spotsMenu",NULL,0);
  XtVaSetValues(spotsButton,XmNsubMenuId,spotsMenu,NULL);

  /* CREATE RESTART BUTTON */
  spotsRestart=XtVaCreateManagedWidget(
    "restartButton",xmPushButtonWidgetClass,spotsMenu,NULL);
  text=XmStringCreateLocalized("Restart");
  XtVaSetValues(spotsRestart,XmNlabelString,text,NULL);
  XtAddCallback(spotsRestart,XmNactivateCallback,SpotsRestart,fftsound);

  /* CREATE KEEP BUTTON */
  spotsKeep=XtVaCreateManagedWidget(
    "keepButton",xmPushButtonWidgetClass,spotsMenu,NULL);
  text=XmStringCreateLocalized("Keep");
  XtVaSetValues(spotsKeep,XmNlabelString,text,NULL);
  XtAddCallback(spotsKeep,XmNactivateCallback,SpotsKeep,fftsound);

  /* CREATE REMOVE BUTTON */
  spotsRemove=XtVaCreateManagedWidget(
    "removeButton",xmPushButtonWidgetClass,spotsMenu,NULL);
  text=XmStringCreateLocalized("Remove");
  XtVaSetValues(spotsRemove,XmNlabelString,text,NULL);
  XtAddCallback(spotsRemove,XmNactivateCallback,SpotsRemove,fftsound);


}

