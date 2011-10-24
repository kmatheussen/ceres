
/*********************************************************************
          Includes
 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <stdbool.h>
/*
#define false 0
#define true 1
#define bool int
*/

#include <pthread.h>

#include "motif.h"

#include <string.h>

#include "python.h"

/*********************************************************************
          Definitions
 ********************************************************************/

#define MAX_SAMPS_PER_FRAME 8


#include "common.h"
#include "config.h"
#include "diskarray.h"

#define MEGFREQ_PUT(place,val) \
 DA_putMem(fftsound->megfreq,(place),val)

#define MEGFREQ_DISK_PUT(place,val) \
 DA_putDiskMem(fftsound->megfreq,(place),val)

#define MEGFREQ_PUT_MUL(place,val) \
 DA_putMemMul(fftsound->megfreq,(place),val)

#define MEGFREQ_PUT_DIV(place,val) \
 DA_putMemDiv(fftsound->megfreq,(place),val)

#define MEGFREQ_PUT_ADD(place,val) \
 DA_putMemAdd(fftsound->megfreq,(place),val)

#define MEGFREQ_PUT_SUB(place,val) \
 DA_putMemSub(fftsound->megfreq,(place),val)

#define MEGFREQ_NEG(place) \
 DA_neg(fftsound->megfreq,(place))


#define MEGAMP_PUT(place,val) \
 DA_putMem(fftsound->megamp,(place),val)

#define MEGAMP_DISK_PUT(place,val) \
 DA_putDiskMem(fftsound->megamp,(place),val)

#define MEGAMP_PUT_MUL(place,val) \
 DA_putMemMul(fftsound->megamp,(place),val)

#define MEGAMP_PUT_DIV(place,val) \
 DA_putMemDiv(fftsound->megamp,(place),val)

#define MEGAMP_PUT_ADD(place,val) \
 DA_putMemAdd(fftsound->megamp,(place),val)

#define MEGAMP_PUT_SUB(place,val) \
 DA_putMemSub(fftsound->megamp,(place),val)

#define MEGAMP_NEG(place) \
 DA_neg(fftsound->megamp,(place))


#define MEGFREQ_DISK_GET(place) \
DA_getDiskMem(fftsound->megfreq,(place))

#define MEGFREQ_GET(place) \
DA_getMem(fftsound->megfreq,(place))

#define MEGAMP_DISK_GET(place) \
DA_getMem(fftsound->megamp,(place))

#define MEGAMP_GET(place) \
DA_getMem(fftsound->megamp,(place))


#define MEGFREQ_COPY(to,from) \
 DA_copyOne(fftsound->megfreq,(to),(from))

#define MEGFREQ_COPYTODISK(to,from) \
 DA_copyOneToDisk(fftsound->megfreq,(to),(from))

#define MEGAMP_COPY(to,from) \
 DA_copyOne(fftsound->megamp,(to),(from))

#define MEGAMP_COPYTODISK(to,from) \
 DA_copyOneToDisk(fftsound->megamp,(to),(from))


////////////////////////////////////


#define MEGFREQ_PUT_C(place,val) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megfreq.dac.s[0].mem[place]=val; \
}else \
DA_putMem_C(fftsound->megfreq,(place)%fftsound->total_channels,(place)/fftsound->total_channels,val); \
}while(0)

#define MEGFREQ_PUT_MUL_C(place,val) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megfreq.dac.s[0].mem[place]*=val; \
}else \
 DA_putMemMul_C(fftsound->megfreq,(place)%fftsound->total_channels,(place)/fftsound->total_channels,val); \
}while(0)


#define MEGFREQ_PUT_DIV_C(place,val) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megfreq.dac.s[0].mem[place]/=val; \
}else \
 DA_putMemDiv_C(fftsound->megfreq,(place)%fftsound->total_channels,(place)/fftsound->total_channels,val); \
}while(0)


#define MEGFREQ_PUT_ADD_C(place,val) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megfreq.dac.s[0].mem[place]+=val; \
}else \
 DA_putMemAdd_C(fftsound->megfreq,(place)%fftsound->total_channels,(place)/fftsound->total_channels,val); \
}while(0)


#define MEGFREQ_PUT_SUB_C(place,val) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megfreq.dac.s[0].mem[place]-=val; \
}else \
 DA_putMemSub_C(fftsound->megfreq,(place)%fftsound->total_channels,(place)/fftsound->total_channels,val); \
}while(0)

