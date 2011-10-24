
#include <stdint.h>
#include "mysndfile.h"
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "ceres.h"
#include "fft.h"
#include "phasevocoder.h"
#include "rsynthdata.h"
#include "play.h"
#include "undo.h"

static Widget stop;
static Widget loadFileBox, saveFileBox, fileButton, fileMenu, quit, load, loadimage, loadbrk, savebrk, save, play, imageFileBox, brkFileBox,  brkSaveFileBox;
static Widget loadLpc, loadLpcBox;

static Widget brkRowCol,brkRadio1;

static Widget sizeWarning,writeWarning;

static Widget loadFileInfo;
static Widget fileInfoRC,loadSepar1;
static Widget  infoL1, infoL2, infoL3;


/*
static long samp_type,filefmt,compression,vers,framecnt;
static long bits_per_samp;
*/

Boolean fileDirty = FALSE;

// #define SGISOUNDFILELIBSTUFF

struct LoadStruct{
  SNDFILE *infile;
  SF_INFO sfinfo;  
};

static struct LoadStruct loadstruct;



#ifdef SNDFILE_0

int my_sf_readf_double(struct LoadStruct *ls,double *to,int ch,int frames){
  short val[ls->sfinfo.channels*frames];
  int val1[ls->sfinfo.channels*frames];
  float val2[ls->sfinfo.channels*frames];
  double val3[ls->sfinfo.channels*frames];
  int ret;
  int lokke;

  switch(ls->sfinfo.format&0xff){

#define DASREAD(a,b,c) \
ret=a(ls->infile,b,frames);\
for(lokke=0;lokke<ret;lokke++) \
  to[lokke]=b[ch+lokke*ls->sfinfo.channels]/c

  case 0x1:
    switch(ls->sfinfo.pcmbitwidth){
    case 16:
      DASREAD(sf_readf_short,val,32768.0);
      break;
    case 24:
      DASREAD(sf_readf_int,val1,8388608.0);
      break;
    case 32:
      DASREAD(sf_readf_int,val1,2147483647);
      break;
    default:
      fprintf(stderr,"Fileformat is not supported.\n");
      ret=0;
      break;
    }
    break;
  case 0x2:

    switch(ls->sfinfo.pcmbitwidth){
    case 32:
      ret=sf_readf_float(ls->infile,val2,1);
      for(lokke=0;lokke<ret;lokke++)
	to[lokke]=(double)val2[ch+lokke*ls->sfinfo.channels];
      break;
    case 64:
      ret=sf_readf_double(ls->infile,val3,1,0);
      for(lokke=0;lokke<ret;lokke++)
	to[lokke]=val3[ch+lokke*ls->sfinfo.channels];
    default:
      fprintf(stderr,"Fileformat is not supported.\n");
      ret=0;
      break;
    }
  default:
    fprintf(stderr,"Fileformat is not supported.\n");
    ret=0;
    break;
  }
  return ret;
}

#endif


/* Loads into Aptr, maximum number of frames to read D, at channel ch. */
/* Returns actually number of frames read. */

int LoadWaveProvidor(
		      struct FFTSound *fftsound,
		      void *pointer,
		      double *Aptr,
		      int D,
		      int ch
		      )
{

  struct LoadStruct *ls=(struct LoadStruct*)pointer;

#ifdef SNDFILE_0
  return my_sf_readf_double(ls,Aptr,ch,D);
#else
  int ret,lokke;
  double val3[ls->sfinfo.channels*D];

  ret=sf_readf_double(ls->infile,val3,D);
  for(lokke=0;lokke<ret;lokke++)
    Aptr[lokke]=val3[ch+lokke*ls->sfinfo.channels];

  return ret;

#endif


#if 0
  int lokke=0;

  do{
    if(my_sf_readf_double(ls,Aptr,ch,D)==0) break;
    Aptr++;
    lokke++;
  }while(lokke<D);

  return lokke;
#endif
			    
}


