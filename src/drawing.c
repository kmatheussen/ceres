
#include "ceres.h"
#include "play.h"
#include "undo.h"

static struct FFTSound *fftsound_here;

int seltoggle=1;

static int squaresize=8;
static int selectedsize=12;

int spotssize=3;

static int fontwidth=6;
static int fontheight=6;
static int nodevaluelength=5;

static const bool doublebuffer=true;

//static XGCValues values;
int xpos2[MAXNOD+1], ypos2[MAXNOD+1], numsquare2=2, selected2=0, funcpt2;
int xpos3[MAXNOD+1], ypos3[MAXNOD+1], numsquare3=2, selected3=0, funcpt3;


static float funcminvalues[3]={0.0f,0.0f,0.0f};
static float funcmaxvalues[3]={1.0f,1.0f,1.0f};

/*
  The following 8 ints can have these values:
  0 - Not drawn.
  1 - Drawn
  2 - Not initialized; not used when considering if a function is drawn or not.
  */

static int showfunc1=2;
static int showfunc2=2;
static int showfunc3=2;

int showfunc2_1=2;
int showfunc2_2=2;
int showfunc2_3=2;

int showpaint=2;
int showgrid=2;


/* FuncGCs for control func 2 and 3 where copied from ceres3. */

void setgreycolors(void)
{
   if (farge==1) return;
   XSetForeground(theDisplay, markerGC, gray0.pixel);
   XSetForeground(theDisplay, theGC0, gray0.pixel);
   XSetForeground(theDisplay, showGC0, gray0.pixel);
   XSetForeground(theDisplay, theGC25, gray25.pixel);
   XSetForeground(theDisplay, showGC25, gray25.pixel);
   XSetForeground(theDisplay, theGC50, gray50.pixel);
   XSetForeground(theDisplay, showGC50, gray50.pixel);
   XSetForeground(theDisplay, theGC75, gray75.pixel);
   XSetForeground(theDisplay, showGC75, gray75.pixel);
   XSetForeground(theDisplay, theGC100, gray100.pixel);
   XSetForeground(theDisplay, showGC100, gray100.pixel);
   XtVaSetValues(sketchpad,XmNbackground, gray100.pixel,NULL);
   XSetForeground(theDisplay, funcGC, red.pixel);
   XSetForeground(theDisplay, func2GC, green.pixel);
   XSetForeground(theDisplay, func3GC, blue.pixel);
   XSetForeground(theDisplay, spotsGC, blue.pixel);
   farge=1;
}

void sethotcolors(void)
{
  if (farge==2) return;
   XSetForeground(theDisplay, markerGC, gray100.pixel);
   XSetForeground(theDisplay, theGC0, gray100.pixel);
   XSetForeground(theDisplay, showGC0, gray100.pixel);
   XSetForeground(theDisplay, theGC25, yellow.pixel);
   XSetForeground(theDisplay, showGC25, yellow.pixel);
   XSetForeground(theDisplay, theGC50, orange.pixel);
   XSetForeground(theDisplay, showGC50, orange.pixel);
   XSetForeground(theDisplay, theGC75, red.pixel);
   XSetForeground(theDisplay, showGC75, red.pixel);
   XSetForeground(theDisplay, theGC100, gray0.pixel);
   XSetForeground(theDisplay, showGC100, gray0.pixel);
   XtVaSetValues(sketchpad,XmNbackground, gray0.pixel,NULL);
   XSetForeground(theDisplay, funcGC, green.pixel);
   XSetForeground(theDisplay, func2GC, blue.pixel);
   XSetForeground(theDisplay, func3GC, gray100.pixel);/*____lightblue_WAS_TOO_WHITE_____*/
   XSetForeground(theDisplay, spotsGC, green.pixel);
   farge=2;
}

void setcoldcolors(void)
{
   if (farge==3) return;
   XSetForeground(theDisplay, markerGC, gray100.pixel);
   XSetForeground(theDisplay, theGC0, gray100.pixel);
   XSetForeground(theDisplay, showGC0, gray100.pixel);
   XSetForeground(theDisplay, theGC25, lightblue.pixel);
   XSetForeground(theDisplay, showGC25, lightblue.pixel);
   XSetForeground(theDisplay, theGC50, blue.pixel);
   XSetForeground(theDisplay, showGC50, blue.pixel);
   XSetForeground(theDisplay, theGC75, darkblue.pixel);
   XSetForeground(theDisplay, showGC75, darkblue.pixel);
   XSetForeground(theDisplay, theGC100, gray0.pixel);
   XSetForeground(theDisplay, showGC100, gray0.pixel);
   XtVaSetValues(sketchpad,XmNbackground, gray0.pixel,NULL);
   XSetForeground(theDisplay, funcGC, green.pixel);
   XSetForeground(theDisplay, func2GC, orange.pixel);
   XSetForeground(theDisplay, func3GC, yellow.pixel);
   XSetForeground(theDisplay, spotsGC, green.pixel);
   farge=3; 
}

void SetWatchCursor(Widget w)
{
        static Cursor watch = (int) NULL;

        if(!watch)
                watch = XCreateFontCursor(XtDisplay(w),XC_watch);

        XDefineCursor(XtDisplay(w),XtWindow(w),watch);
        XmUpdateDisplay(w);
}

void ResetCursor(Widget w)
{
  XUndefineCursor(XtDisplay(w),XtWindow(w));
  XmUpdateDisplay(w);
}


