

#include "ceres.h"
#include "play.h"

static Widget exportButton, exportMenu, tocsound, tomatlab, toparmerud, tostf, toltas, topeak;
static Widget parmerudRowCol, parmL1, parmL2, parmL3, parmL4,
  parmT1, parmT2, parmT3, parmT4;
static Widget tocsoundFileBox,tomatlabFileBox, toparmerudFileBox;

static Widget tostfFileBox, toltasFileBox, topeakFileBox;

static Widget csoundWarning, stfWarning, ltasWarning;

void Tocsound(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(tocsoundFileBox);
}

void TocsoundOk(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call; 
  char *filename;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
  FILE *scorefile;
  int i, j;
  long point;
  double distance=stretchfac*fftsound->duration/(double)fftsound->horiz_num;
  double dur=stretchfac/fftsound->binfreq;
  double onset;

  XmStringGetLtoR(calldata->value, charset, &filename);
  scorefile=fopen(filename,"w");
  if (scorefile==NULL) {
    XtManageChild(csoundWarning);
    XtFree(filename);
    return;
  }

  PlayStopHard();

  point=areat1*fftsound->numchannels;
  for (i=areat1; i<areat2; i++) {
     onset=i*distance;
     for (j=areaf1; j<areaf2; j++) {
       if (
	   (MEGAMP_GET(point+j)>0.) 
	   && (MEGFREQ_GET(point+j)>20.)
	   )
         fprintf(scorefile,"i1    %8.4f    %8.4f    %7.2f    %7.2f\n",
		 onset, dur, MEGAMP_GET(point+j)*10000000., MEGFREQ_GET(point+j));
     }
     point+=fftsound->numchannels;
  }

  fprintf(scorefile,"e\n");

  fclose(scorefile);
  XtFree(filename);
  XtUnmanageChild(tocsoundFileBox);
}

void Tomatlab(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(tomatlabFileBox);
}

void TomatlabOk(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call; 
  char *filename;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
  FILE *scorefile;
  int i, j;
  long point;
  double distance=stretchfac*fftsound->duration/(double)fftsound->horiz_num;
  double onset;

  XmStringGetLtoR(calldata->value, charset, &filename);
  scorefile=fopen(filename,"w");
  if (scorefile==NULL) {
    XtManageChild(csoundWarning);
    XtFree(filename);
    return;
  }

  PlayStopHard();

  point=areat1*fftsound->numchannels;
  for (i=areat1; i<areat2; i++) {
     onset=i*distance;
     for (j=areaf1; j<areaf2; j++) {
       if ((MEGAMP_GET(point+j)>0.) && (MEGFREQ_GET(point+j)>20.))
         fprintf(scorefile,"%8.4f %7.2f %7.2f\n",
           onset, MEGAMP_GET(point+j)*10000000., MEGFREQ_GET(point+j));
     }
     point+=fftsound->numchannels;
  }

  fprintf(scorefile,"e\n");

  fclose(scorefile);
  XtFree(filename);
  XtUnmanageChild(tomatlabFileBox);
}

void Toparmerud(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(toparmerudFileBox);
}