void loadana(struct FFTSound *fftsound,char *filename)
{

  int num=0;
  int in, i, ch, eof, grafx, grafold, j, y;
  long point, p;
  double amp;
  XPoint list0[LSIZE],list1[LSIZE],list2[LSIZE],list3[LSIZE];
  int c0=0,c1=0,c2=0,c3=0;
  double a;
  struct RSynthData *rsd;
  SF_INFO *sfinfo=&loadstruct.sfinfo;

  SNDFILE *infile=sf_open_read(filename,&loadstruct.sfinfo);

  loadstruct.infile=infile;

  if(infile==NULL){
    XtManageChild(fileWarning);
    return;
  }
  if(!sf_format_check(sfinfo)){
    fprintf(stderr,"Unknown file-format\n");
    sf_close(infile);
    return;
  }
  if(sfinfo->channels>MAX_SAMPS_PER_FRAME){
    fprintf(stderr,"%d channel soundfiles not supported. Max %d channels.\n",sfinfo->channels,MAX_SAMPS_PER_FRAME);
    sf_close(infile);
    return;
  }

  PlayStopHard();

  fftsound->samps_per_frame=sfinfo->channels;
  fftsound->total_channels=fftsound->numchannels*fftsound->samps_per_frame;

  fftsound->R=sfinfo->samplerate;

  fftsound->horiz_num = (sfinfo->MSF_FRAMENAME/fftsound->Dn)+fftsound->overlap;

  fftsound->duration = (double)sfinfo->MSF_FRAMENAME/fftsound->R;
  fftsound->binfreq = (double)fftsound->R/fftsound->N;

  DA_closeFile(fftsound);
  UNDO_Reset();
  DA_newFile(fftsound);

  fvec(fftsound->largamp, fftsound->horiz_num);
  if (fftsound->largamp==NULL) {
    XtManageChild(sizeWarning);
    sf_close(infile); 
    return;
  }

  SetWatchCursor(topLevel);
  areat1=0; areat2=fftsound->horiz_num;
  areaf1=0; areaf2=fftsound->numchannels;

  XFillRectangle(XtDisplay(sketchpad), bitmap, theGC100,
    0, 0, thewidth+70, theheight+70);
  XClearArea(XtDisplay(sketchpad), XtWindow(sketchpad), 0, 0, 0, 0, True);

  rsd=getRSynthData(fftsound);

  drawscale(fftsound);

  fftsound->filename=malloc(strlen(filename)+1);
  sprintf(fftsound->filename,"%s",filename);

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    eof=0; si_valid=-1; point=ch*fftsound->numchannels*fftsound->horiz_num;
    p=0; in=-fftsound->Nw, j=0; grafold=-1;

    sf_seek(infile,0,SEEK_SET);

    while (!eof) {
      in+=fftsound->Dn;
      eof=shiftin(fftsound, LoadWaveProvidor,&loadstruct,rsd->input,  fftsound->Nw,  fftsound->Dn, &amp, ch);
      if (ch==0) fftsound->largamp[p++]=amp;
      fold(rsd->input,  fftsound->Wanal,  fftsound->Nw,  rsd->buffer,  fftsound->N,  in);
      rfft(rsd->buffer,  fftsound->N2,  FORWARD);
      convert(rsd, rsd->buffer,  rsd->channel,  fftsound->N2,  fftsound->Dn,  fftsound->R, ch);
      grafx = (int)(j++*(double)thewidth/fftsound->horiz_num)+50;

      for (i=0; i<fftsound->numchannels; i++) { 
	MEGAMP_PUT(point,rsd->channel[i+i]);
	MEGFREQ_PUT(point,rsd->channel[i+i+1]);
        point++;
      }

      if (grafx!=grafold) {
        point-=fftsound->numchannels;
	for (i=0; i<fftsound->numchannels; i++) {
	  a=dgain*MEGAMP_GET(point);
	  if (a>0.0000012) {
	    y=(int)(dscale*MEGFREQ_GET(point)/fftsound->binfreq);
	    if (y<theheight) {        
	      num++;
	      y = ((ch+1)*theheight-y)/fftsound->samps_per_frame+10;
	      if (a>0.0000100) {
		ADD_POINT(grafx,y,list0,c0,showGC0,theGC0);
	      } else if (a>0.0000050){ 
		ADD_POINT(grafx,y,list1,c1,showGC25,theGC25);
	      } else if (a>0.0000025) {
		ADD_POINT(grafx,y,list2,c2,showGC50,theGC50);
	      } else {
		ADD_POINT(grafx,y,list3,c3,showGC75,theGC75);
	      }
	    }
	  }
	  point++;
	}
        grafold=grafx;

      }
    }
  }
  returnRSynthData(rsd);

  FLUSH_LIST(list0,c0,showGC0,theGC0);
  FLUSH_LIST(list1,c1,showGC25,theGC25);
  FLUSH_LIST(list2,c2,showGC50,theGC50);
  FLUSH_LIST(list3,c3,showGC75,theGC75);

  sf_close(infile);

  fileDirty = FALSE;

  RedrawFunc();
  ResetCursor(topLevel);
}



/* GetFileInfo is based on code from Ceres3. */

void GetFileInfo(Widget w,XtPointer client,XtPointer call)
{
  SF_INFO sfinfo={0};
  SNDFILE *infile;

  XmString text;
  char  str[100];

  DIR *dirp;
  char *filename=XmTextGetString(w);

  if ((dirp=opendir(filename))) {
    closedir(dirp);
    return;
  }

//  fprintf(stderr,"filename: -%s-\n",filename==NULL?"NULL":filename);

  infile=sf_open_read(filename,&sfinfo);
  if(infile==NULL){
    sprintf(str,"Unknown file-format.");
    sfinfo.samplerate=1;
  }else{
    sf_close(infile);
    if(!sf_format_check(&sfinfo)){
      fprintf(stderr,"Unknown file-format.\n");
      return;
    }


    strcpy(str,"File Format:  ");
    if (sfinfo.format&0x10000) strcat(str,"WAV");
    else if (sfinfo.format&0x20000) strcat(str,"AIFF");
    else strcat(str,"Other format");

    if (sfinfo.channels==1) strcat(str,", Mono.");
    else if (sfinfo.channels==2) strcat(str,", Stereo.");
    else if (sfinfo.channels>MAX_SAMPS_PER_FRAME) strcat(str,", Unsupported number of channels.");
    else sprintf(str,"%s, %d channels",str,sfinfo.channels);
  }

  text=XmStringCreateSimple(str);
  XtVaSetValues(infoL1,XmNlabelString,text,NULL); XmStringFree(text);
  sprintf(str,"Size:  %d frames, %.2f seconds.",(int)sfinfo.MSF_FRAMENAME,(double)sfinfo.MSF_FRAMENAME/sfinfo.samplerate);
  text=XmStringCreateSimple(str);
  XtVaSetValues(infoL2,XmNlabelString,text,NULL); XmStringFree(text);
  sprintf(str,"Sampling rate:  %d Hz.",sfinfo.samplerate);
  text=XmStringCreateSimple(str);
  XtVaSetValues(infoL3,XmNlabelString,text,NULL); XmStringFree(text);



//  close(fd);
  XtFree(filename);

}

