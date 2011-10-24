
#include "ceres.h"

static Widget grScaleForm,grScaleTextfield,grMoveForm,grMoveTextfield,
  grScale,grScaleOkButton,grScaleCancelButton,grScaleLabel,
  grMove,grMoveOkButton,grMoveCancelButton,grMoveLabel,grNonlin;

static Widget graphButton, graphMenu,  grClear, grInvert, grRetro;

void GrScale(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(grScaleForm);
}

void GrScaleOk(w, client, call)
Widget w;
XtPointer client, call;
{
  char *valstring;
  double val, y;
  int i;

  valstring=XmTextGetString(grScaleTextfield);
  val=atof(valstring);
  XtFree(valstring);

  for (i=1; i<=numsquare; i++) {
    y=val*(1.-(ypos[i]-10.)/theheight);
    if (y<0.) y=0.; if (y>1.) y=1.;
    ypos[i]=(1.-y)*theheight+10;
  }
}

void GrMove(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(grMoveForm); 
}

void GrMoveOk(w, client, call)
Widget w;
XtPointer client, call;
{
  char *valstring;
  double val, y;
  int i;

  valstring=XmTextGetString(grMoveTextfield);
  val=atof(valstring);
  XtFree(valstring);

  for (i=1; i<=numsquare; i++) {
    y=val+(1.-(ypos[i]-10.)/theheight);
    if (y<0.) y=0.; if (y>1.) y=1.;
    ypos[i]=(1.-y)*theheight+10;
  }
}

void GrNonlin(w, client, call)
Widget w;
XtPointer client, call;
{
}

void GrClear(w, client, call)
Widget w;
XtPointer client, call;
{
  ypos[1]=10+theheight/2;
  ypos[2]=ypos[1];
  xpos[2]=thewidth+50;
  numsquare=2;
  selected=0; 
}

void GrInvert(w, client, call)
Widget w;
XtPointer client, call;
{
  double y;
  int i;

  for (i=1; i<=numsquare; i++) {
    y=1.-(1.-(ypos[i]-10.)/theheight);
    if (y<0.) y=0.; if (y>1.) y=1.;
    ypos[i]=(1.-y)*theheight+10;
  }
}

void GrRetro(w, client, call)
Widget w;
XtPointer client, call;
{
}



