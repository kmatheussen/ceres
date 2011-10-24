
#include <unistd.h>
#include <sys/time.h>
#include "ceres.h"
#include "gui.h"
#include "play.h"
#include <pthread.h>
#include "fftsound.h"
#include "phasevocoder.h"
#include "rsynthdata.h"
#include "config.h"
#include "undo.h"

#define USE_PREVIEW 0

extern pthread_t pthread;
pid_t mainpid;

static int transformnum=0;

void GUI_GeneralManage(Widget w,XtPointer client, XtPointer call)
{
  struct CTransform *ctransform=(struct CTransform *)client;

  Widget dialog=ctransform->parentwidget;
  int lokke;

  for(lokke=0;lokke<3;lokke++){
    if(
       ctransform->funcs[lokke]!=NULL && XmToggleButtonGetState(ctransform->funcs[lokke]->togglewidget)
       )
      {
	SetFuncMinMax(lokke,ctransform->funcs[lokke]->minval,ctransform->funcs[lokke]->maxval);
	SetVisibleFunc(
		       lokke+1,
		       1);
      }else{
	SetVisibleFunc(
		       lokke+1,
		       0);
      }
  }

  RestoreWin(ctransform->fftsound);

  XtManageChild(dialog);
}

void  Cancel(Widget w,XtPointer client, XtPointer call)
{
  Widget dialog=(Widget)client;
  XtUnmanageChild(dialog);
}

void  GUI_Cancel(Widget w,XtPointer client, XtPointer call)
{
  struct CTransform *ctransform=(struct CTransform *)client;

  Widget dialog=ctransform->parentwidget;

  SetVisibleFunc(1,2);
  SetVisibleFunc(2,2);
  SetVisibleFunc(3,2);
  RestoreWin(ctransform->fftsound);

  XtUnmanageChild(dialog);
}

Widget a,b;

bool GUI_UpdateProgress(struct CTransform *ctransform,int startval,int nowval,int endval){

#if 0
  struct timeval tv;
  //  XEvent event;

  gettimeofday(&tv,NULL);

  /* Update ~50 times a second. */
  if(abs(tv.tv_usec-ctransform->lasttime)>20000){
    /*
    if(XtAppPeekEvent(app_context, &event)){
      printf("event->display: %x %x %x\n",event.xany.window,a->core.window,b->core.window);
    }
    */
    while (XtAppPending(app_context)) {
      XtAppProcessEvent(app_context,XtIMXEvent|XtIMTimer|XtIMAlternateInput);
    }
    ctransform->lasttime=tv.tv_usec;
  }
#endif

#if 1
  int pr=100*(nowval-startval)/(endval-startval);
  if(pr!=ctransform->lastprogress && getpid()==mainpid){
    XtVaSetValues(progressScale, XmNvalue, pr, NULL);
    XmUpdateDisplay(topLevel);
    ctransform->lastprogress=pr;
  }
#endif

  return true;
}


void GUI_GeneralPreview(Widget w,XtPointer client, XtPointer call)
{
  struct CTransform *ctransform=(struct CTransform *)client;
  struct FFTSound *copy;
  int length;

  PlayStopHard();

  if( ! DA_initialized(ctransform->fftsound)){
    fprintf(stderr,"No Sound loaded\n");
    return;
  }


  XmUpdateDisplay(w);
  SetWatchCursor(topLevel);

  if(ctransform->doprogress==true){
    XtVaSetValues(progressScale, XmNvalue, 0, NULL);
    XtManageChild(progressDialog);
  }

  while (XtAppPending(app_context)) {
     XtAppProcessEvent(app_context,XtIMXEvent|XtIMTimer|XtIMAlternateInput);
  }


  length=min(areat2-areat1,CONFIG_getMaxPreviewLength(ctransform->fftsound));

  copy=makeFFTSoundCopy(ctransform->fftsound,areat1,areat1+length);
  copyFFTSound(ctransform->fftsound,copy,areat1);
  //  copy=ctransform->fftsound;
  (*ctransform->Transform)(ctransform,ctransform->pointer,copy,0,length,areat1);

  Play(copy,0,length);


  /*
  if(pthread_create(&pthread,NULL,DoPlay,copy)!=0){
    fprintf(stderr,"Could not make pthread\n");
  }
  */

  //   RedrawWin(ctransform->fftsound);
  if(ctransform->doprogress==true) XtUnmanageChild(progressDialog);
  ResetCursor(topLevel);

}

extern pthread_t pthread;
extern bool dontstop;

static bool gui_obank;
static int gui_addint;