void Load(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(loadFileBox);
}

void LoadOk(w, client, call)
Widget w;
XtPointer client, call;
{
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call;
  char *filename;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
  struct FFTSound *fftsound=(struct FFTSound *)client;

  XtUnmanageChild(loadFileBox); XmUpdateDisplay(w);
  XmStringGetLtoR(calldata->value, charset, &filename);

  loadana(fftsound,filename);
  XtFree(filename);

}

void loadlpc(struct FFTSound *fftsound,char *filename)
{
  int i, ch, eof, pr, lastpr=-1, S=0;
  long point, j=0;
  double amp;
  double *coef;
  struct RSynthData *rsd;

  SF_INFO *sfinfo=&loadstruct.sfinfo;

  SNDFILE *infile;

  PlayStopHard();

  if (CONFIG_getBool("stabelize_poles")==true) S=1;

  infile=sf_open_read(filename,sfinfo);
  loadstruct.infile=infile;
//  infile=AFopenfile(filename, "r", in_AFsetup);

  if (infile==NULL) {
    XtManageChild(fileWarning);
    return;
  }

  lpc_samps_per_frame   = sfinfo->channels;

  lpc_R                 = sfinfo->samplerate;

  /*
  compression       = AFgetcompression(infile, AF_DEFAULT_TRACK);
  filefmt           = AFgetfilefmt(infile, &vers);   
  AFgetsampfmt(infile, AF_DEFAULT_TRACK, &samp_type, &bits_per_samp);
  */

  lpc_framecnt          = sfinfo->MSF_FRAMENAME;
  
  lpc_horiz_num = (lpc_framecnt/fftsound->Dn)+fftsound->overlap;

  /*
  if (numcoef*lpc_horiz_num*lpc_samps_per_frame > 32000000) {
    XtManageChild(sizeWarning);
    AFclosefile(infile); 
    return;
  } 
  */

  lpc_duration = (double)lpc_framecnt/lpc_R;
  if (lpc_coef!=NULL) free(lpc_coef);
  lpc_coef=NULL;
  ffvec(lpc_coef, (numcoef+2)*lpc_horiz_num*lpc_samps_per_frame);
  if (lpc_coef==NULL) {
    XtManageChild(sizeWarning);
    sf_close(infile);
    return;
  }
  fvec(coef, numcoef+2);

  //  printf("lpc_samps_per_frame: %d, lpc_R: %d, lpc_framecnt: %d, lpc_horiz_num: %d, lpc_duration: %f, numcoef: %d\n",
  //	 lpc_samps_per_frame,lpc_R,lpc_framecnt,lpc_horiz_num,(float)lpc_duration,numcoef);

  SetWatchCursor(topLevel);
  XtVaSetValues(progressScale, XmNvalue, 0, NULL);
  XtManageChild(progressDialog);

  while (XtAppPending(app_context)) {
     XtAppProcessEvent(app_context,XtIMXEvent|XtIMTimer|XtIMAlternateInput);
  }

  rsd=getRSynthData(fftsound);

  for (ch=0; ch<lpc_samps_per_frame; ch++) {
    eof=0; si_valid=-1;
    point=ch*(numcoef+2)*lpc_horiz_num;

    sf_seek(infile,0,SEEK_SET);
    while (!eof) {
      eof=shiftin(fftsound, LoadWaveProvidor,&loadstruct,rsd->input,  fftsound->Nw,  fftsound->Dn, &amp, ch);
      coef[numcoef+1]=lpa(rsd->input, fftsound->Nw, coef, numcoef, S);
      for (i=0; i<numcoef+2; i++) {
 	lpc_coef[point]=coef[i];
        point++;
      }
      j++;
      pr=(int)(100.*(double)j/(lpc_samps_per_frame*lpc_horiz_num));
      if (pr!=lastpr) {
        XtVaSetValues(progressScale, XmNvalue, pr, NULL);
        XmUpdateDisplay(topLevel);
        lastpr=pr;
      }
    }
  }

  returnRSynthData(rsd);

  fftsound->numcoef2=numcoef;

  sf_close(infile);

  free(coef);
  XtUnmanageChild(progressDialog);
  ResetCursor(topLevel);

}


void LoadLpc(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(loadLpcBox);
}

void LoadLpcOk(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call;
  char *filename;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;

  XtUnmanageChild(loadLpcBox); XmUpdateDisplay(w);
  XmStringGetLtoR(calldata->value, charset, &filename);

  loadlpc(fftsound,filename);
  XtFree(filename);
}


void Image(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(imageFileBox);
}

