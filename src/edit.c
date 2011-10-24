
#include "ceres.h"
#include "config.h"
#include "phasevocoder.h"
#include "scales.h"
#include "undo.h"
#include "play.h"
#include "cutcopypaste.h"

static Widget cutButton;
static Widget copyButton;
static Widget pasteButton;

static Widget editButton;
static Widget editMenu;
static Widget settingsButton;
static Widget undoButton;
static Widget redoButton;
static Widget enableundoButton;

static Widget enablecont1;
static Widget enablecont2;
static Widget enablecont3;

static Widget showcont1;
static Widget showcont2;
static Widget showcont3;

static Widget showgridButton;
static Widget showpaintButton;

extern Boolean fileDirty;

extern int seltoggle;

extern int showfunc2_1;
extern int showfunc2_2;
extern int showfunc2_3;

extern int showpaint;
extern int showgrid;

extern int spotssize;

void setSettings(struct FFTSound *fftsound){

  /* Resynthesis */

  fftsound->I=(int)(fftsound->Dn * CONFIG_getDouble("time_stretch_factor"));

  fftsound->P= CONFIG_getDouble("pitch_shift");
  fftsound->Pinc = fftsound->P*8192./fftsound->R;
  if (fftsound->P>1.){
    fftsound->NP=fftsound->N2/fftsound->P;
  }else{
    fftsound->NP=fftsound->N2;
  }
  fftsound->synt=CONFIG_getDouble("threshold")/fftsound->N;

  /* Analysis */

  fftsound->overlap=CONFIG_getInt("overlap_factor");
  if (fftsound->overlap<1)
    fftsound->overlap=1;
  if (fftsound->overlap>32)
    fftsound->overlap=32;

  fftsound->Dn=fftsound->Nw/fftsound->overlap;

// SuperOops.
//  fftsound->I=fftsound->Dn;

  makewindows(fftsound->Wanal,  fftsound->Wsyn,  fftsound->Nw,  fftsound->N,  fftsound->I);

  numcoef=CONFIG_getInt("min_no_extract_nodes");
  if (numcoef<1) numcoef=1;
  if (numcoef>100) numcoef=100;
  if (numcoef!=fftsound->numcoef2)
    fftsound->numcoef2=0;


  /* Display */


  show_node_values=CONFIG_getBool("show_node_values");

  if (CONFIG_getBool("show_paint")==false){
    if (numspots) farge=-1;
    numspots=0;
  }

  dgain=CONFIG_getDouble("display_gain_factor");
  dscale=CONFIG_getDouble("frequency_zoom_factor");

  if (CONFIG_getBool("color_grey")==true) setgreycolors();
  if (CONFIG_getBool("color_hot")==true) sethotcolors();
  if (CONFIG_getBool("color_cool")==true) setcoldcolors();

  spotssize=CONFIG_getInt("paint_size");

  free(outportnames[0]);
  outportnames[0]=CONFIG_getChar("channel_1_port_name");
  free(outportnames[1]);
  outportnames[1]=CONFIG_getChar("channel_2_port_name");
  free(outportnames[2]);
  outportnames[2]=CONFIG_getChar("channel_3_port_name");
  free(outportnames[3]);
  outportnames[3]=CONFIG_getChar("channel_4_port_name");

  free(outportnames[4]);
  outportnames[4]=CONFIG_getChar("channel_5_port_name");
  free(outportnames[5]);
  outportnames[5]=CONFIG_getChar("channel_6_port_name");
  free(outportnames[6]);
  outportnames[6]=CONFIG_getChar("channel_7_port_name");
  free(outportnames[7]);
  outportnames[7]=CONFIG_getChar("channel_8_port_name");


  /* Pitch Grid */

  if (CONFIG_getBool("grid_pentatonic")) makepenta();
  else if (CONFIG_getBool("grid_major")) makemajor();
  else if (CONFIG_getBool("grid_harmonic_minor")) makeminor();
  else if (CONFIG_getBool("grid_chromatic")) makechroma();
  else if (CONFIG_getBool("grid_whole-tone")) makewhole();
  else if (CONFIG_getBool("grid_harmonic_series")) makeharmseries();
  else if (CONFIG_getBool("grid_even-harmonic_series")) makeevenharmseries();
  else if (CONFIG_getBool("grid_odd-harmonic_series")) makeoddharmseries();

  if(CONFIG_getBool("grid_load_pitch_grid_file")==true){
    float f;
    FILE *infile;

    char *filename=CONFIG_getChar("frequency_file");
    infile=fopen(filename, "r");
    free(filename);

    if (infile==NULL) {
      ResetCursor(topLevel);
      XtManageChild(fileWarning);
      XtFree(filename);
      return;
    }

    numfreqs=0;
    while ((!feof(infile)) && (numfreqs<500)) {
      fscanf(infile,"%f",&f); 
      gridfreqs[numfreqs]=f; numfreqs++;
    }
    numfreqs--;
    fclose(infile);

    fileDirty = FALSE;
    
  }
}