#define MEGAMP_PUT_C(place,val) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megamp.dac.s[0].mem[place]=val; \
}else \
 DA_putMem_C(fftsound->megamp,(place)%fftsound->total_channels,(place)/fftsound->total_channels,val); \
}while(0)


#define MEGAMP_PUT_MUL_C(place,val) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megamp.dac.s[0].mem[place]*=val; \
}else \
 DA_putMemMul_C(fftsound->megamp,(place)%fftsound->total_channels,(place)/fftsound->total_channels,val); \
}while(0)


#define MEGAMP_PUT_DIV_C(place,val) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megamp.dac.s[0].mem[place]/=val; \
}else \
 DA_putMemDiv_C(fftsound->megamp,(place)%fftsound->total_channels,(place)/fftsound->total_channels,val); \
}while(0)


#define MEGAMP_PUT_ADD_C(place,val) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megamp.dac.s[0].mem[place]+=val; \
}else \
 DA_putMemAdd_C(fftsound->megamp,(place)%fftsound->total_channels,(place)/fftsound->total_channels,val); \
}while(0)


#define MEGAMP_PUT_SUB_C(place,val) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megamp.dac.s[0].mem[place]-=val; \
}else \
 DA_putMemSub_C(fftsound->megamp,(place)%fftsound->total_channels,(place)/fftsound->total_channels,val); \
}while(0)



#define MEGFREQ_GET2_C(ch,place) (fftsound->usemem==true?fftsound->megfreq.dac.s[0].mem[place]:DA_getMem_C(fftsound->megfreq,(ch),(place)/fftsound->total_channels))

#define MEGAMP_GET2_C(ch,place) (fftsound->usemem==true?fftsound->megamp.dac.s[0].mem[place]:DA_getMem_C(fftsound->megamp,(ch),(place)/fftsound->total_channels))

#define MEGFREQ_GET_C(place) (fftsound->usemem==true?fftsound->megfreq.dac.s[0].mem[place]:DA_getMem_C(fftsound->megfreq,(place)%fftsound->total_channels,(place)/fftsound->total_channels))

#define MEGAMP_GET_C(place) ( \
fftsound->usemem==true \
? \
fftsound->megamp.dac.s[0].mem[place] \
: \
DA_getMem_C(fftsound->megamp,(place)%fftsound->total_channels,(place)/fftsound->total_channels))


#define MEGFREQ_COPY_C(to,from) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megfreq.dac.s[0].mem[to]=fftsound->megfreq.dac.s[0].mem[from]; \
}else \
 DA_copyOne_C(fftsound->megfreq,(to)%fftsound->total_channels,(to)/fftsound->total_channels,(from)%fftsound->total_channels,(from)/fftsound->total_channels); \
}while(0)


#define MEGAMP_COPY_C(to,from) \
do{ \
if(fftsound->usemem==true){ \
fftsound->megamp.dac.s[0].mem[to]=fftsound->megamp.dac.s[0].mem[from]; \
}else \
 DA_copyOne_C(fftsound->megamp,(to)%fftsound->total_channels,(to)/fftsound->total_channels,(from)%fftsound->total_channels,(from)/fftsound->total_channels); \
}while(0)




struct CacheMem{
  float *mem;
  int size;
  int pos;
};

struct FFTSound{

#if 0
  float *megfreq; 
  float *megamp;
#else
  struct DiskArray megamp;
  struct DiskArray megfreq;
#endif

  struct CacheMem cachemem;

  bool usemem;

  struct TempFile *tempfile;          /* Used for converting */


  long horiz_num;	    /* Number of analysis frames */
  double duration;		    /* Duration in secs */
  int numchannels;	    /* Number of phase vocoder channels */

  double binfreq;		    /* Frequency pr. bin */
  int samps_per_frame;

  int total_channels;         /* samps_per_frame*numchannels. */

  double *largamp;

  double *Wanal;
  double *Wsyn;

  int Dn;
  int overlap;

  int NP;
  double P, Pinc;

  int R;
  int N;
  int N2;
  int Nw;
  int Nw2;
  int I;

  double synt;

  int numcoef2;

  bool dontfreeme;  // If false, the struct is fred after playing, in the playthread.
  char *filename;

  pthread_mutex_t mutex;
};

struct RSynthData{
  double **output;
  double **lastphase;
  double **lastamp;
  double **lastfreq;
  double **indx;

  double *alldata;
  int num_channels;