void ImageOk(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call;
  char *filename, line[200];
  XPoint list0[LSIZE],list1[LSIZE],list2[LSIZE],list3[LSIZE];
  int c0=0,c1=0,c2=0,c3=0;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
  int i, j=0, grafx, grafold=-1, x, y, xc, yc, val1, val2, val3;
  long point=0;
  FILE *infile;
  double fr, a;

  XtUnmanageChild(imageFileBox);
  SetWatchCursor(topLevel);
  XmStringGetLtoR(calldata->value, charset, &filename);

  infile=fopen(filename, "r");
  if (infile==NULL) {
    ResetCursor(topLevel);
    XtManageChild(fileWarning);
    XtFree(filename);
    return;
  }

  PlayStopHard();

  fgets(line, 200, infile); /* P3 */
  fgets(line, 200, infile); /* # CREATOR */
  fgets(line, 200, infile); if (line[0]=='#') fgets(line, 200, infile);
  sscanf(line,"%d %d",&x,&y); printf("%d %d\n",x,y);
  fgets(line, 200, infile);

  fftsound->samps_per_frame   = 1;
  fftsound->total_channels=fftsound->numchannels*fftsound->samps_per_frame;
  fftsound->R                 = 44100;

  /*
  compression       = AF_COMPRESSION_NONE;
  filefmt           = AF_FILE_AIFF;  
  samp_type         = AF_SAMPFMT_TWOSCOMP;

  bits_per_samp     = 16;
  */
#ifdef SNDFILE_0
  loadstruct.sfinfo.pcmbitwidth=16;
#else
  loadstruct.sfinfo.format=SF_FORMAT_WAV | SF_FORMAT_PCM_16 ;
#endif

#if 0
  sf_count_t  frames ;     /* Used to be called samples. */
  int         samplerate ;
  int         channels ;
  int         format ;
  int         sections ;
  int         seekable ;
#endif

  loadstruct.sfinfo.samplerate=fftsound->R;
  loadstruct.sfinfo.channels=fftsound->samps_per_frame;

	   

  fftsound->horiz_num=x;
  loadstruct.sfinfo.MSF_FRAMENAME=(fftsound->horiz_num-fftsound->overlap)*fftsound->Dn;

#if 0
  if (fftsound->numchannels*fftsound->horiz_num > 16000000) {
    ResetCursor(topLevel);
    XtManageChild(sizeWarning);
    fclose(infile); 
    XtFree(filename);
    return;
  } 
#endif

  fftsound->duration = (double)loadstruct.sfinfo.MSF_FRAMENAME/fftsound->R;
  fftsound->binfreq = (double)fftsound->R/fftsound->N;

#if 0
  if (fftsound->megfreq!=NULL) free(fftsound->megfreq);
  if (fftsound->megamp!=NULL) free(fftsound->megamp);
  fftsound->megfreq=fftsound->megamp=NULL;
  ffvec(fftsound->megfreq, fftsound->numchannels*fftsound->horiz_num);
  if (fftsound->megfreq==NULL) {
    ResetCursor(topLevel);
    XtManageChild(sizeWarning);
    fclose(infile); 
    XtFree(filename);
    return;
  }
  ffvec(fftsound->megamp, fftsound->numchannels*fftsound->horiz_num);
  if (fftsound->megamp==NULL) {
    ResetCursor(topLevel);
    XtManageChild(sizeWarning);
    fclose(infile); 
    XtFree(filename);
    return;
  }
#endif

  DA_closeFile(fftsound);
  UNDO_Reset();
  DA_newFile(fftsound);

  areat1=0; areat2=fftsound->horiz_num;
  areaf1=0; areaf2=fftsound->numchannels;

  XFillRectangle(XtDisplay(sketchpad), bitmap, theGC100,
    0, 0, thewidth+70, theheight+70);
  XClearArea(XtDisplay(sketchpad), XtWindow(sketchpad), 0, 0, 0, 0, True);

  drawscale(fftsound);

  fr=(y+1)*fftsound->binfreq;
  point=0;

  fprintf(stderr,"end: %d\n",(fftsound)->megfreq.dac.s[0].end);

  for (xc=0; xc<x; xc++)
    for (yc=0; yc<fftsound->numchannels; yc++,point++){
      //fprintf(stderr,"point: %d\n",point);
      MEGFREQ_PUT(point,50.);
      //      fftsound->megfreq[point++]=50.;
    }
  
  for (yc=0; yc<y; yc++) {
    fr-=fftsound->binfreq;
    for (i=0; i<x; i++) {
        fscanf(infile,"%d %d %d", &val1, &val2, &val3);
        val1=255-val1; val2=255-val2; val3=255-val3;
        if (y-yc<511) {
	  MEGAMP_PUT(i*fftsound->numchannels+y-yc,(double)val1*(double)val2*(double)val3*0.000000000001);
	  MEGFREQ_PUT(i*fftsound->numchannels+y-yc,fr+(double)rand()/RAND_MAX*fftsound->binfreq);
	  /*
	  fftsound->megamp[i*fftsound->numchannels+y-yc]=(double)val1*(double)val2*(double)val3
            *0.000000000001;
	  fftsound->megfreq[i*fftsound->numchannels+y-yc]=fr+(double)rand()/RAND_MAX*fftsound->binfreq;
	  */
        }
    }
  }
  for (xc=0; xc<x; xc++) {
    grafx = (int)(j++*(double)thewidth/fftsound->horiz_num)+50;
    if (grafx!=grafold) {
      point=xc*fftsound->numchannels;
        for (i=0; i<fftsound->numchannels; i++) {
          a=dgain*MEGAMP_GET(point);
          if (a>0.0000012) {        
            int y = theheight-(int)(MEGFREQ_GET(point)/fftsound->binfreq)+10;
            if (a>0.0000100) {
           ADD_POINT(grafx,y,list0,c0,showGC0,theGC0);
            } else if (a>0.0000050){ 
           ADD_POINT(grafx,y,list1,c1,showGC25,theGC25);
            } else if (a>0.0000025) {
           ADD_POINT(grafx,y,list2,c2,showGC50,theGC50);
            } else {
           ADD_POINT(grafx,y,list3,c3,showGC75,theGC75);
            }
          }
          point++;
        }
      grafold=grafx;
    }
  }
  FLUSH_LIST(list0,c0,showGC0,theGC0);
  FLUSH_LIST(list1,c1,showGC25,theGC25);
  FLUSH_LIST(list2,c2,showGC50,theGC50);
  FLUSH_LIST(list3,c3,showGC75,theGC75);

  rightbins(fftsound);

  fclose(infile); 
  XtFree(filename);
  fileDirty = FALSE;
  RedrawFunc();
  ResetCursor(topLevel);
}