void drawscale(struct FFTSound *fftsound)
{
  int i, grafx, step=1, ch;
  char tall[1024]; // Should be enough.

  theGC=theGC0; showGC=showGC0;
  XDrawLine(theDisplay,XtWindow(sketchpad),showGC,
    40,theheight+20,thewidth+50,theheight+20);
  XDrawLine(theDisplay,bitmap,theGC,40,theheight+20,thewidth+50,theheight+20);
  XDrawLine(theDisplay,XtWindow(sketchpad),showGC,40,theheight+20,40,10);
  XDrawLine(theDisplay,bitmap,theGC,40,theheight+20,40,10);

  //if (fftsound->duration>25.) step=2;
  //if (fftsound->duration>50.) step=4;
  step=max(1,fftsound->duration*2/25);

  for (i=1; i<=(int)(fftsound->duration); i+=step) {
    grafx=(int)(i*(double)thewidth/fftsound->duration)+50;
    XDrawLine(theDisplay,XtWindow(sketchpad),showGC,
      grafx,theheight+15,grafx,theheight+25);
    XDrawLine(theDisplay,bitmap,theGC,grafx,theheight+15,grafx,theheight+25);
    sprintf(tall, "%d", i);
    XDrawString(theDisplay,XtWindow(sketchpad),showGC,
      grafx-5,theheight+40,tall,strlen(tall));
    XDrawString(theDisplay,bitmap,theGC,
      grafx-5,theheight+40,tall,strlen(tall));
  }
  for (ch=0; ch<fftsound->samps_per_frame; ch++)
    for (i=1; i<=(int)(fftsound->R/2000)/dscale; i+=fftsound->samps_per_frame) {
      grafx=theheight-(int)((dscale*i*1000./fftsound->binfreq)/fftsound->samps_per_frame)+10
        -ch*theheight/fftsound->samps_per_frame;
      XDrawLine(theDisplay,XtWindow(sketchpad),showGC,35,grafx,45,grafx);
      XDrawLine(theDisplay,bitmap,theGC,35,grafx,45,grafx);
      sprintf(tall, "%2d", i);
      XDrawString(theDisplay,XtWindow(sketchpad),showGC,
        2,grafx+5,tall,strlen(tall));
      XDrawString(theDisplay,bitmap,theGC,
        2,grafx+5,tall,strlen(tall));
    }

}

double getfuncval(struct FFTSound *fftsound,int hnum)
{
  double hnumx;

  hnumx=hnum*(double)thewidth/fftsound->horiz_num+50.;
  while ((int)floor(hnumx)>xpos[funcpt+1]) funcpt++;
  return theheight-(ypos[funcpt+1]-ypos[funcpt])*(hnumx-xpos[funcpt])
    /(xpos[funcpt+1]-xpos[funcpt])-ypos[funcpt]+10;
}

double getfuncval2(struct FFTSound *fftsound,int hnum)
{
  double hnumx;

  hnumx=hnum*(double)thewidth/fftsound->horiz_num+50.;
  while ((int)floor(hnumx)>xpos2[funcpt2+1]) funcpt2++;
  return theheight-(ypos2[funcpt2+1]-ypos2[funcpt2])*(hnumx-xpos2[funcpt2])
    /(xpos[funcpt2+1]-xpos2[funcpt2])-ypos2[funcpt2]+10;
}


/* getfuncval3 is taken from ceres3. */

double getfuncval3 (struct FFTSound *fftsound,int vnum3)
{
  double _x, _x1, _x0, _y, _y1, _y0;
  _y = ((double) (fftsound->numchannels - vnum3) * theheight) / fftsound->numchannels + 10.;
  if (_y < (double) ypos3[funcpt3 - 1])
    if (funcpt3 > 1)
      funcpt3--;
  _y1 = (double) ypos3[funcpt3 - 1];
  _y0 = (double) ypos3[funcpt3];
  _x1 = (double) xpos3[funcpt3 - 1];
  _x0 = (double) xpos3[funcpt3];

  _x = _x0 + (_y - _y0) * (_x1 - _x0) / (_y1 - _y0) - 50.;

  return _x;
}


/* CALLBACK ROUTINES */