void create_graph(struct FFTSound *fftsound){
  XmString text;

  /* CREATE GRAPH MENU */
  graphButton=XtVaCreateManagedWidget(
    "graphButton",xmCascadeButtonWidgetClass,menuBar,NULL);
  text=XmStringCreateLocalized("Graph");
  XtVaSetValues(graphButton,XmNlabelString,text,NULL);
  graphMenu=XmCreatePulldownMenu(menuBar,"graphMenu",NULL,0);
  XtVaSetValues(graphButton,XmNsubMenuId,graphMenu,NULL);

  /* CREATE SCALE BUTTON */
  grScale=XtVaCreateManagedWidget(
    "grScale",xmPushButtonWidgetClass,graphMenu,NULL);
  text=XmStringCreateLocalized("Scale");
  XtVaSetValues(grScale,XmNlabelString,text,NULL);

  grScaleForm=XmCreateBulletinBoardDialog(mainWindow,"grScaleForm",NULL,0);
  grScaleOkButton=XtVaCreateManagedWidget("grScaleOk",xmPushButtonWidgetClass,
    grScaleForm,NULL);
  text=XmStringCreateLocalized("Ok");
  XtVaSetValues(grScaleOkButton,XmNlabelString,text,XmNx,50,XmNy,100,
    XmNwidth,80,NULL);
  grScaleCancelButton=XtVaCreateManagedWidget("grScaleCancel",
    xmPushButtonWidgetClass,grScaleForm,NULL);
  text=XmStringCreateLocalized("Cancel");
  XtVaSetValues(grScaleCancelButton,XmNlabelString,text,XmNx,200,XmNy,100,NULL);

  grScaleTextfield=XtVaCreateManagedWidget("grScaleTextfield",
    xmTextFieldWidgetClass,grScaleForm,NULL);
  XtVaSetValues(grScaleTextfield,XmNwidth,100,XmNx,200,XmNy,10,
    XmNvalue,"0",NULL);
  grScaleLabel=XtVaCreateManagedWidget("grScaleLabel",xmLabelWidgetClass,
    grScaleForm,NULL);
  text=XmStringCreateLocalized("Multiplication factor:");
  XtVaSetValues(grScaleLabel,XmNlabelString,text,XmNx,0,XmNy,14,NULL);

  text=XmStringCreateLocalized("Graph scale");
  XtVaSetValues(grScaleForm,XmNdialogTitle,text,
    XmNcancelButton,grScaleCancelButton,
    XmNdefaultButton,grScaleOkButton,NULL);

  XtAddCallback(grScaleOkButton,XmNactivateCallback,GrScaleOk,NULL);
  XtAddCallback(grScale,XmNactivateCallback,GrScale,0);


  /* CREATE MOVE BUTTON */
  grMove=XtVaCreateManagedWidget(
    "grMove",xmPushButtonWidgetClass,graphMenu,NULL);
  text=XmStringCreateLocalized("Add");
  XtVaSetValues(grMove,XmNlabelString,text,NULL);

  grMoveForm=XmCreateBulletinBoardDialog(mainWindow,"grMoveForm",NULL,0);
  grMoveOkButton=XtVaCreateManagedWidget("grMoveOk",xmPushButtonWidgetClass,
    grMoveForm,NULL);
  text=XmStringCreateLocalized("Ok");
  XtVaSetValues(grMoveOkButton,XmNlabelString,text,XmNx,50,XmNy,100,
    XmNwidth,80,NULL);
  grMoveCancelButton=XtVaCreateManagedWidget("grMoveCancel",
    xmPushButtonWidgetClass,grMoveForm,NULL);
  text=XmStringCreateLocalized("Cancel");
  XtVaSetValues(grMoveCancelButton,XmNlabelString,text,XmNx,200,XmNy,100,NULL);

  grMoveTextfield=XtVaCreateManagedWidget("grMoveTextfield",
    xmTextFieldWidgetClass,grMoveForm,NULL);
  XtVaSetValues(grMoveTextfield,XmNwidth,100,XmNx,200,XmNy,10,
    XmNvalue,"0",NULL);
  grMoveLabel=XtVaCreateManagedWidget("grMoveLabel",xmLabelWidgetClass,
    grMoveForm,NULL);
  text=XmStringCreateLocalized("Addition value:");
  XtVaSetValues(grMoveLabel,XmNlabelString,text,XmNx,0,XmNy,14,NULL);

  text=XmStringCreateLocalized("Graph add");
  XtVaSetValues(grMoveForm,XmNdialogTitle,text,
    XmNcancelButton,grMoveCancelButton,
    XmNdefaultButton,grMoveOkButton,NULL);

  XtAddCallback(grMoveOkButton,XmNactivateCallback,GrMoveOk,NULL);
  XtAddCallback(grMove,XmNactivateCallback,GrMove,0);


  /* CREATE NONLIN BUTTON */
  grNonlin=XtVaCreateManagedWidget(
    "grNonlin",xmPushButtonWidgetClass,graphMenu,NULL);
  text=XmStringCreateLocalized("Nonlinear");
  XtVaSetValues(grNonlin,XmNlabelString,text,NULL);
  XtVaSetValues(grNonlin, XmNsensitive, False, NULL);
  XtAddCallback(grNonlin,XmNactivateCallback,GrNonlin,0);

  /* CREATE CLEAR BUTTON */
  grClear=XtVaCreateManagedWidget(
    "grClear",xmPushButtonWidgetClass,graphMenu,NULL);
  text=XmStringCreateLocalized("Clear");
  XtVaSetValues(grClear,XmNlabelString,text,NULL);
  XtAddCallback(grClear,XmNactivateCallback,GrClear,0);

  /* CREATE INVERT BUTTON */
  grInvert=XtVaCreateManagedWidget(
    "grInvert",xmPushButtonWidgetClass,graphMenu,NULL);
  text=XmStringCreateLocalized("Invert");
  XtVaSetValues(grInvert,XmNlabelString,text,NULL);
  XtAddCallback(grInvert,XmNactivateCallback,GrInvert,0);

  /* CREATE RETRO BUTTON */
  grRetro=XtVaCreateManagedWidget(
    "grRetro",xmPushButtonWidgetClass,graphMenu,NULL);
  text=XmStringCreateLocalized("Retrograde");
  XtVaSetValues(grRetro, XmNsensitive, False, NULL);
  XtVaSetValues(grRetro, XmNlabelString,text,NULL);
  XtAddCallback(grRetro, XmNactivateCallback,GrRetro,0);



  
}