void LoadBrk(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(brkFileBox);
}

void LoadBrkOk(w, client, call)
Widget w;
XtPointer client, call;
{
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call;
  char *filename;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
  float x, y;
  int ant=0;
  FILE *infile;
  char temp[500];

  XtUnmanageChild(brkFileBox);
  XmStringGetLtoR(calldata->value, charset, &filename);

  infile=fopen(filename, "r");
  if (infile==NULL) {
    ResetCursor(topLevel);
    XtManageChild(fileWarning);
    XtFree(filename);
    fprintf(stderr,"File not found.\n");
    return;
  }

  if(fgets(temp,499,infile)==NULL){
    fclose(infile);
    ResetCursor(topLevel);
    XtManageChild(fileWarning);
    XtFree(filename);
    fprintf(stderr,"Strange file.\n");
    return;
  }

  if(!strcmp("CERESCONTROL\n",temp)){
    fprintf(stderr,"gakk gakk\n");
  }else{
    rewind(infile);
  }

  PlayStopHard();

  while (1) {
    fscanf(infile, "%f", &x);
    if (feof(infile)) break;
    fscanf(infile, "%f", &y);
    if (feof(infile)) break;
    if (ant>=MAXNOD) break;
    xpos[ant+1]=x*thewidth+50;
    ypos[ant+1]=(1.-y)*theheight+10;
    ant++;
  }
  fclose(infile); 
  XtFree(filename);

  if (ant>0) {
    xpos[1]=50; xpos[ant]=thewidth+50;
    numsquare=ant;
  }

  RedrawFunc();
}

void SaveBrk(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(brkSaveFileBox);
}

void SaveBrkOk(w, client, call)
Widget w;
XtPointer client, call;
{
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call;
  char *filename;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
//  double x, y;
  int ant=0;
  FILE *outfile;

  XtUnmanageChild(brkSaveFileBox);
  XmStringGetLtoR(calldata->value, charset, &filename);

  PlayStopHard();

  outfile=fopen(filename, "w");
  if (outfile==NULL) {
    ResetCursor(topLevel);
    XtManageChild(fileWarning);
    XtFree(filename);
    return;
  }

  for (ant=1; ant<=numsquare; ant++) {
    fprintf(outfile, "%f\t%f\n", (xpos[ant]-50)/(double)thewidth,
      1.-(ypos[ant]-10.)/theheight);
  }
  fclose(outfile); 
  XtFree(filename);
}

void Save(w, client, call)
Widget w;
XtPointer client, call;
{
  XtManageChild(saveFileBox);
}


struct SaveStruct{
  int lastptr;
  struct FFTSound *fftsound;
  SNDFILE *outfile;
  SF_INFO *sfinfo;  
};