static void *GUI_DOGeneralRPreview(void *pointer)
{
  struct CTransform *ctransform=(struct CTransform*)pointer;
  int lokke;
  struct PlayStruct ps;
  struct RSynthData *rsd;
  bool obank;
  int addint;

  ps.usejack=ctransform->usejack;

  if(ps.usejack==false && InitPlay()==FALSE){
    return NULL;
  }
  dontstop=true;

  obank=gui_obank;
  addint=gui_addint;

  ps.fftsound=makeFFTSoundCopy(ctransform->fftsound,0,addint);
  rsd=getRSynthData(ps.fftsound);


  if(ps.usejack==true){
    ps.playdata=JackOpenPlay(ps.fftsound);
  }else{
    ps.playdata=OpenPlay(ps.fftsound);
  }

  if(ps.playdata!=NULL){
    for(
	lokke=areat1;
	lokke<areat2;
	lokke+=addint
	)
      {
	copyFFTSound(ctransform->fftsound,ps.fftsound,lokke);
	(*ctransform->Transform)(ctransform,ctransform->pointer,ps.fftsound,0,min(addint,areat2-lokke),lokke);
	makeWaves(
		  ps.fftsound,
		  rsd,
		  PlayWaveConsumer,
		  NULL,
		  &ps,
		  0,
		  min(addint,areat2-lokke),
		  obank,
                  CONFIG_getBool("keep_formants")
		  );
	if(dontstop==false) break;
      }

    if(ps.usejack==true){
      JackClosePlay(ps.playdata);
    }else{
      ClosePlay(ps.playdata);
    }
  }

  returnRSynthData(rsd);

  freeFFTSound(ps.fftsound);

  return NULL;

}

void GUI_GeneralRPreview(Widget w,XtPointer client, XtPointer call)
{
  struct CTransform *ctransform=(struct CTransform *)client;

  if( ! DA_initialized(ctransform->fftsound)){
    fprintf(stderr,"No Sound loaded.\n");
    return;
  }

  PlayStopHard();

  gui_obank=CONFIG_getBool("pt_additive");
  gui_addint=CONFIG_getInt("num_frames_realtime_preview");

  if(CONFIG_getBool("use_jack")==true){
#ifndef USEJACK
    fprintf(stderr,"Jack support is not compiled into the program.\n");
    return;
#else
    ctransform->usejack=true;
#endif
  }else{
    ctransform->usejack=false;
  }

  if(pthread_create(&pthread,NULL,GUI_DOGeneralRPreview,ctransform)!=0){
    fprintf(stderr,"Could not make pthread\n");
  }

}

void GUI_GeneralStop(Widget w,XtPointer client, XtPointer call){
  PlayStop();
}


void GUI_GeneralOk(Widget w,XtPointer client, XtPointer call)
{
  struct CTransform *ctransform=(struct CTransform *)client;

  if( ! DA_initialized(ctransform->fftsound)){
    fprintf(stderr,"No Sound loaded\n");
    return;
  }

  XmUpdateDisplay(w);
  SetWatchCursor(topLevel);

  if(ctransform->doprogress==true){
    XtVaSetValues(progressScale, XmNvalue, 0, NULL);
    XtManageChild(progressDialog);
  }

  while (XtAppPending(app_context)) {
     XtAppProcessEvent(app_context,XtIMXEvent|XtIMTimer|XtIMAlternateInput);
  }

  UNDO_addTransform(ctransform->fftsound);

  (*ctransform->Transform)(ctransform,ctransform->pointer,ctransform->fftsound,areat1,areat2,0);

  SetVisibleFunc(1,2);
  SetVisibleFunc(2,2);
  SetVisibleFunc(3,2);

  RedrawWin(ctransform->fftsound);
  if(ctransform->doprogress==true) XtUnmanageChild(progressDialog);
  ResetCursor(topLevel);

  XtUnmanageChild(ctransform->parentwidget);

}

struct GeneralMult_pr_horiz{
  struct FFTSound *fftsound;
  double mult_pr_horiz;
};

void GUI_generalMult_pr_horiz(double val,void *pointer){
  struct GeneralMult_pr_horiz *sv=(struct GeneralMult_pr_horiz*)pointer;
  sv->mult_pr_horiz=pow(val,sv->fftsound->duration/sv->fftsound->horiz_num);
}

/*************************************************/
/* The pitch2freq function is taken from Ceres3. */
/*************************************************/

