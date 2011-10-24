
#include "Python.h"

#include "ceres.h"

static Widget  help, helpButton, helpMenu, helpBox, htmlButton;

void ShowHelp(w, client, call)
Widget w;
XtPointer client, call;
{
  Widget dialog=(Widget) client;
  XtManageChild(dialog);
}

void ShowHtml(w, client, call)
Widget w;
XtPointer client, call;
{
  PyRun_SimpleString("showhelp()");
}



void create_help(struct FFTSound *fftsound){
  XmString text;
  static Widget temp;
  char tempstring[1024];

  /* CREATE HELP THINGS */
  helpButton=XtVaCreateManagedWidget(
    "helpButton",xmCascadeButtonWidgetClass,menuBar,NULL);
  text=XmStringCreateLocalized("Help");
  XtVaSetValues(helpButton,XmNlabelString,text,NULL);
  XtVaSetValues(menuBar,XmNmenuHelpWidget,helpButton,NULL);

  helpMenu=XmCreatePulldownMenu(menuBar,"helpMenu",NULL,0);
  help=XtVaCreateManagedWidget(
    "helpMenu",xmPushButtonWidgetClass,helpMenu,NULL);
  text=XmStringCreateLocalized("About");
  XtVaSetValues(help,XmNlabelString,text,NULL);  
  XtVaSetValues(helpButton,XmNsubMenuId,helpMenu,NULL);

  helpBox=XmCreateMessageDialog(help,"helpBox",NULL,0);
  text=XmStringCreateSimple("Help");
  XtVaSetValues(helpBox,XmNdialogTitle,text,NULL);
  sprintf(tempstring,


"CERES is a graphical sound editor working\n\
in the frequency domain.\n\n\
The phase vocoder core is based on code from\n\
F.Richard Moore: Elements of Computer Music\n\n\
Version %0.2f, Oyvind Hammer(DSP/GUI) / Kjetil Matheussen(GUI) %d\n\
Please send your comments to k.s.matheussen@notam02.no.\n\
\n\
Some code written by Johnathan F. Lee (Ceres2)\n\
and Stanko Juzbasic (Ceres3).\
",VERSION,YEAR);

	  text=XmStringCreateLtoR(tempstring,

     XmFONTLIST_DEFAULT_TAG);
  XtVaSetValues(helpBox,XmNmessageString,text,XmNwidth,600,
    XmNx,100,XmNy,200,NULL);

  temp=XmMessageBoxGetChild(helpBox,XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);
  temp=XmMessageBoxGetChild(helpBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  XtAddCallback(help,XmNactivateCallback,ShowHelp,helpBox);

  progressDialog=XmCreateWorkingDialog(mainWindow,"progressDialog",NULL,0);
  temp=XmMessageBoxGetChild(progressDialog,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  temp=XmMessageBoxGetChild(progressDialog,XmDIALOG_OK_BUTTON);
  XtUnmanageChild(temp);
  temp=XmMessageBoxGetChild(progressDialog,XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp); 
  progressScale=XtVaCreateManagedWidget("progressScale",
    xmScaleWidgetClass, progressDialog, NULL);
  text=XmStringCreateLocalized("Working");
  XtVaSetValues(progressDialog,XmNdialogTitle,text,NULL,NULL);
  text=XmStringCreateLocalized("Please wait ...");
  XtVaSetValues(progressDialog,XmNmessageString,text,
    XmNorientation, XmHORIZONTAL, NULL,NULL);

  htmlButton=XtVaCreateManagedWidget(
    "helpMenu",xmPushButtonWidgetClass,helpMenu,NULL);
  text=XmStringCreateLocalized("Help");
  XtVaSetValues(htmlButton,XmNlabelString,text,NULL);  
  XtAddCallback(htmlButton,XmNactivateCallback,ShowHtml,fftsound);
}