void EDIT_setUndoRedoMenues(void){
  static XmString undotext=NULL,redotext=NULL;
  if(undotext==NULL){
    undotext=XmStringCreateLocalized("Undo");
    redotext=XmStringCreateLocalized("Redo");
  }
  if(UNDO_allowedUndo()==true){
    XtVaSetValues(undoButton,XmNlabelString,undotext,XmNsensitive,True,NULL);
  }else{
    XtVaSetValues(undoButton,XmNlabelString,undotext,XmNsensitive,False,NULL);
  }
  if(UNDO_allowedRedo()==true){
    XtVaSetValues(redoButton,XmNlabelString,redotext,XmNsensitive,True,NULL);
  }else{
    XtVaSetValues(redoButton,XmNlabelString,redotext,XmNsensitive,False,NULL);
  }
}

static void EDIT_undo(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  UNDO_do(fftsound);
}


static void EDIT_redo(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  UNDO_redo(fftsound);
}

static void EDIT_enableundo(w, client, call)
Widget w;
XtPointer client, call;
{
  XmString text;
  int localdoundo=UNDO_getDoUndo();
  if(localdoundo==2){
    if(CONFIG_getBool("disable_undo")==true){
      text=XmStringCreateLocalized("Enable Undo      *");
      UNDO_setDoUndo(1);
    }else{
      text=XmStringCreateLocalized("Enable Undo");
      UNDO_setDoUndo(0);
    }
  }else{
    if(localdoundo==1){
      text=XmStringCreateLocalized("Enable Undo");
      UNDO_setDoUndo(0);
    }else{
      text=XmStringCreateLocalized("Enable Undo      *");
      UNDO_setDoUndo(1);
    }
  }
}

static void EDIT_cut(Widget w, XtPointer client, XtPointer call){
  struct FFTSound *fftsound=(struct FFTSound *)client;

  UNDO_addTransform(fftsound);
  CCP_cut(fftsound,areat1,areat2);
  RedrawWin(fftsound);
}

static void EDIT_copy(Widget w, XtPointer client, XtPointer call){
  struct FFTSound *fftsound=(struct FFTSound *)client;
  CCP_copy(fftsound,areat1,areat2);
}
static void EDIT_paste(Widget w, XtPointer client, XtPointer call){
  struct FFTSound *fftsound=(struct FFTSound *)client;
  UNDO_addTransform(fftsound);
  CCP_paste(fftsound,areat1,areat2);
  RedrawWin(fftsound);
}

static void EDIT_enablecont1(w, client, call)
Widget w;
XtPointer client, call;
{
  seltoggle=1;
}
static void EDIT_enablecont2(w, client, call)
Widget w;
XtPointer client, call;
{
  seltoggle=2;
}
static void EDIT_enablecont3(w, client, call)
Widget w;
XtPointer client, call;
{
  seltoggle=3;
}