double GUI_pitch2freq(char *s){
  double r=0.0;
  double o=0.0;
  int c;
  /*55.0 = A0 ; equal temperament = 1.0594631*/
  if ((s[0]=='c')||(s[0]=='C'))r=32.703;
  if( ((s[0]=='c')||(s[0]=='C'))&&(s[1]=='b') )r=30.8677;
  if( ((s[0]=='c')||(s[0]=='C'))&&(s[1]=='#') )r=34.6478;
  
  if((s[0]=='d')||(s[0]=='D'))r=36.7081;
  if( ((s[0]=='d')||(s[0]=='D'))&&(s[1]=='b') )r=34.6478;
  if( ((s[0]=='d')||(s[0]=='D'))&&(s[1]=='#') )r=38.8909;
  
  if  ((s[0]=='e')||(s[0]=='E')) r=41.2034;
  if( ((s[0]=='e')||(s[0]=='E'))&&(s[1]=='b') )r=38.8909;
  if( ((s[0]=='e')||(s[0]=='E'))&&(s[1]=='#') )r=43.6535;
  
  if ((s[0]=='f')||(s[0]=='F')) r=43.6535; 
  if (((s[0]=='f')||(s[0]=='F'))&&(s[1]=='b') )r=41.2034;
  if (((s[0]=='f')||(s[0]=='F'))&&(s[1]=='#') )r=46.2493;
  
  if ((s[0]=='g')||(s[0]=='G')) r=48.999;
  if (((s[0]=='g')||(s[0]=='G'))&&(s[1]=='b') )r=46.2493;
  if (((s[0]=='g')||(s[0]=='G'))&&(s[1]=='#') )r=51.913;
  
  if ((s[0]=='a')||(s[0]=='A')) r=55.0;
  if (((s[0]=='a')||(s[0]=='A'))&&(s[1]=='b') )r=51.913;
  if (((s[0]=='a')||(s[0]=='A'))&&(s[1]=='#') )r=58.2705;
  
  if ((s[0]=='b')||(s[0]=='B')) r=61.7354;
  if (((s[0]=='b')||(s[0]=='B'))&&(s[1]=='b') )r=58.2705;
  if (((s[0]=='b')||(s[0]=='B'))&&(s[1]=='#') )r=65.4064;


  c = strlen(s)-1;
  if(c>0){
    if(s[c]=='0')o=1.;if(s[c]=='1')o=2.;if(s[c]=='2')o=4.;
    if(s[c]=='3')o=8.;if(s[c]=='4')o=16.;if(s[c]=='5')o=32.;
    if(s[c]=='6')o=64.;if(s[c]=='7')o=128.;if(s[c]=='8')o=256.;
    if(s[c]=='9')o=512.;
    if( (s[c]=='1')&&(s[c-1]=='-') )o=.5;
    /*
      printf("%d\t%f\t%f\t%f\n", c,r,o,r*o);
    */
    return r*(double)o;
  }

  return 440.0f;
}