  double *channel;
  double *input;
  double *buffer;
};

/*********************************************************************
          Global variables
 ********************************************************************/

extern Widget sketchpad, topLevel, dw;

extern XtAppContext app_context;
extern Widget mainWindow, menuBar, frame;

extern Widget fileWarning;

extern Widget progressDialog, progressScale;

extern Widget transMenu;

extern float *lpc_coef;

extern int theheight;
extern int thewidth;

extern int areat1, areat2, areaf1, areaf2, areach;
extern int spotst[MAX_PAINT], spotsf[MAX_PAINT];
extern int si_valid, playing, xold, numspots;
extern double stretchfac, dgain, dscale;

extern int lpc_R, numcoef, lpc_samps_per_frame;
extern long lpc_horiz_num, lpc_framecnt;
extern double lpc_duration;

extern XGCValues values;
extern Window theRoot, theWindow;
extern Colormap theCmap;
extern Display *theDisplay;
extern int screen;
extern Pixmap bitmap, spotsmap;
extern XColor gray0, gray25, gray50, gray75, gray100;
extern XColor red, orange, yellow, green, brown;
extern XColor darkblue, blue, lightblue;

extern GC theGC, showGC, funtheGC, funshowGC;
extern GC theGC0, theGC25, theGC50, theGC75, theGC100;
extern GC showGC0, showGC25, showGC50, showGC75, showGC100;
extern GC funcGC, func2GC,func3GC,gridGC, spotsGC, markerGC;

extern int xpos[MAXNOD+1], ypos[MAXNOD+1], numsquare, selected, funcpt;
extern int xpos2[MAXNOD+1], ypos2[MAXNOD+1], numsquare2, selected2, funcpt2;
extern int xpos3[MAXNOD+1], ypos3[MAXNOD+1], numsquare3, selected3, funcpt3;
bool show_node_values;

extern int farge;
extern double gridfreqs[600], gridfreqs2[600];
extern int numfreqs;


extern char *outportnames[MAX_SAMPS_PER_FRAME]; // For jack.


/*********************************************************************
          Global functions
 ********************************************************************/


/* lpc */
extern double lpamp(double, double, double *, int);
extern double lpa(double *, int, double *, int, int);

/* draw, etc. */

void Cancel(Widget w,XtPointer client, XtPointer call);
void RedrawWin(struct FFTSound *fftsound);
void RestoreWin(struct FFTSound *fftsound);
void SetWatchCursor(Widget w);
void drawscale(struct FFTSound *fftsound);
void RedrawFunc(void);
void ResetCursor(Widget w);
double getfuncval(struct FFTSound *fftsound,int hnum);
double getfuncval2(struct FFTSound *fftsound,int hnum);
double getfuncval3 (struct FFTSound *fftsound,int vnum3);
void SetVisibleFunc(int funcnum,int f);
void SetFuncMinMax(int funcnum,float minvalue,float maxvalue);
void setgreycolors(void);
void loadana(struct FFTSound *fftsound,char *filename);
void Playlast(struct FFTSound *fftsound);
void setgreycolors(void);
void sethotcolors(void);
void setcoldcolors(void);
void setSettings(struct FFTSound *fftsound);
void EDIT_showcont1(Widget w, XtPointer client, XtPointer call);
void EDIT_showcont2(Widget w, XtPointer client, XtPointer call);
void EDIT_showcont3(Widget w, XtPointer client, XtPointer call);
void EDIT_showgrid(Widget w, XtPointer client, XtPointer call);
void EDIT_showpaint(Widget w, XtPointer client, XtPointer call);
void EDIT_setUndoRedoMenues(void);

/* Init code (called from main) */

void create_phasevocoder(struct FFTSound *fftsound,int windowsize);
void create_transforms(struct FFTSound *fftsound);
void create_gui(struct FFTSound *fftsound);
void create_play(struct FFTSound *fftsound);
void createUndo(struct FFTSound *fftsound);
void create_drawing(struct FFTSound *fftsound);
void create_file(struct FFTSound *fftsound);
void create_export(struct FFTSound *fftsound);
void create_settings(struct FFTSound *fftsound);
void create_paint(struct FFTSound *fftsound);
void create_extract(struct FFTSound *fftsound);
void create_graph(struct FFTSound *fftsound);
void create_lpc2(struct FFTSound *fftsound);
void create_help(struct FFTSound *fftsound);
void create_tempfile(void);
void create_edit(struct FFTSound *fftsound);