void RedrawWin(struct FFTSound *fftsound)
{
  int i, j, grafx, grafold, ch, y;
  long point=0;
  XPoint list0[LSIZE],list1[LSIZE],list2[LSIZE],list3[LSIZE];
  int c0=0,c1=0,c2=0,c3=0;
  double a;
  int prevx=0,prevy=0,prevcol=0;
 
  XFillRectangle(theDisplay, bitmap, theGC100, 0, 0, thewidth+70, theheight+70);

  XClearArea(theDisplay, XtWindow(sketchpad), 0, 0, 0, 0, True);

  SetWatchCursor(topLevel);


  if (fftsound->horiz_num>0) drawscale(fftsound);

  if(fftsound->usemem==false) pthread_mutex_lock(&fftsound->mutex);

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    grafold=-1; point=ch*fftsound->numchannels*fftsound->horiz_num;

    for (j=0; j<fftsound->horiz_num; j++) {
      grafx = (int)(j*(double)thewidth/fftsound->horiz_num)+50;

      if (grafx!=grafold) {
	// amptemp and freqtemp is used to avoid switching between two files too much.
	// (Probably no noticable difference.)
	float amptemp[fftsound->numchannels];
	float freqtemp[fftsound->numchannels];
	int temppoint=point;

	for(i=0;i<fftsound->numchannels;i++){
	  amptemp[i]=MEGAMP_GET(point)*dgain;
	  point++;
	}

	point=temppoint;

	for(i=0;i<fftsound->numchannels;i++){
	  if(amptemp[i]>0.0000012){
	    freqtemp[i]=MEGFREQ_GET(point);
	  }
	  point++;
	}

        for (i=0; i<fftsound->numchannels; i++) {

          a=amptemp[i];
          if (a>0.0000012) {

            y=(int)(dscale*freqtemp[i]/fftsound->binfreq);

            if (y<theheight) {
	      int col;
	      y=((ch+1)*theheight-y)/fftsound->samps_per_frame+10;
	      if (a>0.0000100) {
		col=1;
	      } else if (a>0.0000050){ 
		col=2;
	      } else if (a>0.0000025) {
		col=3;
	      } else {
		col=4;
	      }

	      if(prevy==y && prevx==grafx && col==prevcol){
		continue;
	      }
	      prevy=y;prevx=grafx;
	      prevcol=col;

	      switch(col){
	      case 1:
		ADD_POINT(grafx,y,list0,c0,showGC0,theGC0);
		break;
	      case 2:
		ADD_POINT(grafx,y,list1,c1,showGC25,theGC25);
		break;
	      case 3:
		ADD_POINT(grafx,y,list2,c2,showGC50,theGC50);
		break;
	      case 4:
		ADD_POINT(grafx,y,list3,c3,showGC75,theGC75);
		break;
	      }
	    }
	  }

        }

        grafold=grafx;
      } else point+=fftsound->numchannels;
    }
  }
  if(fftsound->usemem==false) pthread_mutex_unlock(&fftsound->mutex);


  FLUSH_LIST(list0,c0,showGC0,theGC0);
  FLUSH_LIST(list1,c1,showGC25,theGC25);
   FLUSH_LIST(list2,c2,showGC50,theGC50);
   FLUSH_LIST(list3,c3,showGC75,theGC75);

  for (ch=0; ch<fftsound->samps_per_frame; ch++) for (i=0; i<numspots; i++) {
    y=dscale*spotsf[i];
    if (y<theheight)
      XFillRectangle(
		     theDisplay,bitmap,spotsGC,
		     spotst[i]*(double)thewidth/fftsound->horiz_num+50-spotssize,
		     theheight-(ch*theheight+y)/fftsound->samps_per_frame+10-spotssize,
		     spotssize*2,
		     spotssize*2
		     );
  }

  ResetCursor(topLevel);

  RestoreWin(fftsound);
}


static void DrawNodeValue(int x,int y,float value,int presicion){
  char number[100];

  if(show_node_values==false) return;

  if(presicion==nodevaluelength){
    sprintf(number, "%.5f",value);
  }else{
    sprintf(number, "%.2f",value);
  }

  x=min(thewidth+50+selectedsize-2-(strlen(number)*fontwidth),x+squaresize);
  y=max(0,y-(squaresize*2)-1);
  
  XFillRectangle(
		 theDisplay,
		 doublebuffer?spotsmap:XtWindow(sketchpad),
		 theGC100,
		 x, y,
		 fontwidth*(strlen(number))+1 , fontheight+3
		 );

  XDrawString(
	      theDisplay,
	      doublebuffer?spotsmap:XtWindow(sketchpad),
	      theGC0,
	      x+2 , y+squaresize+1,
	      number , strlen(number)
	      );
      

}