char *GUI_freq2pitch(double freq){
  char *res1;
  char *res2;

  char octave;
  char *notes[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
  int note;
  double temp;

  res1=malloc(4);
  res2=malloc(4);
  sprintf(res2,"-C0");

  for(octave='0';octave<='9';octave++){
    for(note=0;note<12;note++){
      sprintf(res1,"%s%c",notes[note],octave);
      temp=GUI_pitch2freq(res1);
      if(temp==freq){
	sprintf(res2,"%s",res1);
	goto foundit;
      }
      if(temp>freq) goto foundit;
      sprintf(res2,"%s",res1);
    }
  }
 foundit:
  free(res1);
  return res2;
}


void GUI_CallCallBackDouble(struct CSlider *cslider,double val){
  if(cslider->callbackfunc!=NULL){
    if(cslider->isfunc){
      (*((void (*)(double val,void *pointer))(cslider->callbackfunc)))(val,cslider->pointer);
    }else{
      *(double*)(cslider->callbackfunc)=val;
    }
  }
}
void GUI_CallCallBackInt(struct CSlider *cslider,int val){
  if(cslider->callbackfunc!=NULL){
    if(cslider->isfunc){
      (*((void (*)(int val,void *pointer))(cslider->callbackfunc)))(val,cslider->pointer);
    }else{
      *(int*)(cslider->callbackfunc)=val;
    }
  }
}

extern int seltoggle;
extern bool inited;

void GUI_CallCallBackToggle(struct CSlider *cslider,bool val){
  if(cslider->callbackfunc!=NULL){
    if(cslider->isfunc){
      (*((void (*)(bool val,void *pointer))(cslider->callbackfunc)))(val,cslider->pointer);
    }else{
      *(bool*)(cslider->callbackfunc)=val;
    }
  }

  if(inited==true && cslider->graphnum>0){
    seltoggle=cslider->graphnum;
    //    RedrawWin(cslider->fftsound);
    SetFuncMinMax(cslider->graphnum-1,cslider->minval,cslider->maxval);
    SetVisibleFunc(cslider->graphnum,val==true?1:0);
    RestoreWin(cslider->fftsound);
  }
}

void GUI_GeneralFreqTextField(Widget w,XtPointer client,XtPointer call);


void GUI_GeneralToggle(Widget w,XtPointer client,XtPointer call){
  struct CSlider *cslider=(struct CSlider *)client;
  Boolean val=XmToggleButtonGetState(cslider->togglewidget);
  GUI_CallCallBackToggle(cslider,val==True?true:false);
}

void GUI_GeneralTextField(Widget w,XtPointer client,XtPointer call){
  struct CSlider *cslider=(struct CSlider *)client;
  char *valstring=XmTextGetString(cslider->textwidget);
  char *freqtext;
  double value=atof(valstring);
  XtFree(valstring);


  if(cslider->islog){
    double ai;
    if(value==0.0f) value=0.000001f;
    ai=((log(value)/log(cslider->maxval))-cslider->logval)*(1/(1-cslider->logval));
    XtVaSetValues(cslider->sliderwidget,
		  XmNvalue,
#ifdef USELESSTIF
		  MAXSLIDERVAL-(int)min(MAXSLIDERVAL,max(0,(ai*MAXSLIDERVAL))),
#else
		  (int)min(MAXSLIDERVAL,max(0,(ai*MAXSLIDERVAL))),
#endif
		  NULL
		  );
  }else{
    XtVaSetValues(cslider->sliderwidget,
		  XmNvalue,
#ifdef USELESSTIF
		  MAXSLIDERVAL-min(MAXSLIDERVAL,(int)(MAXSLIDERVAL*(value-cslider->minval)/(cslider->maxval-cslider->minval))),
#else
		  min(MAXSLIDERVAL,(int)(MAXSLIDERVAL*(value-cslider->minval)/(cslider->maxval-cslider->minval))),
#endif
		  NULL
		  );
  }

  if(cslider->freqtextwidget!=NULL){
    freqtext=GUI_freq2pitch(value);
    XtRemoveCallback(cslider->freqtextwidget,XmNvalueChangedCallback,GUI_GeneralFreqTextField,cslider);
    XmTextSetString(cslider->freqtextwidget,freqtext);
    XtAddCallback(cslider->freqtextwidget,XmNvalueChangedCallback,GUI_GeneralFreqTextField,cslider);
    free(freqtext);
  }

  if(cslider->isdouble){
    GUI_CallCallBackDouble(cslider,value);
  }else{
    GUI_CallCallBackInt(cslider,(int)value);
  }

}


void GUI_GeneralFreqTextField(Widget w,XtPointer client,XtPointer call){
  struct CSlider *cslider=(struct CSlider *)client;
  char *valstring=XmTextGetString(cslider->freqtextwidget);
  char text[20];
  double value=GUI_pitch2freq(valstring);
  XtFree(valstring);

  if(cslider->islog){
    double ai;
    if(value==0.0f) value=0.000001f;
    ai=((log(value)/log(cslider->maxval))-cslider->logval)*(1/(1-cslider->logval));
    XtVaSetValues(cslider->sliderwidget,
		  XmNvalue,
#ifdef USELESSTIF
		  MAXSLIDERVAL-(int)min(MAXSLIDERVAL,max(0,(ai*MAXSLIDERVAL))),
#else
		  (int)min(MAXSLIDERVAL,max(0,(ai*MAXSLIDERVAL))),
#endif
		  NULL
		  );
  }else{
    XtVaSetValues(cslider->sliderwidget,
		  XmNvalue,
#ifdef USELESSTIF
		  MAXSLIDERVAL-min(MAXSLIDERVAL,(int)(MAXSLIDERVAL*(value-cslider->minval)/(cslider->maxval-cslider->minval))),
#else
		  min(MAXSLIDERVAL,(int)(MAXSLIDERVAL*(value-cslider->minval)/(cslider->maxval-cslider->minval))),
#endif
		  NULL
		  );
  }

  if(cslider->isdouble){
    sprintf(text,"%.*f",cslider->resolution,value);
    GUI_CallCallBackDouble(cslider,value);
  }else{
    sprintf(text,"%d",(int)value);
    GUI_CallCallBackInt(cslider,(int)value);
  }
  
  XtRemoveCallback(cslider->textwidget,XmNvalueChangedCallback,GUI_GeneralTextField,cslider);
  XmTextSetString(cslider->textwidget,text);
  XtAddCallback(cslider->textwidget,XmNvalueChangedCallback,GUI_GeneralTextField,cslider);
}

#define DASE 2.718281828f

void GUI_GeneralSlider(Widget w,XtPointer client,XtPointer call){
  char text[20];
  struct CSlider *cslider=(struct CSlider *)client;
  char *freqtext;
  double value;
  double slidervalue;

  XmScrollBarCallbackStruct *wx=(XmScrollBarCallbackStruct *)call;

#ifdef USELESSTIF
  slidervalue=(double)(MAXSLIDERVAL-wx->value);
#else
  slidervalue=(double)wx->value;
#endif

  if(cslider->islog){
    double s=slidervalue/MAXSLIDERVAL;
    value=pow(DASE,log(cslider->maxval)*(s-(s*cslider->logval)+cslider->logval));
  }else{
    value=(slidervalue*(cslider->maxval-cslider->minval)/MAXSLIDERVAL)+cslider->minval;
  }

  if(cslider->isdouble){
    sprintf(text,"%.*f",cslider->resolution,value);
    GUI_CallCallBackDouble(cslider,value);
  }else{
    sprintf(text,"%d",(int)value);
    GUI_CallCallBackInt(cslider,(int)value);
  }
  
  XtRemoveCallback(cslider->textwidget,XmNvalueChangedCallback,GUI_GeneralTextField,cslider);
  if(cslider->freqtextwidget!=NULL)
    XtRemoveCallback(cslider->freqtextwidget,XmNvalueChangedCallback,GUI_GeneralFreqTextField,cslider);

  XmTextSetString(cslider->textwidget,text);

  if(cslider->freqtextwidget!=NULL){
    freqtext=GUI_freq2pitch(value);
    XmTextSetString(cslider->freqtextwidget,freqtext);
    free(freqtext);
  }

  XtAddCallback(cslider->textwidget,XmNvalueChangedCallback,GUI_GeneralTextField,cslider);
  if(cslider->freqtextwidget!=NULL)
    XtAddCallback(cslider->freqtextwidget,XmNvalueChangedCallback,GUI_GeneralFreqTextField,cslider);


  return;
}

int GUI_addYVarInput(struct VarInput *sm){
  sm->y+=32;
  return sm->y;
}


struct VarInput *GUI_MakeMenuButton(
				    struct FFTSound *fftsound,
				    void *pointer,
				    Widget menu,
				    char *name,
				    int x1,int x2
				    )
{

  struct VarInput *sm=calloc(1,sizeof(struct VarInput));
  XmString text=XmStringCreateLocalized(name);
  char *keys[]={"0","1","2","3","4","5","6","7","8","9","Q","W","E","R","T","Y","U","I","O","A","S","D"};
  char *temp1;
  char temp2[32];

  temp1=keys[transformnum];
  sprintf(temp2,"<Key>%s",temp1);
  transformnum++;

  a=sm->managedwidget=XtVaCreateManagedWidget(
				       "something",
				       xmPushButtonWidgetClass,menu,
				       XmNmnemonic,XStringToKeysym(temp1),XmNaccelerator,temp2,
				       XmNacceleratorText,XmStringCreateLocalized(temp1),
				       NULL
				       );

  XtVaSetValues(sm->managedwidget,XmNlabelString,text,NULL);

  sm->parentwidget=XmCreateBulletinBoardDialog(mainWindow,"sfform",NULL,0);
  XtVaSetValues(sm->parentwidget,XmNautoUnmanage,FALSE,NULL);

  sm->x1=x1;
  sm->x2=x2;
  sm->y=4;
  sm->radio=NULL;
  sm->fftsound=fftsound;
  sm->pointer=pointer;

  return sm;
}

void GUI_CloseMenuButtonOkCancelPreview(
			  struct VarInput *sm,
			  void (*func)(struct CTransform *ctransform,void *pointer,struct FFTSound *fftsound,int area_start,int area_end,int funcstartdiff),
			  bool doprogress
			  )
{
  int y=GUI_addYVarInput(sm);

  struct CTransform *ctransform=calloc(1,sizeof(struct CTransform));

  Widget okbutton=XtVaCreateManagedWidget("alabel",xmPushButtonWidgetClass,sm->parentwidget,NULL);
  Widget cancelbutton=XtVaCreateManagedWidget("alabel",xmPushButtonWidgetClass,sm->parentwidget,NULL);
  Widget previewbutton=XtVaCreateManagedWidget("alabel",xmPushButtonWidgetClass,sm->parentwidget,NULL);
#if USE_PREVIEW
  Widget rpreviewbutton=XtVaCreateManagedWidget("alabel",xmPushButtonWidgetClass,sm->parentwidget,NULL);
#endif
  Widget stopbutton=XtVaCreateManagedWidget("alabel",xmPushButtonWidgetClass,sm->parentwidget,NULL);

  XmString text=XmStringCreateLocalized("Ok");
  XtVaSetValues(
		okbutton,
		XmNlabelString,text,
		XmNx,sm->x1,
		XmNy,y,
		XmNwidth,80,
		NULL
		);

  text=XmStringCreateLocalized("Cancel");
  XtVaSetValues(
		cancelbutton,
		XmNlabelString,text,
		XmNx,sm->x1+120,
		XmNy,y,
		XmNwidth,80,
		NULL
		);

  text=XmStringCreateLocalized("Preview");
  XtVaSetValues(
		previewbutton,
		XmNlabelString,text,
		XmNx,sm->x1+220,
		XmNy,y,
		XmNwidth,80,
		NULL
		);

#if USE_PREVIEW
  text=XmStringCreateLocalized("Preview Realtime");
  XtVaSetValues(
		rpreviewbutton,
		XmNlabelString,text,
		XmNx,sm->x1+320,
		XmNy,y,
		XmNwidth,140,
		NULL
		);
#endif

  text=XmStringCreateLocalized("Stop");
  XtVaSetValues(
		stopbutton,
		XmNlabelString,text,
		XmNx,sm->x1+500,
		XmNy,y,
		XmNwidth,80,
		NULL
		);

  XtVaSetValues(
		sm->parentwidget,
		XmNdialogTitle,text,
		XmNcancelButton,cancelbutton,
		XmNdefaultButton,okbutton,
		NULL);

  ctransform->doprogress=doprogress;
  ctransform->Transform=func;
  ctransform->fftsound=sm->fftsound;
  ctransform->pointer=sm->pointer;
  ctransform->parentwidget=sm->parentwidget;
  ctransform->funcs[0]=sm->funcs[0];
  ctransform->funcs[1]=sm->funcs[1];
  ctransform->funcs[2]=sm->funcs[2];

  XtAddCallback(okbutton,XmNactivateCallback,GUI_GeneralOk,ctransform);
  XtAddCallback(cancelbutton,XmNactivateCallback,GUI_Cancel,ctransform);
  XtAddCallback(previewbutton,XmNactivateCallback,GUI_GeneralPreview,ctransform);
#if USE_PREVIEW
  XtAddCallback(rpreviewbutton,XmNactivateCallback,GUI_GeneralRPreview,ctransform);
#endif
  XtAddCallback(stopbutton,XmNactivateCallback,GUI_GeneralStop,NULL);


  XtAddCallback(sm->managedwidget,XmNactivateCallback,GUI_GeneralManage,ctransform);

  free(sm);
}

void GUI_setVarInputRadioStart(struct VarInput *sm,Boolean onoff){
  sm->radio=XtVaCreateManagedWidget(
				     "label",
				     xmRowColumnWidgetClass,sm->parentwidget,
				     NULL
				     );

  XtVaSetValues(
		sm->radio,
		XmNy,sm->y+32,
		XmNradioBehavior,onoff,
		XmNradioAlwaysOne,True,
		NULL
		);
}

void GUI_setVarInputRadioStop(struct VarInput *sm){
  sm->radio=NULL;
}

struct CSlider *GUI_MakeInputVarGeneral(
		struct VarInput *sm,
		char *vartext,
		//		Widget *textfieldwidget,
		void *callbackfunc,
		Boolean isfunc, // If callbackfunc is a function and not a pointer to a variable.
		Boolean isdouble,
		Boolean islog,
		double minval,
		double maxval,
		int resolution,
		double dasdefault
		)
{
  XmString text;
  char text2[200];
  struct CSlider *cslider;

  Widget sliderwidget;
  Widget label;
  Widget textfieldwidget;

  int y=GUI_addYVarInput(sm);

  label=XtVaCreateManagedWidget("alabel",xmLabelWidgetClass,
				     sm->parentwidget,NULL);
  XtVaSetValues(label,XmNlabelString,XmStringCreateLocalized(vartext),XmNx,sm->x1,XmNy,y+4,NULL);

  if(isdouble){
    sprintf(text2,"%.*f",resolution,dasdefault);
  }else{
    sprintf(text2,"%d",(int)dasdefault);
  }

  textfieldwidget=XtVaCreateManagedWidget("textfield",
					 xmTextFieldWidgetClass,sm->parentwidget,NULL);

  XtVaSetValues(textfieldwidget,XmNwidth,70,XmNx,sm->x2,XmNy,y,
		XmNvalue,text2,NULL);

  sliderwidget=XtVaCreateManagedWidget("widget",xmScaleWidgetClass,sm->parentwidget,NULL);
  cslider=calloc(1,sizeof(struct CSlider));
  cslider->isdouble=isdouble;
  cslider->islog=islog;
  cslider->fftsound=sm->fftsound;
  cslider->isfunc=isfunc;
  cslider->pointer=sm->pointer;

  if(islog){
    if(minval==0.0f) minval=0.00001f;
    if(maxval==0.0f) maxval=-0.00001f;
  }
  cslider->minval=minval;
  cslider->maxval=maxval;
  cslider->resolution=resolution;
  cslider->textwidget=textfieldwidget;
  cslider->sliderwidget=sliderwidget;
  cslider->callbackfunc=callbackfunc;

  if(islog){
    cslider->logval=log(minval)/log(maxval);
  }

  text=XmStringCreateLocalized("slider");
  XtVaSetValues(sliderwidget,XmNlabelString,text,XmNx,sm->x2+80,XmNy,y+5,XmNwidth,400,
		XmNminimum,0,XmNmaximum,MAXSLIDERVAL,
		XmNvalue,0,
		XmNorientation,XmHORIZONTAL,
		NULL
		);

  if(cslider->islog){
    double ai=((log(dasdefault==0.0f?0.000001f:dasdefault)/log(cslider->maxval))-cslider->logval)*(1/(1-cslider->logval));
    XtVaSetValues(cslider->sliderwidget,
		  XmNvalue,
#ifdef USELESSTIF
		  MAXSLIDERVAL-(int)min(MAXSLIDERVAL,max(0,(ai*MAXSLIDERVAL))),
#else
		  (int)min(MAXSLIDERVAL,max(0,(ai*MAXSLIDERVAL))),
#endif
		  NULL
		  );
  }else{
    XtVaSetValues(cslider->sliderwidget,
		  XmNvalue,
#ifdef USELESSTIF
		  MAXSLIDERVAL-min(MAXSLIDERVAL,(int)(MAXSLIDERVAL*(dasdefault-cslider->minval)/(cslider->maxval-cslider->minval))),
#else
		  min(MAXSLIDERVAL,(int)(MAXSLIDERVAL*(dasdefault-cslider->minval)/(cslider->maxval-cslider->minval))),
#endif
		  NULL
		  );
  }

  XtAddCallback(sliderwidget,XmNdragCallback,GUI_GeneralSlider,cslider);
  XtAddCallback(textfieldwidget,XmNvalueChangedCallback,GUI_GeneralTextField,cslider);

  if(isdouble){
    GUI_CallCallBackDouble(cslider,dasdefault);
  }else{
    GUI_CallCallBackInt(cslider,(int)dasdefault);
  }

  return cslider;
}

struct CSlider *GUI_MakeInputVarDouble(
		struct VarInput *sm,
		char *vartext,
		double *func,
		//		Widget *textfieldwidget,
		double minval,
		double maxval,
		double dasdefault
		)
{
  return GUI_MakeInputVarGeneral(sm,vartext,func,FALSE,TRUE,FALSE,minval,maxval,2,dasdefault);
}

struct CSlider *GUI_MakeInputVarDouble_func(
		struct VarInput *sm,
		char *vartext,
		void (*func)(double val,void *pointer),
		//		Widget *textfieldwidget,
		double minval,
		double maxval,
		double dasdefault
		)
{
  return GUI_MakeInputVarGeneral(sm,vartext,func,TRUE,TRUE,FALSE,minval,maxval,2,dasdefault);
}

struct CSlider *GUI_MakeInputVarLog(
		struct VarInput *sm,
		char *vartext,
		double *func,
		//		Widget *textfieldwidget,
		double minval,
		double maxval,
		double dasdefault
		)
{
  return GUI_MakeInputVarGeneral(sm,vartext,func,FALSE,TRUE,TRUE,minval,maxval,6,dasdefault);
}

struct CSlider *GUI_MakeInputVarLog_func(
		struct VarInput *sm,
		char *vartext,
		void (*func)(double val,void *pointer),
		//		Widget *textfieldwidget,
		double minval,
		double maxval,
		double dasdefault
		)
{
  return GUI_MakeInputVarGeneral(sm,vartext,func,TRUE,TRUE,TRUE,minval,maxval,6,dasdefault);
}

struct CSlider *GUI_MakeInputVarInt(
		struct VarInput *sm,
		char *vartext,
		//		Widget *textfieldwidget,
		int *func,
		int minval,
		int maxval,
		int dasdefault
		)
{
  return GUI_MakeInputVarGeneral(sm,vartext,func,FALSE,FALSE,FALSE,(double)minval,(double)maxval,2,(double)dasdefault);
}

struct CSlider *GUI_MakeInputVarInt_func(
		struct VarInput *sm,
		char *vartext,
		//		Widget *textfieldwidget,
		void (*func)(int val,void *pointer),
		int minval,
		int maxval,
		int dasdefault
		)
{
  return GUI_MakeInputVarGeneral(sm,vartext,func,TRUE,FALSE,FALSE,(double)minval,(double)maxval,2,(double)dasdefault);
}

void GUI_MakeInputVarToggle_general(
			struct VarInput *sm,
			char *vartext,
			//			Widget *togglewidget,
			void *func,
			Boolean isfunc,
			bool dasdefault,
			int graphnum,
			float minval,
			float maxval
			)
{
  XmString text;
  int y=GUI_addYVarInput(sm);
  struct CSlider *cslider=calloc(1,sizeof(struct CSlider));
  cslider->callbackfunc=func;

  text=XmStringCreateLocalized(vartext);

  cslider->togglewidget=XtVaCreateManagedWidget(
				       "alabel",
				       xmToggleButtonWidgetClass,
				       sm->radio!=NULL?sm->radio:sm->parentwidget,
				       NULL
				       );
  cslider->fftsound=sm->fftsound;
  cslider->graphnum=graphnum;
  cslider->isfunc=isfunc;
  cslider->pointer=sm->pointer;

  cslider->minval=minval;
  cslider->maxval=maxval;

  if(graphnum>0) sm->funcs[graphnum-1]=cslider;

  XtVaSetValues(
		cslider->togglewidget,
		XmNlabelString,text,
		XmNset,dasdefault,
		sm->radio!=NULL?NULL:XmNy,y,
		NULL
		);

  XtAddCallback(cslider->togglewidget,XmNvalueChangedCallback,GUI_GeneralToggle,cslider);
  GUI_CallCallBackToggle(cslider,dasdefault);
}

void GUI_MakeInputVarToggle_func(
			struct VarInput *sm,
			char *vartext,
			//			Widget *togglewidget,
			void (*func)(bool val,void *pointer),
			bool dasdefault
			)
{
  GUI_MakeInputVarToggle_general(sm,vartext,func,TRUE,dasdefault,0,0.0f,0.0f);
}

void GUI_MakeInputVarToggle(
			struct VarInput *sm,
			char *vartext,
			//			Widget *togglewidget,
			bool *func,
			bool dasdefault
			)
{
  GUI_MakeInputVarToggle_general(sm,vartext,func,FALSE,dasdefault,0,0.0f,0.0f);
}

void GUI_MakeInputVarToggle_graph(
				  struct VarInput *sm,
				  char *vartext,
				  bool *func,
				  bool dasdefault,
				  int graphnum,
				  float minval,
				  float maxval
				  )
{
  char *newvartext=malloc(strlen(vartext)+100);
  sprintf(newvartext,"%s (%.2f - %.2f)",vartext,minval,maxval);
  GUI_MakeInputVarToggle_general(sm,newvartext,func,FALSE,dasdefault,graphnum,minval,maxval);
}

			  
void GUI_MakeInputVarFreq(
		struct VarInput *sm,
		char *vartext,
		//		Widget *textfieldwidget,
		//		void (*func)(double val,void *pointer),
		double *func,
		double minval,
		double maxval,
		double dasdefault
		)
{
  Widget freqtextfieldwidget;
  struct CSlider *cslider=GUI_MakeInputVarGeneral(sm,vartext,func,FALSE,TRUE,TRUE,minval,maxval,6,dasdefault);
  char *freqtext;

  freqtextfieldwidget=XtVaCreateManagedWidget("freqTextfield",
					 xmTextFieldWidgetClass,sm->parentwidget,NULL);

  freqtext=GUI_freq2pitch(dasdefault);

  XtVaSetValues(freqtextfieldwidget,XmNwidth,60,XmNx,sm->x2-80,XmNy,sm->y,
		XmNvalue,freqtext,NULL);

  free(freqtext);

  b=cslider->freqtextwidget=freqtextfieldwidget;

  XtAddCallback(freqtextfieldwidget,XmNvalueChangedCallback,GUI_GeneralFreqTextField,cslider);

  GUI_CallCallBackDouble(cslider,dasdefault);
}


void create_gui(struct FFTSound *fftsound){
  mainpid=getpid();
}