void ToparmerudOk(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call; 
  char *filename, *valstring;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
  FILE *scorefile;
  int i, j, k, strings, state, maxpoint;
  long point=0;
  double distance=stretchfac*fftsound->duration/(double)fftsound->horiz_num;
//  double dur=stretchfac/fftsound->binfreq;
  double onset=0.0, saveamp=0.0;
  double triglevel, gatetime, resontime, strfrq, range, maxamp, timeren=0.0;

  XmStringGetLtoR(calldata->value, charset, &filename);
  scorefile=fopen(filename,"w");
  if (scorefile==NULL) {
    XtManageChild(csoundWarning);
    XtFree(filename);
    return;
  }

  PlayStopHard();

  valstring=XmTextGetString(parmT1);
  triglevel=atof(valstring);
  valstring=XmTextGetString(parmT2);
  gatetime=atof(valstring);
  valstring=XmTextGetString(parmT3);
  strings=atoi(valstring);
  valstring=XmTextGetString(parmT4);
  resontime=atof(valstring);

  strfrq=30.;
  for (i=0; i<strings*8; i++) {
    state=0;
    range=(strfrq*pow(2., (double)strings/12.)-strfrq)/2.;
    for (j=0; j<fftsound->horiz_num; j++) {
      point=j*fftsound->numchannels;
      maxamp=-100.; maxpoint=-1;
      for (k=0; k<fftsound->numchannels; k++) {
        if (
	    (MEGFREQ_GET(point)>strfrq-range) && (MEGFREQ_GET(point)<strfrq+range)
	    && (MEGAMP_GET(point)*10000000.>triglevel)
	    )
	  {
          if (MEGAMP_GET(point)>maxamp) {
            maxamp=MEGAMP_GET(point); maxpoint=point;
          }
        }
        point++;
      }
      if (state==0) {
        if (maxpoint>-1) {
          saveamp=maxamp*10000000.;
          onset=j*distance;
          state=1;
        }
      } else {
	if (state==1) {
	  if (maxpoint==-1) {
	    fprintf(scorefile,"i1    %8.4f    %8.4f    %7.2f    %7.2f\n",
		    onset, (resontime>j*distance-onset?resontime:j*distance-onset),
		    saveamp, strfrq);
	    state=2; timeren=0.;
	  }
	} else {
	  timeren+=distance;
	  if (timeren>gatetime) state=0;
	}
      }
    }
    strfrq*=pow(2., 1./(double)strings);
  }

  fprintf(scorefile,"e\n");

  fclose(scorefile);
  XtFree(filename);
  XtUnmanageChild(toparmerudFileBox);
}

void Topeak(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(topeakFileBox);
}

void TopeakOk(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call; 
  char *filename;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
  FILE *scorefile;
  int i, j, keep;
  long point;
  double distance=stretchfac*fftsound->duration/(double)fftsound->horiz_num;
//  double dur=stretchfac/fftsound->binfreq;
  double onset;

  XmStringGetLtoR(calldata->value, charset, &filename);
  scorefile=fopen(filename,"w");
  if (scorefile==NULL) {
    XtManageChild(csoundWarning);
    XtFree(filename);
    return;
  }

  PlayStopHard();

  point=areat1*fftsound->numchannels;
  for (i=areat1; i<areat2; i++) {
     onset=i*distance;
     for (j=0; j<fftsound->numchannels; j++) {
       keep=0; if (j==0) keep=1; else {
       if ((MEGAMP_GET(point-1)<MEGAMP_GET(point)) && (MEGAMP_GET(point+1)<MEGAMP_GET(point)))
         keep=1; }
       if ((MEGAMP_GET(point)>0.000001) && (MEGFREQ_GET(point)>20.) && keep)
         fprintf(scorefile,"%8.4f    %7.2f    %7.2f\n",
           onset, MEGAMP_GET(point)*10000000., MEGFREQ_GET(point));
       point++;
     }
  }

  fclose(scorefile);
  XtFree(filename);
  XtUnmanageChild(topeakFileBox);
}

void Tostf(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(tostfFileBox);
}