void RedrawFunc(void)
{

  int i;
  if(sketchpad == NULL)return;

  if(
     showfunc1==1 || (showfunc1==2 && (showfunc2_1==1 || (showfunc2_1==2 && CONFIG_getBool("show_control_function_1"))))
  ){
    for (i=1; i<=numsquare; i++) {
      XDrawRectangle(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),funcGC,
		     xpos[i]-squaresize,ypos[i]-squaresize,squaresize*2,squaresize*2);
    }
    for (i=1; i<numsquare; i++) {
      XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),funcGC,
		xpos[i],ypos[i],xpos[i+1],ypos[i+1]);
      XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),funcGC,
		xpos[i]-1,ypos[i],xpos[i+1]-1,ypos[i+1]);
      XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),funcGC,
		xpos[i]+1,ypos[i],xpos[i+1]+1,ypos[i+1]);
      XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),funcGC,
		xpos[i],ypos[i]-1,xpos[i+1],ypos[i+1]-1);
      XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),funcGC,
		xpos[i],ypos[i]+1,xpos[i+1],ypos[i+1]+1);
    }
    if (selected>0) {
      XFillRectangle(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),funcGC,
		     xpos[selected]-(selectedsize*1),ypos[selected]-selectedsize,selectedsize*2,selectedsize*2);
    }
    for (i=1; i<=numsquare; i++) {
      float value=(1.0f-((ypos[i]-10)/(float)theheight))*(funcmaxvalues[0]-funcminvalues[0])+funcminvalues[0];
      DrawNodeValue(xpos[i],ypos[i],value,i==selected?nodevaluelength:2);
    }
  }
  
  if(
     showfunc2==1 || (showfunc2==2 && (showfunc2_2==1 || (showfunc2_2==2 && CONFIG_getBool("show_control_function_2"))))
     ){
    for (i=1; i<=numsquare2; i++) {
      XDrawRectangle(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),func2GC,
		     xpos2[i]-squaresize,ypos2[i]-squaresize,squaresize*2,squaresize*2);
    }
    for (i=1; i<numsquare2; i++) {
      XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),func2GC,
		xpos2[i],ypos2[i],xpos2[i+1],ypos2[i+1]);
      XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),func2GC,
		xpos2[i]-1,ypos2[i],xpos2[i+1]-1,ypos2[i+1]);
      XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),func2GC,
		xpos2[i]+1,ypos2[i],xpos2[i+1]+1,ypos2[i+1]);
      XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),func2GC,
		xpos2[i],ypos2[i]-1,xpos2[i+1],ypos2[i+1]-1);
      XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),func2GC,
		xpos2[i],ypos2[i]+1,xpos2[i+1],ypos2[i+1]+1);
    }
    if (selected2>0) {
      XFillRectangle(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),func2GC,
		     xpos2[selected2]-selectedsize,ypos2[selected2]-selectedsize,selectedsize*2,selectedsize*2);
    }
    for (i=1; i<=numsquare2; i++) {
      float value=(1.0f-((ypos2[i]-10)/(float)theheight))*(funcmaxvalues[1]-funcminvalues[1])+funcminvalues[1];
      DrawNodeValue(xpos2[i],ypos2[i],value,i==selected2?nodevaluelength:2);
    }
  }

  if (
      showfunc3==1 || (showfunc3==2 && (showfunc2_3==1 || (showfunc2_3==2 && CONFIG_getBool("show_control_function_3"))))
      )
    {
      for (i = 1; i <= numsquare3; i++)
	{
	  XDrawRectangle (theDisplay,
			  doublebuffer ? spotsmap : XtWindow (sketchpad),
			  func3GC, xpos3[i] - squaresize,
			  ypos3[i] - squaresize, squaresize * 2,
			  squaresize * 2);
	}
      for (i = 1; i < numsquare3; i++)
	{
	  XDrawLine (theDisplay,
		     doublebuffer ? spotsmap : XtWindow (sketchpad), func3GC,
		     xpos3[i], ypos3[i], xpos3[i + 1], ypos3[i + 1]);
	  XDrawLine (theDisplay,
		     doublebuffer ? spotsmap : XtWindow (sketchpad), func3GC,
		     xpos3[i] - 1, ypos3[i], xpos3[i + 1] - 1, ypos3[i + 1]);
	  XDrawLine (theDisplay,
		     doublebuffer ? spotsmap : XtWindow (sketchpad), func3GC,
		     xpos3[i] + 1, ypos3[i], xpos3[i + 1] + 1, ypos3[i + 1]);
	  XDrawLine (theDisplay,
		     doublebuffer ? spotsmap : XtWindow (sketchpad), func3GC,
		     xpos3[i] , ypos3[i] +1, xpos3[i + 1], ypos3[i + 1]+1);
	  XDrawLine (theDisplay,
		     doublebuffer ? spotsmap : XtWindow (sketchpad), func3GC,
		     xpos3[i] , ypos3[i] -1, xpos3[i + 1], ypos3[i + 1]-1);
	}
      if (selected3 > 0)
	{
	  XFillRectangle (theDisplay,
			  doublebuffer ? spotsmap : XtWindow (sketchpad),
			  func3GC, xpos3[selected3] - selectedsize,
			  ypos3[selected3] - selectedsize, selectedsize * 2,
			  selectedsize * 2);
	}
      for (i=1; i<=numsquare3; i++) {
	float value=((xpos3[i]-50)/(float)thewidth)*(funcmaxvalues[2]-funcminvalues[2])+funcminvalues[2];
	DrawNodeValue(xpos3[i],ypos3[i],value,i==selected3?nodevaluelength:2);
      }
    }

/*
  int i;

  if (XmToggleButtonGetState(displayToggleFunc)==False) return;
  for (i=1; i<=numsquare; i++) {
    XDrawRectangle(theDisplay,XtWindow(sketchpad),funcGC,
      xpos[i]-4,ypos[i]-4,8,8);
  }
  for (i=1; i<numsquare; i++) {
    XDrawLine(theDisplay,XtWindow(sketchpad),funcGC,
      xpos[i],ypos[i],xpos[i+1],ypos[i+1]);
  }
  if (selected>0) {
    XFillRectangle(theDisplay,XtWindow(sketchpad),funcGC,
      xpos[selected]-4,ypos[selected]-4,8,8);
      }
  */
}

void SetVisibleFunc(int funcnum,int f){
switch(funcnum){
  case 1:
    showfunc1=f;
    break;
  case 2:
    showfunc2=f;
    break;
  case 3:
    showfunc3=f;
    break;
  }
}

void SetFuncMinMax(int funcnum,float minvalue,float maxvalue){
  funcminvalues[funcnum]=minvalue;
  funcmaxvalues[funcnum]=maxvalue;
}


void RedrawArea(struct FFTSound *fftsound)
{
  int i, t1, t2, f1, f2;

  if ((areat1==0) && (areat2==fftsound->horiz_num)
    && (areaf1==0) && (areaf2==fftsound->numchannels)) return;
  if (areat1<areat2) { t1=areat1; t2=areat2; } else { t1=areat2; t2=areat1; }
  if (areaf1<areaf2) { f1=areaf1; f2=areaf2; } else { f1=areaf2; f2=areaf1; }
  f1*=dscale; f2*=dscale;
  if (f1>theheight) f1=theheight;
  if (f2>theheight) f2=theheight;
  for (i=0; i<fftsound->samps_per_frame; i++)
    XDrawRectangle(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),funcGC,
      t1*(double)thewidth/fftsound->horiz_num+50,
      theheight-i*theheight/fftsound->samps_per_frame-f2/fftsound->samps_per_frame+10,
      (t2-t1)*(double)thewidth/fftsound->horiz_num, (f2-f1)/fftsound->samps_per_frame);
}