void SaveWaveConsumer(
		      struct FFTSound *fftsound,
		      void *pointer,
		      double **samples,
		      int num_samples
		      )
{
  struct SaveStruct *ss=(struct SaveStruct*)pointer;
  int i,ch;

  if(MSF_ISFLOATTYPE(loadstruct.sfinfo)){
    if(MSF_ISFLOAT(loadstruct.sfinfo)){
      float framebuff[fftsound->Nw*2*MAX_SAMPS_PER_FRAME];
      for (i=0; i<num_samples; i++) {
	for (ch=0; ch<fftsound->samps_per_frame; ch++)
	  *(framebuff+i*fftsound->samps_per_frame+ch)=(float)(samples[ch][i]);
      }
      sf_writef_float(ss->outfile,framebuff,num_samples);
    }else{
      double framebuff[fftsound->Nw*2*MAX_SAMPS_PER_FRAME];
      for (i=0; i<num_samples; i++) {
	for (ch=0; ch<fftsound->samps_per_frame; ch++)
	  *(framebuff+i*fftsound->samps_per_frame+ch)=samples[ch][i];
      }
      sf_writef_double(ss->outfile,framebuff,num_samples
#ifdef SNDFILE_0
,0
#endif
);
    }
    return;
  }

  if (MSF_IS16(loadstruct.sfinfo)) {
    int16_t framebuff[fftsound->Nw*2*MAX_SAMPS_PER_FRAME];
    for (i=0; i<num_samples; i++) {
      for (ch=0; ch<fftsound->samps_per_frame; ch++)
	*(framebuff+i*fftsound->samps_per_frame+ch)=(short)(samples[ch][i]*32767.);
    }
    sf_writef_short(ss->outfile,framebuff,num_samples);
  } else if (MSF_IS24(loadstruct.sfinfo)) {
    int32_t iframebuff[fftsound->Nw*2*MAX_SAMPS_PER_FRAME];
    for (i=0; i<num_samples; i++) {
      for (ch=0; ch<fftsound->samps_per_frame; ch++)
	*(iframebuff+i*fftsound->samps_per_frame+ch)=(int)(samples[ch][i]*8388608.);
    }
    sf_writef_int(ss->outfile,iframebuff,num_samples);
  } else if (MSF_IS32(loadstruct.sfinfo)){
    int32_t iframebuff[fftsound->Nw*2*MAX_SAMPS_PER_FRAME];
    for (i=0; i<num_samples; i++) {
      for (ch=0; ch<fftsound->samps_per_frame; ch++)
	*(iframebuff+i*fftsound->samps_per_frame+ch)=(int)(samples[ch][i]*2147483647.0);
    }
    sf_writef_int(ss->outfile,iframebuff,num_samples);
  }

}

bool SaveProgressUpdate(int j,void *pointer){
  struct SaveStruct *ss=(struct SaveStruct*)pointer;
  int pr=(int)(100.*(double)j/ss->fftsound->horiz_num);

  if(ss->lastptr!=pr){
    XtVaSetValues(progressScale, XmNvalue, pr, NULL);
    XmUpdateDisplay(topLevel);
    ss->lastptr=pr;
  }
  return true;
}