void EDIT_showcont1(Widget w, XtPointer client, XtPointer call)
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmString text=0;

  switch(showfunc2_1){
  case 0:
    showfunc2_1=1;
    text=XmStringCreateLocalized("Hide Control Function 1");
    seltoggle=1;
    break;
  case 2:
    if(CONFIG_getBool("show_control_function_1")){
      showfunc2_1=1;
    }else{
      showfunc2_1=0;
    }
    EDIT_showcont1(w,client,call);
    return;
  case 1:
    showfunc2_1=0;
    text=XmStringCreateLocalized("Show Control Function 1");
    break;
  }
  XtVaSetValues(showcont1,XmNlabelString,text,NULL);
  
  RestoreWin(fftsound);
}
void EDIT_showcont2(Widget w, XtPointer client, XtPointer call)
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmString text=0;

  switch(showfunc2_2){
  case 0:
    showfunc2_2=1;
    text=XmStringCreateLocalized("Hide Control Function 2");
    seltoggle=2;
    break;
  case 2:
    if(CONFIG_getBool("show_control_function_2")){
      showfunc2_2=1;
    }else{
      showfunc2_2=0;
    }
    EDIT_showcont2(w,client,call);
    return;
  case 1:
    showfunc2_2=0;
    text=XmStringCreateLocalized("Show Control Function 2");
    break;
  }
  XtVaSetValues(showcont2,XmNlabelString,text,NULL);
  
  RestoreWin(fftsound);
}
void EDIT_showcont3(Widget w, XtPointer client, XtPointer call)
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmString text=0;

  switch(showfunc2_3){
  case 0:
    showfunc2_3=1;
    text=XmStringCreateLocalized("Hide Control Function 3");
    seltoggle=3;
    break;
  case 2:
    if(CONFIG_getBool("show_control_function_3")){
      showfunc2_3=1;
    }else{
      showfunc2_3=0;
    }
    EDIT_showcont3(w,client,call);
    return;
  case 1:
    showfunc2_3=0;
    text=XmStringCreateLocalized("Show Control Function 3");
    break;
  }
  XtVaSetValues(showcont3,XmNlabelString,text,NULL);
  
  RestoreWin(fftsound);
}

void EDIT_showgrid(Widget w, XtPointer client, XtPointer call)
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmString text=0;

  switch(showgrid){
  case 0:
    showgrid=1;
    text=XmStringCreateLocalized("Hide Grid");
    break;
  case 2:
    if(CONFIG_getBool("show_grid")){
      showgrid=1;
    }else{
      showgrid=0;
    }
    EDIT_showgrid(w,client,call);
    return;
  case 1:
    showgrid=0;
    text=XmStringCreateLocalized("Show Grid");
    break;
  }
  XtVaSetValues(showgridButton,XmNlabelString,text,NULL);
  
  RestoreWin(fftsound);
}

void EDIT_showpaint(Widget w, XtPointer client, XtPointer call)
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmString text=0;

  switch(showpaint){
  case 0:
    showpaint=1;
    text=XmStringCreateLocalized("Hide Paint");
    RestoreWin(fftsound);
    break;
  case 2:
    if(CONFIG_getBool("show_paint")){
      showpaint=1;
    }else{
      showpaint=0;
    }
    EDIT_showpaint(w,client,call);
    return;
  case 1:
    showpaint=0;
    text=XmStringCreateLocalized("Show Paint");
    if (numspots) farge=-1;
    numspots=0;
    RedrawWin(fftsound);
    break;
  }
  XtVaSetValues(showpaintButton,XmNlabelString,text,NULL);

}


static void EDIT_settings(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;

  CONFIG_show();

  PlayStopHard();

  setSettings(fftsound);

  RedrawWin(fftsound);
}