void RedrawScale(struct FFTSound *fftsound)
{
  int i, f, ch;

  if ( showgrid==1 || (showgrid==2 && CONFIG_getBool("show_grid")==true)){
    for (ch=0; ch<fftsound->samps_per_frame; ch++)
      for (i=0; i<numfreqs; i++) if ((gridfreqs[i]>0.) && (gridfreqs[i]<fftsound->R/2.))  {
	f=(int)(dscale*gridfreqs[i]/fftsound->binfreq);
	if (f>theheight) break;
	f=theheight-(ch*theheight+f)/fftsound->samps_per_frame+10;
	XDrawLine(theDisplay,doublebuffer?spotsmap:XtWindow(sketchpad),gridGC,50,f,thewidth+50,f);
      }
  }
}



void RestoreWin_area(
		     struct FFTSound *fftsound,
		     int x1,int y1,
		     int x2,int y2
){

  XCopyArea(theDisplay, bitmap, doublebuffer?spotsmap:XtWindow(sketchpad),
	    DefaultGCOfScreen(XtScreen(sketchpad)), x1, y1, x2-x1,y2-y1, x1, y1);
  /*
  XCopyArea(theDisplay, bitmap, XtWindow(sketchpad),
	    DefaultGCOfScreen(XtScreen(sketchpad)), 0, 0, 800+70, theheight+70, 0, 0);
  */

  RedrawFunc();
  RedrawArea(fftsound);
  RedrawScale(fftsound);

  if(doublebuffer==true){
    XCopyArea(
	      theDisplay,
	      spotsmap, XtWindow(sketchpad),
	      DefaultGCOfScreen(XtScreen(sketchpad)),
	      x1, y1, x2-x1, y2-y1, x1, y1
	      );
  }

}

void RestoreWin(struct FFTSound *fftsound){
  RestoreWin_area(fftsound,0,0,thewidth+70,theheight+70);
}

void RestoreWinMotif(Widget w,XtPointer client,XtPointer call){
  struct FFTSound *fftsound=(struct FFTSound*)client;
  RestoreWin(fftsound);
}

static void Keydown(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  KeySym keysym;
  char key_string[1];
  int key_string_len, i;

  int *xposs=NULL;
  int *yposs=NULL;
  int *sel=NULL;
  int *num=NULL;

  switch(seltoggle){
  case 1:
    xposs=xpos;
    yposs=ypos;
    sel=&selected;
    num=&numsquare;
    break;
  case 2:
    xposs=xpos2;
    yposs=ypos2;
    sel=&selected2;
    num=&numsquare2;
    break;
  case 3:
    xposs=xpos3;
    yposs=ypos3;
    sel=&selected3;
    num=&numsquare3;
    break;
  }

  key_string_len=XLookupString(
			       (XKeyEvent *) event, key_string,
			       sizeof key_string, &keysym,
			       (XComposeStatus *) NULL
			       );

  if(
     (*sel>1) 
     && (*sel<*num)
     && ((keysym==XK_BackSpace) || (keysym==XK_Delete))
     )
    {

      for (i=*sel; i<*num; i++) {
        xposs[i]=xposs[i+1]; yposs[i]=yposs[i+1];
      }
      (*num)--;
      if(keysym==XK_BackSpace){
	*sel=*sel==2?1:*sel-1;
      }else{
	*sel=*sel==*num+1?1:*sel;
      }
      RestoreWin(fftsound_here);
    }


  if(keysym=='z'){
    UNDO_do(fftsound_here);
  }
  if(keysym=='x'){
    UNDO_redo(fftsound_here);
  }


  if(keysym==XK_Left){
    *sel=*sel==1?*num:*sel-1;
    RestoreWin(fftsound_here);
  }

  if(keysym==XK_Right){
    *sel=*sel==*num?1:*sel+1;
    RestoreWin(fftsound_here);
  }


  /* The following keys are here, because I dont know how to set it in the menues with motif, please fix if you do. -Kjetil. */

  if(keysym==XK_F1){
    seltoggle=1;
  }
  if(keysym==XK_F2){
    seltoggle=2;
  }
  if(keysym==XK_F3){
    seltoggle=3;
  }

  if(keysym==XK_F5){
    EDIT_showcont1(NULL,fftsound_here,NULL);
  }
  if(keysym==XK_F6){
    EDIT_showcont2(NULL,fftsound_here,NULL);
  }
  if(keysym==XK_F7){
    EDIT_showcont3(NULL,fftsound_here,NULL);
  }

  if(keysym==XK_F11){
    EDIT_showgrid(NULL,fftsound_here,NULL);
  }
  if(keysym==XK_F12){
    EDIT_showpaint(NULL,fftsound_here,NULL);
  }

  /*
  if(keysym==XK_p){
    Play(fftsound_here,areat1,areat2);
  }
  */

  if (keysym==XK_space){
    PlayStop();
  }

}