void SaveOk(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  XmFileSelectionBoxCallbackStruct
    *calldata=(XmFileSelectionBoxCallbackStruct*) call; 
  char *filename;
  static XmStringCharSet charset =
                           (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;

  struct SaveStruct ss;
  struct RSynthData *rsd;

  ss.lastptr=-1;
  ss.fftsound=fftsound;
  ss.sfinfo=&loadstruct.sfinfo;

  SetWatchCursor(topLevel);
  XtUnmanageChild(saveFileBox);
  XmUpdateDisplay(w);

  XmStringGetLtoR(calldata->value, charset, &filename);

  ss.outfile=sf_open_write(filename,ss.sfinfo);

  /*
  out_AFsetup=AFnewfilesetup();
  AFinitchannels(out_AFsetup, AF_DEFAULT_TRACK, fftsound->samps_per_frame);
  AFinitrate(out_AFsetup, AF_DEFAULT_TRACK, fftsound->R);
  AFinitcompression(out_AFsetup, AF_DEFAULT_TRACK, compression);
  AFinitfilefmt(out_AFsetup, filefmt);
  AFinitsampfmt(out_AFsetup, AF_DEFAULT_TRACK, samp_type, bits_per_samp);
  outfile=AFopenfile(filename, "w", out_AFsetup);
  */

  if (ss.outfile==NULL) {
    ResetCursor(topLevel);
    XtManageChild(writeWarning);
    XtFree(filename);
    return;
  }

  XtVaSetValues(progressScale, XmNvalue, 0, NULL);
  XtManageChild(progressDialog);

  while (XtAppPending(app_context)) {
     XtAppProcessEvent(app_context,XtIMXEvent|XtIMTimer|XtIMAlternateInput);
  }

  PlayStopHard();

  rsd=getRSynthData(fftsound);

  /*****************************/
  makeWaves(
	    fftsound,rsd,
	    SaveWaveConsumer,SaveProgressUpdate,
	    &ss,0,fftsound->horiz_num,
	    CONFIG_getBool("st_additive"),
            CONFIG_getBool("keep_formants")
	    );
  /*****************************/

  returnRSynthData(rsd);

  sf_close(ss.outfile);

//  AFclosefile(outfile);

  fileDirty = FALSE;

  XtFree(filename);

  XtUnmanageChild(progressDialog);
  ResetCursor(topLevel);
  
}

void PlayOk(w, client, call)
Widget w;
XtPointer client, call;
{
  struct FFTSound *fftsound=(struct FFTSound *)client;
  fprintf(stderr,"Playing %p %p %p\n",fftsound,areat1,areat2);
  Play(fftsound,areat1,areat2);
}

void StopOk(w, client, call)
Widget w;
XtPointer client, call;
{
  PlayStop();
}

void Quit(w, client, call)
Widget w;
XtPointer client, call;
{
  exit(0);
}


void create_file(struct FFTSound *fftsound){
  static Widget temp;

  XmString text;

    /* CREATE FILE MENU */
  fileButton=XtVaCreateManagedWidget(
				     "fileButton",xmCascadeButtonWidgetClass,menuBar,NULL);
  text=XmStringCreateLocalized("File");
  XtVaSetValues(fileButton,XmNlabelString,text,NULL);
  
  fileMenu=XmCreatePulldownMenu(menuBar,"fileMenu",NULL,0);
  XtVaSetValues(fileButton,XmNsubMenuId,fileMenu,NULL);

  /* CREATE LOAD & ANALYZE BUTTON */
  load=XtVaCreateManagedWidget(
    "load",xmPushButtonWidgetClass,fileMenu,
    XmNmnemonic,XStringToKeysym("O"),XmNaccelerator,"Alt<Key>O",
    XmNacceleratorText,XmStringCreateLocalized("Alt+O"),
    NULL);
  text=XmStringCreateLocalized("Load & Analyze");
  XtVaSetValues(load,XmNlabelString,text,NULL);

  loadFileBox=XmCreateFileSelectionDialog(mainWindow,"loadFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(loadFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Load & Analyze");
  XtVaSetValues(loadFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,500,
#endif
		NULL);

  fileInfoRC=XtVaCreateManagedWidget("fileInfoRC",xmRowColumnWidgetClass,
				     loadFileBox,NULL);
  loadSepar1=XtVaCreateManagedWidget("loadSepar1",xmSeparatorWidgetClass,
				     fileInfoRC,NULL);
  text=XmStringCreateLocalized("(File Info:)");
  infoL1=XtVaCreateManagedWidget("infoL1",xmLabelWidgetClass,
				 fileInfoRC,XmNlabelString,text,NULL);
  text=XmStringCreateLocalized(" ");
  infoL2=XtVaCreateManagedWidget("infoL2",xmLabelWidgetClass,
				 fileInfoRC,XmNlabelString,text,NULL);
  infoL3=XtVaCreateManagedWidget("infoL3",xmLabelWidgetClass,
				 fileInfoRC,XmNlabelString,text,NULL);

  loadFileInfo=XmFileSelectionBoxGetChild(loadFileBox,XmDIALOG_TEXT);
  XtAddCallback(loadFileInfo,XmNvalueChangedCallback,GetFileInfo,NULL);



  XtAddCallback(loadFileBox,XmNcancelCallback,Cancel,loadFileBox);
  XtAddCallback(loadFileBox,XmNokCallback,LoadOk,fftsound);
  XtAddCallback(load,XmNactivateCallback,Load,fftsound);


  /* CREATE SYNTH & SAVE BUTTON */
  save=XtVaCreateManagedWidget(
    "save",xmPushButtonWidgetClass,fileMenu,
    XmNmnemonic,XStringToKeysym("W"),XmNaccelerator,"Alt<Key>W",
    XmNacceleratorText,XmStringCreateLocalized("Alt+W"),
    NULL);
  text=XmStringCreateLocalized("Synth & Save");
  XtVaSetValues(save,XmNlabelString,text,NULL);

  saveFileBox=XmCreateFileSelectionDialog(mainWindow,"saveFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(saveFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Synth & Save");
  XtVaSetValues(saveFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,500,
#endif
		NULL);
  XtAddCallback(saveFileBox,XmNcancelCallback,Cancel,saveFileBox);
  XtAddCallback(saveFileBox,XmNokCallback,SaveOk,fftsound);
  XtAddCallback(save,XmNactivateCallback,Save,fftsound);


  /* CREATE LOADLPC BUTTON */
  loadLpc=XtVaCreateManagedWidget(
    "loadLpc",xmPushButtonWidgetClass,fileMenu,NULL);
  text=XmStringCreateLocalized("Load & Analyze LPC");
  XtVaSetValues(loadLpc,XmNlabelString,text,NULL);

  loadLpcBox=XmCreateFileSelectionDialog(mainWindow,"loadLpcBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(loadLpcBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Load & Analyze LPC");
  XtVaSetValues(loadLpcBox,XmNdialogTitle,text,
    XmNwidth,500,NULL);

  XtAddCallback(loadLpcBox,XmNcancelCallback,Cancel,loadLpcBox);
  XtAddCallback(loadLpcBox,XmNokCallback,LoadLpcOk,fftsound);
  XtAddCallback(loadLpc,XmNactivateCallback,LoadLpc,fftsound);


  /* CREATE LOAD IMAGE BUTTON */
  loadimage=XtVaCreateManagedWidget(
    "loadimage",xmPushButtonWidgetClass,fileMenu,NULL);
  text=XmStringCreateLocalized("Load PBM ascii image");
  XtVaSetValues(loadimage,XmNlabelString,text,NULL);

  imageFileBox=XmCreateFileSelectionDialog(mainWindow,"imageFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(imageFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Load PBM ascii image");
  XtVaSetValues(imageFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,500,
#endif
		NULL);

  XtAddCallback(imageFileBox,XmNcancelCallback,Cancel,imageFileBox);
  XtAddCallback(imageFileBox,XmNokCallback,ImageOk,fftsound);
  XtAddCallback(loadimage,XmNactivateCallback,Image,fftsound);


  /* CREATE LOAD BREAKPOINT BUTTON */
  loadbrk=XtVaCreateManagedWidget(
    "loadbrk",xmPushButtonWidgetClass,fileMenu,NULL);
  text=XmStringCreateLocalized("Load control function");
  XtVaSetValues(loadbrk,XmNlabelString,text,NULL);

  brkFileBox=XmCreateFileSelectionDialog(mainWindow,"brkFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(brkFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Load control function");
  XtVaSetValues(brkFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,500,
#endif
		NULL);

  brkRowCol=XtVaCreateManagedWidget("brkRowCol",xmRowColumnWidgetClass,
    brkFileBox,NULL);

  brkRadio1=XtVaCreateManagedWidget("parmT3",
    xmTextFieldWidgetClass,brkRowCol,NULL);
  XtVaSetValues(brkRadio1,XmNwidth,100,XmNvalue,"6",NULL);

  XtAddCallback(brkFileBox,XmNcancelCallback,Cancel,brkFileBox);
  XtAddCallback(brkFileBox,XmNokCallback,LoadBrkOk,fftsound);
  XtAddCallback(loadbrk,XmNactivateCallback,LoadBrk,fftsound);

  /* CREATESAVE BREAKPOINT BUTTON */
  savebrk=XtVaCreateManagedWidget(
    "savebrk",xmPushButtonWidgetClass,fileMenu,NULL);
  text=XmStringCreateLocalized("Save control function");
  XtVaSetValues(savebrk,XmNlabelString,text,NULL);

  brkSaveFileBox=XmCreateFileSelectionDialog(mainWindow,"brkSaveFileBox",NULL,0);
  temp=XmFileSelectionBoxGetChild(brkSaveFileBox,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Save control function");
  XtVaSetValues(brkSaveFileBox,XmNdialogTitle,text,
		XmNwidth,500,
#ifdef USELESSTIF
		XmNheight,500,
#endif
		NULL);

  XtAddCallback(brkSaveFileBox,XmNcancelCallback,Cancel,brkSaveFileBox);
  XtAddCallback(brkSaveFileBox,XmNokCallback,SaveBrkOk,fftsound);
  XtAddCallback(savebrk,XmNactivateCallback,SaveBrk,fftsound);


  /* CREATE PLAY BUTTON */
  play=XtVaCreateManagedWidget(
    "play",
    xmPushButtonWidgetClass,fileMenu,
    XmNmnemonic,XStringToKeysym("P"),XmNaccelerator,"<Key>P",
    XmNacceleratorText,XmStringCreateLocalized("P"),
    NULL
    );
  text=XmStringCreateLocalized("Play");
  XtVaSetValues(play,XmNlabelString,text,NULL);
  XtAddCallback(play,XmNactivateCallback,PlayOk,fftsound);

  /* CREATE STOP BUTTON */
  stop=XtVaCreateManagedWidget(
    "stop",
    xmPushButtonWidgetClass,fileMenu,
    //    XmNmnemonic,XK_space,XmNaccelerator,"<Key>Space",
    XmNacceleratorText,XmStringCreateLocalized("Space"),
    NULL
    );
  text=XmStringCreateLocalized("Stop");
  XtVaSetValues(stop,XmNlabelString,text,NULL);
  XtAddCallback(stop,XmNactivateCallback,StopOk,fftsound);

  /* CREATE QUIT BUTTON */
  quit=XtVaCreateManagedWidget(
    "quit",xmPushButtonWidgetClass,fileMenu,
    XmNmnemonic,XStringToKeysym("Q"),XmNaccelerator,"Alt<Key>Q",
    XmNacceleratorText,XmStringCreateLocalized("Alt+Q"),NULL);
  text=XmStringCreateLocalized("Quit");
  XtVaSetValues(quit,XmNlabelString,text,NULL);
  XtAddCallback(quit,XmNactivateCallback,Quit,fftsound);

  /* CREATE FILE OPENING WARNING */
  fileWarning=XmCreateWarningDialog(mainWindow,"fileWarning",NULL,0);
  temp=XmMessageBoxGetChild(fileWarning,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  temp=XmMessageBoxGetChild(fileWarning,XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Error!");
  XtVaSetValues(fileWarning,XmNdialogTitle,text,
    XmNwidth,400,NULL);
  text=XmStringCreateSimple("Can't open file");
  XtVaSetValues(fileWarning,XmNmessageString,text,NULL);

  /* CREATE FILE WRITE WARNING */
  writeWarning=XmCreateWarningDialog(mainWindow,"writeWarning",NULL,0);
  temp=XmMessageBoxGetChild(writeWarning,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  temp=XmMessageBoxGetChild(writeWarning,XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Error!");
  XtVaSetValues(writeWarning,XmNdialogTitle,text,
    XmNwidth,400,NULL);
  text=XmStringCreateSimple("Can't write to file");
  XtVaSetValues(writeWarning,XmNmessageString,text,NULL);

  /* CREATE FILE SIZE WARNING */
  sizeWarning=XmCreateWarningDialog(mainWindow,"sizeWarning",NULL,0);
  temp=XmMessageBoxGetChild(sizeWarning,XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);
  temp=XmMessageBoxGetChild(sizeWarning,XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);
  text=XmStringCreateSimple("Error!");
  XtVaSetValues(sizeWarning,XmNdialogTitle,text,
    XmNwidth,400,NULL);
  text=XmStringCreateLtoR(
    "Sorry, can't handle soundfiles that large",
    XmFONTLIST_DEFAULT_TAG);
  XtVaSetValues(sizeWarning,XmNmessageString,text,NULL);


}

