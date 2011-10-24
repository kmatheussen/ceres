
#include "ceres.h"





Widget sketchpad, topLevel, dw;

Widget fileWarning;

XtAppContext app_context;
Widget mainWindow, menuBar, frame;

Widget transMenu;
Widget progressDialog, progressScale;

Window theRoot, theWindow;
Colormap theCmap;
Display *theDisplay;
int screen;
Pixmap bitmap, spotsmap;
XColor gray0, gray25, gray50, gray75, gray100;
XColor red, orange, yellow, green, brown;
XColor darkblue, blue, lightblue;

GC theGC, showGC;
GC theGC0, theGC25, theGC50, theGC75, theGC100;
GC showGC0, showGC25, showGC50, showGC75, showGC100;
GC funcGC, func2GC,func3GC,gridGC, spotsGC, markerGC;
int farge;

double gridfreqs[600];
int numfreqs;

float *lpc_coef=NULL;

int xpos[MAXNOD+1], ypos[MAXNOD+1], numsquare=2, selected=0, funcpt;
bool show_node_values;

int theheight;
int thewidth=400;

int areat1=0, areat2=0, areaf1=0, areaf2=0, areach=0;
int spotst[MAX_PAINT], spotsf[MAX_PAINT];
int si_valid=-1, playing=0, xold, numspots=0;
double stretchfac=1., dgain=1., dscale=1.;

int lpc_R=44100, numcoef=60, lpc_samps_per_frame=1;
long lpc_horiz_num=0, lpc_framecnt;
double lpc_duration;