static void GraphClick(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  struct FFTSound *fftsound=fftsound_here;
  int x, y, addpos;
  int graphnum;
  int *sel=NULL;
  int lokke;
  int *numsq=NULL;
  int *xp=NULL,*yp=NULL;
  int size;

  XButtonEvent *motion=(XButtonEvent *)event;
  x=motion->x; y=motion->y;

  /*
  if (seltoggle==1) FuncGraphClick(x, y);
  else if (seltoggle==2) FuncGraphClick2(x, y);
  else if (seltoggle==3) FuncGraphClick3(x, y);
  return;
  */

  for(graphnum=1;graphnum<=3;graphnum++){
    switch(graphnum){
    case 1:
      numsq=&numsquare;
      xp=xpos;
      yp=ypos;
      sel=&selected;
      break;
    case 2:
      numsq=&numsquare2;
      xp=xpos2;
      yp=ypos2;
      sel=&selected2;
      break;
    case 3:
      numsq=&numsquare3;
      xp=xpos3;
      yp=ypos3;
      sel=&selected3;
      break;
    }

    for(lokke=1;lokke<=*numsq;lokke++){
      if(lokke==*sel){
	size=selectedsize;
      }else{
	size=squaresize;
      }
      if(
	 x>=xp[lokke]-size
	 && x<=xp[lokke]+size
	 && y>=yp[lokke]-size
	 && y<=yp[lokke]+size
	 ){
	seltoggle=graphnum;
	*sel=lokke;
	RestoreWin(fftsound);
	return;
      }
    }
  }

  switch(seltoggle){
  case 1:
    numsq=&numsquare;
    xp=xpos;
    yp=ypos;
    sel=&selected;
    break;
  case 2:
    numsq=&numsquare2;
    xp=xpos2;
    yp=ypos2;
    sel=&selected2;
    break;
  case 3:
    numsq=&numsquare3;
    xp=xpos3;
    yp=ypos3;
    sel=&selected3;
    break;
  }

  if(x<=50 || x>=thewidth+50 || y<=10 || y>=theheight+10) return;
  if(*numsq>=50) return;

  addpos=2;
  if(seltoggle==3){
    while(y>yp[addpos]) addpos++;
  }else{
    while(x>xp[addpos]) addpos++;
  }
  (*numsq)++;

  for(lokke=*numsq; lokke>addpos; lokke--) {
    xp[lokke]=xp[lokke-1];
    yp[lokke]=yp[lokke-1];
  }

  xp[addpos]=x;
  yp[addpos]=y;

  *sel=addpos;

  RestoreWin(fftsound);

#if 0
  int x, y, i, which=0, addpos=0;

  XButtonEvent *motion=(XButtonEvent *)event;
  x=motion->x; y=motion->y;

  if (y>theheight+10) y=theheight+10;
  if (y<10) y=10;
  for (i=1; i<=numsquare; i++)
    if ((x>xpos[i]-4) && (x<xpos[i]+4)) {
      which=i;
      break;
    }
  if (which>0) selected=which;
  else if ((x>50) && (x<thewidth+50) && (numsquare<MAXNOD)) { 
    for (i=1; i<=numsquare; i++)
      if (x<xpos[i]) {
        addpos=i; break;
      }
    numsquare++;
    for (i=numsquare; i>addpos; i--) {
      xpos[i]=xpos[i-1]; ypos[i]=ypos[i-1];
    }
    xpos[addpos]=x; ypos[addpos]=y;
    selected=addpos;
  }
  RestoreWin(fftsound_here);
#endif
}

static void GraphDrag(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  struct FFTSound *fftsound=fftsound_here;
  int x, y;
  int *xp,*yp;
  int sel;
  int numsq;

  int xold;
  int yold;
  int temp;

  int minx,maxx;
  int miny,maxy;

  XButtonEvent *motion=(XButtonEvent *)event;
  x=motion->x; y=motion->y;

  y=min(y,theheight+10);
  y=max(y,10);

  x=min(x,thewidth+50);
  x=max(x,50);

  switch(seltoggle){
  case 1:
    xp=xpos;
    yp=ypos;
    sel=selected;
    numsq=numsquare;
    break;
  case 2:
    xp=xpos2;
    yp=ypos2;
    sel=selected2;
    numsq=numsquare2;
    break;
  case 3:
    xp=ypos3;
    yp=xpos3;
    temp=y;
    y=x;
    x=temp;
    sel=selected3;
    numsq=numsquare3;
    break;
  default:
    fprintf(stderr,"Illegal argument to FuncGraphDrag_redraw()\n");
    return;
  }

  if(sel<=0) return;


  //  if(seltoggle!=3){
    if (sel==1){
      x=xp[1];
    }else{
      x=max(x,xp[sel-1]);
    }

    if(sel==numsq){
      x=xp[numsq];
    }else{
      x=min(x,xp[sel+1]);
    }
    //  }

  xold=xp[sel];
  yold=yp[sel];

  yp[sel]=y;
  xp[sel]=x;


  minx=min(xold,x);
  miny=min(y,yold);
  maxx=max(xold,x);
  maxy=max(y,yold);

  if(sel>1){
    minx=min(minx,xp[sel-1]);
    miny=min(miny,yp[sel-1]);
    maxy=max(maxy,yp[sel-1]);
  }

  if(sel<numsq){
    maxx=max(maxx,xp[sel+1]);
    miny=min(miny,yp[sel+1]);
    maxy=max(maxy,yp[sel+1]);
  }


  if(seltoggle==3){
    RestoreWin_area(
		    fftsound,
		    miny-selectedsize,minx-selectedsize-fontheight,
		    maxy+selectedsize+((nodevaluelength+3)*fontwidth),maxx+selectedsize
		    );
  }else{
    RestoreWin_area(
		    fftsound,
		    minx-selectedsize,miny-selectedsize-fontheight,
		    maxx+selectedsize+((nodevaluelength+3)*fontwidth),maxy+selectedsize
		    );
  }

#if 0
  int x, y;

  XButtonEvent *motion=(XButtonEvent *)event;
  x=motion->x; y=motion->y;

  if (y>theheight+10) y=theheight+10;
  if (y<10) y=10;
  if (selected>0) {
    if (selected==1) ypos[1]=y;
    else if (selected==numsquare) ypos[numsquare]=y;
    else if ((x>xpos[selected-1]) && (x<xpos[selected+1])) {
      xpos[selected]=x; ypos[selected]=y;
    }
    RestoreWin(fftsound_here);
  }
#endif
}

