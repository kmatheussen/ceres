
#include <pthread.h>


#include "ceres.h"
#include "phasevocoder.h"
#include "play.h"
#include "fftsound.h"
#include "rsynthdata.h"

pthread_t pthread={0};
bool thread_is_joinable=false;
bool dontstop=true;
static bool useadditive=false;

void PlayWaveConsumer(
		      struct FFTSound *fftsound,
		      void *pointer,
		      double **samples,
		      int num_samples
		      )
{
  struct PlayStruct *ps=(struct PlayStruct *)pointer;
  //  printf("num_samples: %x\n",num_samples);
  if(ps->usejack==true){
    JackWritePlay(fftsound,ps->playdata,samples,num_samples);
  }else{
    WritePlay(fftsound,ps->playdata,samples,num_samples);
  }
}

bool PlayProgressUpdate(int j,void *pointer){
  //  struct PlayStruct *ps=(struct PlayStruct *)pointer;

  /*
  static int a=0;


  if(XtAppPending(app_context)&XtIMAlternateInput){
    fprintf(stderr,"jadda %d\n",a++);
  }
  */
  //  XDrawLine(theDisplay,XtWindow(sketchpad),markerGC,j,10,j,10+theheight);

  if(dontstop==false){
    dontstop=true;
    return false;
  }
  return true;
}

void *DoPlay(void *pointer){
  struct DoPlayStruct *doplaystruct=(struct DoPlayStruct *)pointer;
  struct FFTSound *fftsound=doplaystruct->fftsound;
  struct PlayStruct ps;
  struct RSynthData *rsd;


  thread_is_joinable=true;

  if(doplaystruct->usejack==true){
    fprintf(stderr,"Jack support disabled. Sndlib works better with jack than ceres.\n");
  }
  ps.usejack=false;
  //ps.usejack=doplaystruct->usejack;

  if(doplaystruct->usejack==false && InitPlay()==FALSE){
    return NULL;
  }

  dontstop=true;

  ps.fftsound=fftsound;

  rsd=getRSynthData(ps.fftsound);

  if(doplaystruct->usejack==true){
    printf("Open Jack Play.\n");
    ps.playdata=JackOpenPlay(ps.fftsound);
  }else{
    printf("Open Non-Jack Play.\n");
    ps.playdata=OpenPlay(ps.fftsound);
  }

  if(ps.playdata!=NULL){

    makeWaves(
	      ps.fftsound,rsd,
	      PlayWaveConsumer,PlayProgressUpdate,
	      &ps,doplaystruct->start,doplaystruct->end,
	      useadditive,
              doplaystruct->keep_formants
	      );
  
  
    if(ps.usejack==true){
      JackClosePlay(ps.playdata);
    }else{
      ClosePlay(ps.playdata);
    }

  }

  returnRSynthData(rsd);

  if(ps.fftsound->dontfreeme==false)
    freeFFTSound(ps.fftsound);

  free(doplaystruct);

  //thread_is_joinable=false;

  return NULL;
}

void PlayStopHard(void){
  dontstop=false;
  if(thread_is_joinable==true){
    pthread_join(pthread,NULL);
  }
}

void PlayStop(void){
  dontstop=false;
}


/* Call from main process only. */

void Play(struct FFTSound *fftsound,int start,int end){
  struct DoPlayStruct *dps=malloc(sizeof(struct DoPlayStruct));

  if(CONFIG_getBool("use_jack")==true){
#ifndef USEJACK
    fprintf(stderr,"Jack support is not compiled into the program.\n");
    return;
#else
    dps->usejack=true;
#endif
  }else{
    dps->usejack=false;
  }

  dps->keep_formants = CONFIG_getBool("keep_formants");

  useadditive=CONFIG_getBool("pt_additive");

  PlayStopHard();

  dps->fftsound=fftsound;
  dps->start=start;
  dps->end=end;

#if 0
  DoPlay(fftsound);
#else
  if(pthread_create(&pthread,NULL,DoPlay,dps)!=0){
    fprintf(stderr,"Could not make pthread\n");
  }
#endif
}



#if 0
void Play(struct FFTSound *fftsound){
  int sampsread, chnls, t=1, numfram, fill, x;
//  int i;
  ALport audioport;
  ALconfig config;
  AFfilehandle filehandle;
//  AFfilesetup filesetup;
  short int frames[4096];
//  XmString text;
  double time=0.;

  if (playing) { playing=0; return; }
  if (playfile[0]=='\0') return;

  filehandle=AFopenfile(playfile, "r", NULL);
  if (filehandle==NULL) {
    XtManageChild(fileWarning);
    return;
  }

  XSetFunction(theDisplay, markerGC, GXxor);
  XSetForeground(theDisplay, markerGC, gray100.pixel);

  playing=1; xold=-999999;
  chnls=AFgetchannels(filehandle, AF_DEFAULT_TRACK);
  config=ALnewconfig();
  ALsetchannels(config, chnls);
  audioport=ALopenport("Ceres Play", "w", config);

  do {
    sampsread=AFreadframes(filehandle, AF_DEFAULT_TRACK, frames, 1024);
    numfram=sampsread*chnls;
    ALwritesamps(audioport, frames, numfram);
    fill=ALgetfilled(audioport);
    x=(int)((time-(double)fill/R/chnls)*800./duration)+50;
    if (x!=xold) {
      if (x<-10000) x=-10000; if (x>10000) x=10000;
      XDrawLine(theDisplay,XtWindow(sketchpad),markerGC,xold,10,xold,10+theheight);
      XDrawLine(theDisplay,XtWindow(sketchpad),markerGC,x,10,x,10+theheight);
      xold=x;
    }
    time+=1024/(double)R;

    if (XtAppPending(app_context) && ((t++)%3==0))
      XtAppProcessEvent(app_context,XtIMXEvent|XtIMTimer|XtIMAlternateInput);
  } while ((playing) && (sampsread==1024));

  while ((fill=ALgetfilled(audioport))) {
    x=(int)((time-(double)fill/R/chnls)*800./duration)+50;
    if (x!=xold) {
      XDrawLine(theDisplay,XtWindow(sketchpad),markerGC,xold,10,xold,10+theheight);
      XDrawLine(theDisplay,XtWindow(sketchpad),markerGC,x,10,x,10+theheight);
      xold=x; XFlush(theDisplay);
    }
  }
  XDrawLine(theDisplay,XtWindow(sketchpad),markerGC,xold,10,xold,10+theheight);

  ALcloseport(audioport);
  playing=0;
}
#endif

#if 0
static pthread_mutex_t playmutex;

void Play_ObtainMutex(void){

}

void Play_ReleaseMutex(void){
}
#endif

void create_play(struct FFTSound *fftsound){
//  pthread_mutex_init(playmutex,NULL);
}






