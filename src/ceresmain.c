
/* CERES: A combination of FFT and granular synthesis */

#include <signal.h>

#include "ceres.h"
#include "phasevocoder.h"
#include "scales.h"
#include "config.h"

bool inited=false;

void finish(int sig){
  TF_cleanup_exit();
  exit(0);
}


#if 0
void aisig(int sig){
  printf("bla %d\n",sig);
}


pthread_t pthread2;

void *Doit(void *pointer){
  XEvent event;
  for(;;){
    static int b=0;
    XtAppNextEvent(app_context,&event);
    XtDispatchEvent(&event);
    //        printf("aia ai %d\n",b++);
  }
}
#endif

//int main(int argc, char **argv)
int ceresmain(int windowsize,char *filename)
{

//  int i, j;
  XEvent event;

  // (struct FFTSound was to big for the stack on my RedHat7.2 machine. -Kjetil.)
  struct FFTSound *fftsound=calloc(1,sizeof(struct FFTSound));

  signal(SIGINT,finish);
  //  signal(8,aisig);

  //  memset(&fftsound,0,sizeof(struct FFTSound));

  //printf("Oh boy!\n");

  fftsound->horiz_num=0;
#if 0
  fftsound->megamp=NULL;
  fftsound->megfreq=NULL;
#endif
  fftsound->largamp=NULL;
  fftsound->samps_per_frame=1;
  fftsound->R=44100;
  fftsound->numcoef2=0;

  fftsound->dontfreeme=true;

  fftsound->cachemem.size=CONFIG_getInt("disk_cache_size")/sizeof(float);


  pthread_mutex_init(&fftsound->mutex,NULL);

  printf("Starting Ceres ...\n");

  create_phasevocoder(fftsound,windowsize);


  create_gui(fftsound);

  create_play(fftsound);

  createUndo(fftsound);

  create_drawing(fftsound);

  create_file(fftsound);

  create_edit(fftsound);

  create_transforms(fftsound);

  create_export(fftsound);

  //  create_settings(&fftsound);

  create_paint(fftsound);


  create_extract(fftsound);

  create_graph(fftsound);

  create_lpc2(fftsound);

  create_help(fftsound);

  /*  flush_all_underflows_to_zero(); */
  XtRealizeWidget(topLevel);

  setgreycolors();

  makewindows(fftsound->Wanal,  fftsound->Wsyn,  fftsound->Nw,  fftsound->N,  fftsound->I);

  makemajor();

  setSettings(fftsound);

  RedrawWin( fftsound);



  if (strcmp(filename,"++////bzzzt_nofile")){
    loadana(fftsound,filename);
  }

  /* Calls this as the last one, because it sets up an atexit routine that tries to clean up everything. */
  create_tempfile();

  InitPlay();

  inited=true;

#if 1
  //  XtAppMainLoop(app_context);
  for(;;){
    //static int b=0;
    //        printf("aia ai %d\n",b++);
    XtAppNextEvent(app_context,&event);
    XtDispatchEvent(&event);
  }

#else

  if(pthread_create(&pthread2,NULL,Doit,NULL)!=0){
  }

  //  printf("ak: %d\n",pause());
#endif

  return 0;
}