static void AreaClick(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  int x, y;
  struct FFTSound *fftsound=fftsound_here;
  XButtonEvent *motion=(XButtonEvent *)event;
  x=motion->x; y=motion->y;

  if (y>theheight+20) y=theheight+20;
  if (y<0) y=0;
  areat1=((x-50)*fftsound->horiz_num)/thewidth;
  if (areat1<0) areat1=0; if (areat1>fftsound->horiz_num) areat1=fftsound->horiz_num;
  areat2=areat1;
  areaf1=theheight-y+10;
  areach=areaf1*fftsound->samps_per_frame/fftsound->numchannels;
  areaf1=(areaf1%(fftsound->numchannels/fftsound->samps_per_frame))*fftsound->samps_per_frame;
  areaf1/=dscale;
  if (areaf1<0) areaf1=0; if (areaf1>fftsound->numchannels) areaf1=fftsound->numchannels;
  areaf2=areaf1;

  RestoreWin(fftsound_here);
}

static void AreaDrop(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  int swap;
  struct FFTSound *fftsound=fftsound_here;

  if ((areat1==areat2) && (areaf1==areaf2)) {
    areat1=0; areat2=fftsound->horiz_num;
    areaf1=0; areaf2=fftsound->numchannels;
    return;
  }
  if (areat1>areat2) { swap=areat1; areat1=areat2; areat2=swap; }
  if (areaf1>areaf2) { swap=areaf1; areaf1=areaf2; areaf2=swap; }
}


static void AreaDrag(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  int x, y, ch;
//  int swap;
  XButtonEvent *motion=(XButtonEvent *)event;
  struct FFTSound *fftsound=fftsound_here;

  x=motion->x; y=motion->y;
  if (y>theheight+20) y=theheight+20;
  if (y<0) y=0;
  if (x<40) x=40;
  if ( ! (showpaint==1 || (showpaint==2 && CONFIG_getBool("show_paint")==true))){
    areat2=((x-50)*fftsound->horiz_num)/(double)thewidth;
    if (areat2<0) areat2=0; if (areat2>fftsound->horiz_num) areat2=fftsound->horiz_num;
    areaf2=theheight-y+10;
    if (areaf2<areach*fftsound->numchannels/fftsound->samps_per_frame)
      areaf2=areach*fftsound->numchannels/fftsound->samps_per_frame;
    if (areaf2>=(areach+1)*fftsound->numchannels/fftsound->samps_per_frame) areaf2=fftsound->numchannels;
      else areaf2=(areaf2%(fftsound->numchannels/fftsound->samps_per_frame))*fftsound->samps_per_frame;
    areaf2/=dscale;
    RestoreWin(fftsound);
  } else if (numspots<MAX_PAINT) {
    spotst[numspots]=((x-50)*fftsound->horiz_num)/(double)thewidth;
    if (spotst[numspots]<0) spotst[numspots]=0;
    if (spotst[numspots]>fftsound->horiz_num-1) spotst[numspots]=fftsound->horiz_num-1;
    spotsf[numspots]=theheight-y+10;
    spotsf[numspots]=(spotsf[numspots]%(fftsound->numchannels/fftsound->samps_per_frame))*fftsound->samps_per_frame;
    spotsf[numspots]/=dscale;
    for (ch=0; ch<fftsound->samps_per_frame; ch++) {
      XFillRectangle(theDisplay,bitmap,spotsGC,
        spotst[numspots]*(double)thewidth/fftsound->horiz_num+50-spotssize,
        theheight-ch*theheight/fftsound->samps_per_frame
          -dscale*spotsf[numspots]/fftsound->samps_per_frame+10-spotssize,
        spotssize*2,spotssize*2);
      XFillRectangle(XtDisplay(sketchpad),XtWindow(sketchpad),spotsGC,
        spotst[numspots]*(double)thewidth/fftsound->horiz_num+50-spotssize,
        theheight-ch*theheight/fftsound->samps_per_frame
          -dscale*spotsf[numspots]/fftsound->samps_per_frame+10-spotssize,
        spotssize*2,spotssize*2);
    }
    numspots++;
  }
}