void TostfOk(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call; 
  char *filename;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
  FILE *utfil;
  int i, j, hori;
  long point=0;

  XmStringGetLtoR(calldata->value, charset, &filename);
  utfil=fopen(filename,"w");
  if (utfil==NULL) {
    XtManageChild(stfWarning);
    XtFree(filename);
    return;
  }

  PlayStopHard();

  hori=fftsound->horiz_num/4-1;

  fprintf(utfil, "#Inventor V2.0 ascii\n");
  fprintf(utfil, "Separator {\n");
  fprintf(utfil, "  DirectionalLight { direction  -1 -1 -0.9  }\n");


  fprintf(utfil, "  Separator {\n");
  fprintf(utfil, "    Material {\n");
  fprintf(utfil, "        diffuseColor [ .6 .8 0.5 ]\n");
  fprintf(utfil, "        specularColor [ .1 .1 .1 ]\n");
  fprintf(utfil, "        shininess 0.99\n");
  fprintf(utfil, "        transparency 0.0\n");
  fprintf(utfil, "    }\n");
  fprintf(utfil, "    Coordinate3 {\n");
  fprintf(utfil, "      point [\n");

  for (i=0; i<hori; i++) {
    point=i*4*fftsound->numchannels;
    for (j=3; j<fftsound->numchannels/2; j++,point++) {
      fprintf(utfil, "%d %d %.2f, ", j*50, i*50,
        MEGAMP_GET(point)*10000000.);
    }
    fprintf(utfil, "\n");
  }

  fprintf(utfil, "      ]\n");
  fprintf(utfil, "    }\n");
  fprintf(utfil, "    ShapeHints {\n");
  fprintf(utfil, "      vertexOrdering COUNTERCLOCKWISE\n");
  fprintf(utfil, "    }\n");
  fprintf(utfil, "    QuadMesh {\n");
  fprintf(utfil, "      verticesPerColumn %d\n", hori);
  fprintf(utfil, "      verticesPerRow    %d\n", fftsound->numchannels/2-3);
  fprintf(utfil, "    }\n");
  fprintf(utfil, "  }\n");

  fprintf(utfil, "}\n");


  fclose(utfil);
  XtFree(filename);
  XtUnmanageChild(tostfFileBox);
}

void Toltas(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(toltasFileBox);
}

void ToltasOk(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call; 
  char *filename;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
  FILE *ltasfile;
  int j;
//  int i;
  long point;

  XmStringGetLtoR(calldata->value, charset, &filename);
  ltasfile=fopen(filename,"w");
  if (ltasfile==NULL) {
    XtManageChild(ltasWarning);
    XtFree(filename);
    return;
  }

  PlayStopHard();

 point=4*fftsound->numchannels;
 for (j=0; j<fftsound->numchannels; j++,point++)
   fprintf(ltasfile,"%f\n",MEGAMP_GET(point)*10000);

  fclose(ltasfile);
  XtFree(filename);
  XtUnmanageChild(toltasFileBox);
}