void create_edit(struct FFTSound *fftsound)
{

  XmString text;

  /* CREATE EDIT MENU */
  editButton=XtVaCreateManagedWidget(
    "editButton",xmCascadeButtonWidgetClass,menuBar,NULL);
  text=XmStringCreateLocalized("Edit");
  XtVaSetValues(editButton,XmNlabelString,text,NULL);
  editMenu=XmCreatePulldownMenu(menuBar,"editMenu",NULL,0);
  XtVaSetValues(editButton,XmNsubMenuId,editMenu,NULL);

  /* CREATE UNDO BUTTON */
  undoButton=XtVaCreateManagedWidget(
    "--<Undo>--",
    xmPushButtonWidgetClass,editMenu,
    XmNmnemonic,XStringToKeysym("Z"),XmNaccelerator,"Alt<Key>Z",
    XmNacceleratorText,XmStringCreateLocalized("Alt+Z"),
    NULL
    );
  text=XmStringCreateLocalized("Undo");
  XtVaSetValues(undoButton,XmNlabelString,text,XmNshadowType,True,XmNsensitive,False,NULL);
  XtAddCallback(undoButton,XmNactivateCallback,EDIT_undo,fftsound);

  /* CREATE REDO BUTTON */
  redoButton=XtVaCreateManagedWidget(
    "Redo",
    xmPushButtonWidgetClass,editMenu,
    XmNmnemonic,XStringToKeysym("X"),XmNaccelerator,"Alt<Key>X",
    XmNacceleratorText,XmStringCreateLocalized("Alt+X"),
    NULL
    );
  text=XmStringCreateLocalized("Redo");
  XtVaSetValues(redoButton,XmNlabelString,text,XmNsensitive,False,NULL);
  XtAddCallback(redoButton,XmNactivateCallback,EDIT_redo,fftsound);

  /* CREATE ENABLE UNDO BUTTON */
  enableundoButton=XtVaCreateManagedWidget(
    "Enable Undo",
    xmToggleButtonWidgetClass,editMenu,
    NULL
    );
  text=XmStringCreateLocalized("Enable Undo");
  XtVaSetValues(enableundoButton,XmNlabelString,text,NULL);
  XmToggleButtonSetState(enableundoButton,CONFIG_getBool("disable_undo")==true?False:True,False);
  //  XtAddCallback(cslider->togglewidget,XmNvalueChangedCallback,GUI_GeneralToggle,cslider);
           XtAddCallback(enableundoButton,XmNvalueChangedCallback,EDIT_enableundo,fftsound);
  //  XtAddCallback(enableundoButton,XmNactivateCallback,EDIT_enableundo,fftsound);

  XtVaCreateManagedWidget(
			  "sep8", xmSeparatorWidgetClass,
			  editMenu, XmNorientation, XmHORIZONTAL,
			  NULL
			  );

  /* CREATE CUT/COPY/PASTE */


  cutButton=XtVaCreateManagedWidget(
    "cutButton",xmPushButtonWidgetClass,editMenu,
    XmNmnemonic,XStringToKeysym("X"),XmNaccelerator,"Ctrl<Key>X",
    XmNacceleratorText,XmStringCreateLocalized("Ctrl+X"),
    NULL);
  text=XmStringCreateLocalized("Cut");
  XtVaSetValues(cutButton,XmNlabelString,text,NULL);
  XtAddCallback(cutButton,XmNactivateCallback,EDIT_cut,fftsound);

  copyButton=XtVaCreateManagedWidget(
    "copyButton",xmPushButtonWidgetClass,editMenu,
    XmNmnemonic,XStringToKeysym("C"),XmNaccelerator,"Ctrl<Key>C",
    XmNacceleratorText,XmStringCreateLocalized("Ctrl+C"),
    NULL);
  text=XmStringCreateLocalized("Copy");
  XtVaSetValues(copyButton,XmNlabelString,text,NULL);
  XtAddCallback(copyButton,XmNactivateCallback,EDIT_copy,fftsound);

  pasteButton=XtVaCreateManagedWidget(
    "pasteButton",xmPushButtonWidgetClass,editMenu,
    XmNmnemonic,XStringToKeysym("X"),XmNaccelerator,"Ctrl<Key>V",
    XmNacceleratorText,XmStringCreateLocalized("Ctrl+V"),
    NULL);
  text=XmStringCreateLocalized("Paste");
  XtVaSetValues(pasteButton,XmNlabelString,text,NULL);
  XtAddCallback(pasteButton,XmNactivateCallback,EDIT_paste,fftsound);


  XtVaCreateManagedWidget(
			  "sep9", xmSeparatorWidgetClass,
			  editMenu, XmNorientation, XmHORIZONTAL,
			  NULL
			  );

  /* CREATE SELECT CONTROL FUNCTION BUTTONS */
  enablecont1=XtVaCreateManagedWidget(
    "qewfqewf",
    xmPushButtonWidgetClass,editMenu,
    XmNacceleratorText,XmStringCreateLocalized("F1"),
    NULL
    );
  text=XmStringCreateLocalized("Select Control Function 1");

 XtVaSetValues(enablecont1,XmNlabelString,text,NULL);
  XtAddCallback(enablecont1,XmNactivateCallback,EDIT_enablecont1,fftsound);

  enablecont2=XtVaCreateManagedWidget(
    "qewfqewf",
    xmPushButtonWidgetClass,editMenu,
    XmNacceleratorText,XmStringCreateLocalized("F2"),
    NULL
    );
  text=XmStringCreateLocalized("Select Control Function 2");

  XtVaSetValues(enablecont2,XmNlabelString,text,NULL);
  XtAddCallback(enablecont2,XmNactivateCallback,EDIT_enablecont2,fftsound);
  enablecont3=XtVaCreateManagedWidget(
    "qewfqewf",
    xmPushButtonWidgetClass,editMenu,
    XmNacceleratorText,XmStringCreateLocalized("F3"),
    NULL
    );
  text=XmStringCreateLocalized("Select Control Function 3");

  XtVaSetValues(enablecont3,XmNlabelString,text,NULL);
  XtAddCallback(enablecont3,XmNactivateCallback,EDIT_enablecont3,fftsound);


  XtVaCreateManagedWidget(
			  "sep", xmSeparatorWidgetClass,
			  editMenu, XmNorientation, XmHORIZONTAL,
			  NULL
			  );
  /* CREATE SHOW/HIDE CONTROL FUNCTION BUTTONS */
  showcont1=XtVaCreateManagedWidget(
    "qewfqewf",
    xmPushButtonWidgetClass,editMenu,
    XmNacceleratorText,XmStringCreateLocalized("F5"),
    NULL
    );
  text=XmStringCreateLocalized(CONFIG_getBool("show_control_function_1")==false?"Show Control Function 1":"Hide Control Fucntion 1");

  XtVaSetValues(showcont1,XmNlabelString,text,NULL);
  XtAddCallback(showcont1,XmNactivateCallback,EDIT_showcont1,fftsound);

  showcont2=XtVaCreateManagedWidget(
    "qewfqewf",
    xmPushButtonWidgetClass,editMenu,
    XmNacceleratorText,XmStringCreateLocalized("F6"),
    NULL
    );
  text=XmStringCreateLocalized(CONFIG_getBool("show_control_function_2")==false?"Show Control Function 2":"Hide Control Fucntion 2");

  XtVaSetValues(showcont2,XmNlabelString,text,NULL);
  XtAddCallback(showcont2,XmNactivateCallback,EDIT_showcont2,fftsound);

  showcont3=XtVaCreateManagedWidget(
    "qewfqewf",
    xmPushButtonWidgetClass,editMenu,
    XmNacceleratorText,XmStringCreateLocalized("F7"),
    NULL
    );
  text=XmStringCreateLocalized(CONFIG_getBool("show_control_function_3")==false?"Show Control Function 3":"Hide Control Fucntion 3");

  XtVaSetValues(showcont3,XmNlabelString,text,NULL);
  XtAddCallback(showcont3,XmNactivateCallback,EDIT_showcont3,fftsound);

  XtVaCreateManagedWidget(
			  "sep", xmSeparatorWidgetClass,
			  editMenu, XmNorientation, XmHORIZONTAL,
			  NULL
			  );
  /* CREATE SHOW/HIDE GRID/PAINT BUTTONS */

  showgridButton=XtVaCreateManagedWidget(
    "qewfqewf",
    xmPushButtonWidgetClass,editMenu,
    XmNacceleratorText,XmStringCreateLocalized("F11"),
    NULL
    );
  text=XmStringCreateLocalized(CONFIG_getBool("show_grid")==false?"Show Grid":"Hide Grid");

  XtVaSetValues(showgridButton,XmNlabelString,text,NULL);
  XtAddCallback(showgridButton,XmNactivateCallback,EDIT_showgrid,fftsound);

  showpaintButton=XtVaCreateManagedWidget(
    "qewfqewf",
    xmPushButtonWidgetClass,editMenu,
    XmNacceleratorText,XmStringCreateLocalized("F12"),
    NULL
    );
  text=XmStringCreateLocalized(CONFIG_getBool("show_paint")==false?"Show Paint":"Hide Paint");

  XtVaSetValues(showpaintButton,XmNlabelString,text,NULL);
  XtAddCallback(showpaintButton,XmNactivateCallback,EDIT_showpaint,fftsound);

  XtVaCreateManagedWidget(
			  "sep", xmSeparatorWidgetClass,
			  editMenu, XmNorientation, XmHORIZONTAL,
			  NULL
			  );

  /* CREATE SETTINGS BUTTON */
  settingsButton=XtVaCreateManagedWidget(
    "settingsButton",xmPushButtonWidgetClass,editMenu,
    XmNmnemonic,XStringToKeysym("S"),XmNaccelerator,"Alt<Key>S",
    XmNacceleratorText,XmStringCreateLocalized("Alt+S"),
    NULL);
  text=XmStringCreateLocalized("Preferences");
  XtVaSetValues(settingsButton,XmNlabelString,text,NULL);

  XtAddCallback(settingsButton,XmNactivateCallback,EDIT_settings,fftsound);
}