void create_drawing(struct FFTSound *fftsound){
  static XtActionsRec actions[]={
    {"GraphClick",  GraphClick},
    {"GraphDrag",   GraphDrag},
    {"AreaClick",   AreaClick},
    {"AreaDrag",    AreaDrag},
    {"AreaDrop",    AreaDrop},
    {"Keydown",     Keydown},
  };
  static char drawTranslations[]=
    "<Btn2Down>:   GraphClick()\n\
<Btn2Motion>: GraphDrag()\n\
<Btn3Down>:   AreaClick()\n\
<Btn3Up>:     AreaDrop()\n\
<Btn3Motion>: AreaDrag()\n\
<Key>:        Keydown()";

  static char *myargv[1]={"./ceres"};
  static int myargc=1;



  fftsound_here=fftsound;

  thewidth=CONFIG_getInt("window_width");


  /* INITIALIZE X */
/*  XtSetLanguageProc(NULL, (XtLanguageProc)NULL, NULL); */
  topLevel=XtVaAppInitialize(
    &app_context,"Ceres",NULL,0,&myargc,myargv,NULL,NULL);
  dw = XmGetXmDisplay(XtDisplay(topLevel));
  XtVaSetValues(dw, XmNdragInitiatorProtocolStyle, XmDRAG_NONE, NULL);


  /* CREATE MAIN WINDOW & IMMEDIATE CHILDREN */
  mainWindow=XtVaCreateManagedWidget(
    "mainWindow",xmMainWindowWidgetClass,topLevel,
    XmNscrollingPolicy, XmAUTOMATIC, NULL);

  menuBar=XmCreateMenuBar(mainWindow,"menuBar",NULL,0);
  XtManageChild(menuBar);

  frame=XtVaCreateManagedWidget(
    "frame",xmFrameWidgetClass,mainWindow,NULL);

  XmMainWindowSetAreas(mainWindow, menuBar, NULL, NULL, NULL, frame);
  XtVaSetValues(mainWindow,XmNwidth,thewidth+78,XmNheight,theheight+109, NULL);


  /* CREATE GRAPHICS */
   sketchpad = XtVaCreateManagedWidget(
     "sketchpad",xmDrawingAreaWidgetClass, frame,
      XmNwidth,thewidth+70,XmNheight,theheight+70,NULL);

   theDisplay = XtDisplay(sketchpad);

   //   fprintf(stderr,"theDisplay: %x\n",theDisplay);

   theWindow = XtWindow(sketchpad);
   screen = DefaultScreen(theDisplay);
   theRoot = RootWindowOfScreen(XtScreen(sketchpad));
   theCmap = DefaultColormap( theDisplay, DefaultScreen(theDisplay) );
   bitmap = XCreatePixmap(XtDisplay(sketchpad),theRoot,thewidth+70,theheight+70,
     DefaultDepthOfScreen(XtScreen(sketchpad)));
   spotsmap = XCreatePixmap(XtDisplay(sketchpad),theRoot,thewidth+70,theheight+70,
     DefaultDepthOfScreen(XtScreen(sketchpad)));
   theGC = XCreateGC( theDisplay, bitmap, 0, 0 );
   showGC = XCreateGC( theDisplay, theRoot, 0, 0 );
   theGC0 = XCreateGC( theDisplay, bitmap, 0, 0 );
   showGC0 = XCreateGC( theDisplay, theRoot, 0, 0 );
   theGC25 = XCreateGC( theDisplay, bitmap, 0, 0 );
   showGC25 = XCreateGC( theDisplay, theRoot, 0, 0 );
   theGC50 = XCreateGC( theDisplay, bitmap, 0, 0 );
   showGC50 = XCreateGC( theDisplay, theRoot, 0, 0 );
   theGC75 = XCreateGC( theDisplay, bitmap, 0, 0 );
   showGC75 = XCreateGC( theDisplay, theRoot, 0, 0 );
   theGC100 = XCreateGC( theDisplay, bitmap, 0, 0 );
   showGC100 = XCreateGC( theDisplay, theRoot, 0, 0 );
   funcGC = XCreateGC( theDisplay, theRoot, 0, 0);
   func2GC = XCreateGC( theDisplay, theRoot, 0, 0);
   func3GC = XCreateGC( theDisplay, theRoot, 0, 0);
   gridGC = XCreateGC( theDisplay, theRoot, 0, 0);
   spotsGC = XCreateGC( theDisplay, theRoot, 0, 0);
   markerGC = XCreateGC( theDisplay, theRoot, 0, 0 );

   XParseColor(theDisplay,theCmap,"gray0",&gray0);
   XAllocColor( theDisplay, theCmap,&gray0);
   XParseColor(theDisplay,theCmap,"gray25",&gray25);
   XAllocColor( theDisplay, theCmap,&gray25);
   XParseColor(theDisplay,theCmap,"gray50",&gray50);
   XAllocColor( theDisplay, theCmap,&gray50);
   XParseColor(theDisplay,theCmap,"gray75",&gray75);
   XAllocColor( theDisplay, theCmap,&gray75);
   XParseColor(theDisplay,theCmap,"gray100",&gray100);
   XAllocColor( theDisplay, theCmap,&gray100);
   XParseColor(theDisplay,theCmap,"light blue",&lightblue);
   XAllocColor( theDisplay, theCmap,&lightblue);
   XParseColor(theDisplay,theCmap,"blue",&blue);
   XAllocColor( theDisplay, theCmap,&blue);
   XParseColor(theDisplay,theCmap,"navy",&darkblue);
   XAllocColor( theDisplay, theCmap,&darkblue);
   XParseColor(theDisplay,theCmap,"yellow",&yellow);
   XAllocColor( theDisplay, theCmap,&yellow);
   XParseColor(theDisplay,theCmap,"red",&red);
   XAllocColor( theDisplay, theCmap,&red);
   XParseColor(theDisplay,theCmap,"orange",&orange);
   XAllocColor( theDisplay, theCmap,&orange);
   XParseColor(theDisplay,theCmap,"green",&green);
   XAllocColor( theDisplay, theCmap,&green);
   XParseColor(theDisplay,theCmap,"coral",&brown);
   XAllocColor( theDisplay, theCmap,&brown);

   XSetForeground(theDisplay, gridGC, brown.pixel);
  
  
  XtAppAddActions(app_context, actions, XtNumber(actions));
  XtOverrideTranslations(sketchpad,XtParseTranslationTable(drawTranslations));
  XtAddCallback(sketchpad,XmNexposeCallback,RestoreWinMotif,fftsound);
  
  xpos[1]=50; ypos[1]=100; xpos[2]=thewidth+50; ypos[2]=100;
  //  xpos2[1]=50; ypos2[1]=200; xpos2[2]=850; ypos2[2]=200;

  xpos2[1] = 50;
  ypos2[1] = theheight + 6;
  xpos2[2] = thewidth + 50;
  ypos2[2] = theheight + 6;
  xpos3[1] = 50;
  ypos3[1] = 10;
  xpos3[2] = 50;
  ypos3[2] = theheight + 10;

}