void create_export(struct FFTSound *fftsound){
  static Widget temp;

  XmString text;

  /* CREATE EXPORT MENU */
  exportButton=XtVaCreateManagedWidget(
    "exportButton",xmCascadeButtonWidgetClass,menuBar,NULL);
  text=XmStringCreateLocalized("Export");
  XtVaSetValues(exportButton,XmNlabelString,text,NULL);
  exportMenu=XmCreatePulldownMenu(menuBar,"exportMenu",NULL,0);
  XtVaSetValues(exportButton,XmNsubMenuId,exportMenu,NULL);

  /* CREATE PARMERUD BUTTON */
  toparmerud=XtVaCreateManagedWidget(
    "toparmerud",xmPushButtonWidgetClass,exportMenu,NULL);
  text=XmStringCreateLocalized("Parmerud Csound scorefile");
  XtVaSetValues(toparmerud,XmNlabelString,text,NULL);

  toparmerudFileBox=XmCreateFileSelectionDialog(mainWindow,
    "toparmerudFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(toparmerudFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Write Parmerud Csound score file");

  parmerudRowCol=XtVaCreateManagedWidget("parmerudRowCol",xmRowColumnWidgetClass,
    toparmerudFileBox,NULL);
  text=XmStringCreateLocalized("Trigging threshold");
  parmL1=XtVaCreateManagedWidget("parmL1",xmLabelWidgetClass,
    parmerudRowCol,XmNlabelString,text,NULL);
  parmT1=XtVaCreateManagedWidget("parmT1",
    xmTextFieldWidgetClass,parmerudRowCol,NULL);
  XtVaSetValues(parmT1,XmNwidth,100,XmNvalue,"10",NULL);
  text=XmStringCreateLocalized("Hysteresis time");
  parmL2=XtVaCreateManagedWidget("parmL2",xmLabelWidgetClass,
    parmerudRowCol,XmNlabelString,text,NULL);
  parmT2=XtVaCreateManagedWidget("parmT2",
    xmTextFieldWidgetClass,parmerudRowCol,NULL);
  XtVaSetValues(parmT2,XmNwidth,100,XmNvalue,"1",NULL);
  text=XmStringCreateLocalized("Strings pr. octave");
  parmL3=XtVaCreateManagedWidget("parmL3",xmLabelWidgetClass,
    parmerudRowCol,XmNlabelString,text,NULL);
  parmT3=XtVaCreateManagedWidget("parmT3",
    xmTextFieldWidgetClass,parmerudRowCol,NULL);
  XtVaSetValues(parmT3,XmNwidth,100,XmNvalue,"6",NULL);
  text=XmStringCreateLocalized("Resonance time");
  parmL4=XtVaCreateManagedWidget("parmL4",xmLabelWidgetClass,
				 parmerudRowCol,XmNlabelString,text,NULL);
  parmT4=XtVaCreateManagedWidget("parmT4",
    xmTextFieldWidgetClass,parmerudRowCol,NULL);
  XtVaSetValues(parmT4,XmNwidth,100,XmNvalue,"1",NULL);

  XtVaSetValues(toparmerudFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,700,
#endif
		NULL);

  XtAddCallback(toparmerudFileBox,XmNcancelCallback,Cancel,toparmerudFileBox);
  XtAddCallback(toparmerudFileBox,XmNokCallback,ToparmerudOk,fftsound);
  XtAddCallback(toparmerud,XmNactivateCallback,Toparmerud,fftsound);


  /* CREATE PEAK BUTTON */
  topeak=XtVaCreateManagedWidget(
    "topeak",xmPushButtonWidgetClass,exportMenu,NULL);
  text=XmStringCreateLocalized("Spectral peaks");
  XtVaSetValues(topeak,XmNlabelString,text,NULL);

  topeakFileBox=XmCreateFileSelectionDialog(mainWindow,
    "topeakFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(topeakFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Write spectral peaks");
  XtVaSetValues(topeakFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,500,
#endif
		NULL);

  XtAddCallback(topeakFileBox,XmNcancelCallback,Cancel,topeakFileBox);
  XtAddCallback(topeakFileBox,XmNokCallback,TopeakOk,fftsound);
  XtAddCallback(topeak,XmNactivateCallback,Topeak,fftsound);

  /* CREATE CSOUND BUTTON */
  tocsound=XtVaCreateManagedWidget(
    "tocsound",xmPushButtonWidgetClass,exportMenu,NULL);
  text=XmStringCreateLocalized("Csound scorefile");
  XtVaSetValues(tocsound,XmNlabelString,text,NULL);

  tocsoundFileBox=XmCreateFileSelectionDialog(mainWindow,
    "tocsoundFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(tocsoundFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Write Csound score file");
  XtVaSetValues(tocsoundFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,500,
#endif
		NULL);
  XtAddCallback(tocsoundFileBox,XmNcancelCallback,Cancel,tocsoundFileBox);
  XtAddCallback(tocsoundFileBox,XmNokCallback,TocsoundOk,fftsound);
  XtAddCallback(tocsound,XmNactivateCallback,Tocsound,fftsound);

  /* CREATE Matlab BUTTON */
  tomatlab=XtVaCreateManagedWidget(
    "tomatlab",xmPushButtonWidgetClass,exportMenu,NULL);
  text=XmStringCreateLocalized("Matlab file");
  XtVaSetValues(tomatlab,XmNlabelString,text,NULL);

  tomatlabFileBox=XmCreateFileSelectionDialog(mainWindow,
    "tomatlabFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(tomatlabFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Write Matlab file");
  XtVaSetValues(tomatlabFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,500,
#endif
		NULL);
  XtAddCallback(tomatlabFileBox,XmNcancelCallback,Cancel,tomatlabFileBox);
  XtAddCallback(tomatlabFileBox,XmNokCallback,TomatlabOk,fftsound);
  XtAddCallback(tomatlab,XmNactivateCallback,Tomatlab,fftsound);

  /* CREATE CSOUND OPENING WARNING */
  csoundWarning=XmCreateWarningDialog(mainWindow,"csoundWarning",NULL,0);
  temp=XmMessageBoxGetChild(csoundWarning,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  temp=XmMessageBoxGetChild(csoundWarning,XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Error!");
  XtVaSetValues(csoundWarning,XmNdialogTitle,text,
    XmNwidth,400,NULL);
  text=XmStringCreateSimple("Can't open file");
  XtVaSetValues(csoundWarning,XmNmessageString,text,NULL);

  /* CREATE STF BUTTON */
  tostf=XtVaCreateManagedWidget(
    "tostf",xmPushButtonWidgetClass,exportMenu,NULL);
  text=XmStringCreateLocalized("Inventor visualization file");
  XtVaSetValues(tostf,XmNlabelString,text,NULL);

  tostfFileBox=XmCreateFileSelectionDialog(mainWindow,
    "tostfFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(tostfFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Write Inventor visualization file");
  XtVaSetValues(tostfFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,500,
#endif
		NULL);
  XtAddCallback(tostfFileBox,XmNcancelCallback,Cancel,tostfFileBox);
  XtAddCallback(tostfFileBox,XmNokCallback,TostfOk,fftsound);
  XtAddCallback(tostf,XmNactivateCallback,Tostf,fftsound);

  /* CREATE STF OPENING WARNING */
  stfWarning=XmCreateWarningDialog(mainWindow,"stfWarning",NULL,0);
  temp=XmMessageBoxGetChild(stfWarning,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  temp=XmMessageBoxGetChild(stfWarning,XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Error!");
  XtVaSetValues(stfWarning,XmNdialogTitle,text,
    XmNwidth,400,NULL);
  text=XmStringCreateSimple("Can't open file");
  XtVaSetValues(stfWarning,XmNmessageString,text,NULL);

  /* CREATE TOLTAS BUTTON */
  toltas=XtVaCreateManagedWidget(
    "toltas",xmPushButtonWidgetClass,exportMenu,NULL);
  text=XmStringCreateLocalized("Single LTAS frame");
  XtVaSetValues(toltas,XmNlabelString,text,NULL);

  toltasFileBox=XmCreateFileSelectionDialog(mainWindow,
    "toltasFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(toltasFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Write single LTAS frame file");
  XtVaSetValues(toltasFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,500,
#endif
		NULL);
  XtAddCallback(toltasFileBox,XmNcancelCallback,Cancel,toltasFileBox);
  XtAddCallback(toltasFileBox,XmNokCallback,ToltasOk,fftsound);
  XtAddCallback(toltas,XmNactivateCallback,Toltas,fftsound);

  /* CREATE LTAS OPENING WARNING */
  ltasWarning=XmCreateWarningDialog(mainWindow,"ltasWarning",NULL,0);
  temp=XmMessageBoxGetChild(ltasWarning,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  temp=XmMessageBoxGetChild(ltasWarning,XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Error!");
  XtVaSetValues(ltasWarning,XmNdialogTitle,text,
    XmNwidth,400,NULL);
  text=XmStringCreateSimple("Can't open file");
  XtVaSetValues(ltasWarning,XmNmessageString,text,NULL);


}


