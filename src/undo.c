#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ceres.h"
#include "cachememhandler.h"
#include "play.h"

/*
  Undo handling logic based on code from the program "Radium".
  -Kjetil M.
*/


#define UNDOTRANSFORM 0
#define UNDOANALYSIS 0

struct Undo{
  struct Undo *prev;
  struct Undo *next;
  int type;  
  int num;
};

struct Undo_transform{
  struct Undo undo;
  struct TempFile *megampfile;
  struct TempFile *megfreqfile;
};

struct Undo_analysis{
  struct Undo undo;
  struct TempFile *wanalfile;
  struct TempFile *wsynfile;
};

static struct Undo UndoRoot={0};
static struct Undo *CurrUndo=&UndoRoot;
static int num_undos=0;
static int undonum=0;
static int doundo=2;

static int UNDO_getMaxNumUndos(void){
  if(CONFIG_getBool("unlimited_undo")==true){
    return 0;
  }
  return CONFIG_getInt("max_number_of_undos");
}

void UNDO_cleanup(void){
  while(CurrUndo->next!=NULL){
    struct Undo_transform *ut=(struct Undo_transform*)CurrUndo->next;
    struct Undo *temp=CurrUndo->next->next;

    TF_delete(ut->megfreqfile);
    TF_delete(ut->megampfile);
    free(ut);

    CurrUndo->next=temp;
  }
}



void UNDO_Reset(void){

  CurrUndo=&UndoRoot;
  num_undos=0;

  UNDO_cleanup();
}

int UNDO_getDoUndo(void){
  return doundo;
}

void UNDO_setDoUndo(int dasdoundo){
  doundo=dasdoundo;
}

bool UNDO_allowedToDoUndo(void){
  if(doundo==0 || (doundo==2 && CONFIG_getBool("disable_undo")==true)){
    return false;
  }
  return true;
}

bool UNDO_allowedUndo(void){
  if(CurrUndo==&UndoRoot) return false;
  return true;
}

bool UNDO_allowedRedo(void){
  if(CurrUndo->next==NULL) return false;
  return true;
}

void UNDO_addTransform(struct FFTSound *fftsound){
  struct Undo_transform *undo_trans;
  struct Undo *undo;

  if(doundo==0 || (doundo==2 && CONFIG_getBool("disable_undo")==true)) return;
  if(UNDO_allowedToDoUndo()==false) return;

  undo_trans=calloc(1,sizeof(struct Undo_transform));

  PlayStopHard();

  if(fftsound->usemem==true){
    int len;
    undo_trans->megampfile=TF_new("megamp");
    len=fwrite(
	       fftsound->megamp.dac.s[0].mem,
	       fftsound->total_channels*fftsound->horiz_num,sizeof(float),
	       undo_trans->megampfile->file
	       );
    if(len!=sizeof(float)){
      fprintf(stderr,"Could not make undo\n");
      TF_delete(undo_trans->megampfile);
      free(undo_trans);
      return;
    }
  }else{
    DAC_writeBack(&fftsound->megamp.dac);
    undo_trans->megampfile=TF_makeCopy("megamp",fftsound->megamp.tempfile);
    if(undo_trans->megampfile==NULL){
      fprintf(stderr,"Could not make undo\n");
      free(undo_trans);
      return;
    }
  }

  if(fftsound->usemem==true){
    int len;
    undo_trans->megfreqfile=TF_new("megfreq");
    len=fwrite(
	       fftsound->megfreq.dac.s[0].mem,
	       fftsound->total_channels*fftsound->horiz_num,sizeof(float),
	       undo_trans->megfreqfile->file
	       );
    if(len!=sizeof(float)){
      fprintf(stderr,"Could not make undo\n");
      TF_delete(undo_trans->megfreqfile);
      TF_delete(undo_trans->megampfile);
      free(undo_trans);
      return;
    }
  }else{
    DAC_writeBack(&fftsound->megfreq.dac);
    undo_trans->megfreqfile=TF_makeCopy("megfreq",fftsound->megfreq.tempfile);
    if(undo_trans->megfreqfile==NULL){
      fprintf(stderr,"Could not make undo\n");
      TF_delete(undo_trans->megampfile);
      free(undo_trans);
      return;
    }
  }

  undo=&undo_trans->undo;

  undo->prev=CurrUndo;

  while(CurrUndo->next!=NULL){
    struct Undo_transform *ut=(struct Undo_transform*)CurrUndo->next;
    struct Undo *temp=CurrUndo->next->next;

    TF_delete(ut->megfreqfile);
    TF_delete(ut->megampfile);
    free(ut);

    CurrUndo->next=temp;
  }

  CurrUndo->next=undo;
  CurrUndo=undo;

  undo->type=UNDOTRANSFORM;
  undo->num=undonum;

  num_undos++;
  undonum++;

  while(num_undos!=0 && num_undos>UNDO_getMaxNumUndos()){
    struct Undo_transform *ut=(struct Undo_transform*)UndoRoot.next;
    struct Undo *temp=UndoRoot.next->next;

    TF_delete(ut->megfreqfile);
    TF_delete(ut->megampfile);
    free(ut);

    num_undos--;
    UndoRoot.next=temp;
    UndoRoot.next->prev=&UndoRoot;
  }

  EDIT_setUndoRedoMenues();
}


static void UNDO_doInternal(struct FFTSound *fftsound){
  struct Undo *undo;
  struct Undo_transform *ut;

  undo=CurrUndo;
  ut=(struct Undo_transform*)undo;

  if(fftsound->usemem==true){
    int len;
    struct TempFile *temp;

    temp=TF_new("megfreq");
    len=fwrite(
	       fftsound->megfreq.dac.s[0].mem,
	       fftsound->total_channels*fftsound->horiz_num,sizeof(float),
	       temp->file
	       );
    if(len!=sizeof(float)){
      fprintf(stderr,"Problem making redo\n");
    }

    fseek(ut->megfreqfile->file,0,SEEK_SET);
    fread(
	  fftsound->megfreq.dac.s[0].mem,
	  fftsound->total_channels*fftsound->horiz_num,sizeof(float),
	  ut->megfreqfile->file
	  );
    TF_delete(ut->megfreqfile);
    ut->megfreqfile=temp;

    temp=TF_new("megamp");
    len=fwrite(
	       fftsound->megamp.dac.s[0].mem,
	       fftsound->total_channels*fftsound->horiz_num,sizeof(float),
	       temp->file
	       );
    if(len!=sizeof(float)){
      fprintf(stderr,"Problem making redo\n");
    }

    fseek(ut->megampfile->file,0,SEEK_SET);
    fread(
	  fftsound->megamp.dac.s[0].mem,
	  fftsound->total_channels*fftsound->horiz_num,sizeof(float),
	  ut->megampfile->file
	  );
    TF_delete(ut->megampfile);
    ut->megampfile=temp;


  }else{
    struct TempFile *temp;

    pthread_mutex_lock(&fftsound->mutex);

    DAC_writeBack(&fftsound->megfreq.dac);
    DAC_writeBack(&fftsound->megamp.dac);

    temp=fftsound->megfreq.tempfile;
    fftsound->megfreq.tempfile=ut->megfreqfile;
    ut->megfreqfile=temp;
    
    temp=fftsound->megamp.tempfile;
    fftsound->megamp.tempfile=ut->megampfile;
    ut->megampfile=temp;
    
    CH_clear(fftsound);
    DA_setupCacheMem(&fftsound->megfreq,false);
    DA_setupCacheMem(&fftsound->megamp,false);

    pthread_mutex_unlock(&fftsound->mutex);
  }

  CurrUndo=undo->prev;
  num_undos--;

}

void UNDO_do(struct FFTSound *fftsound){
  if(UNDO_allowedUndo()==false) return;


  UNDO_doInternal(fftsound);

  //  RedrawAll(fftsound);
  
  RedrawWin(fftsound);

  //ResetCursor(topLevel);

  EDIT_setUndoRedoMenues();  

}

void UNDO_redo(struct FFTSound *fftsound){


  if(UNDO_allowedRedo()==false) return;
  
  CurrUndo=CurrUndo->next;
  UNDO_doInternal(fftsound);
  CurrUndo=CurrUndo->next;
  
  num_undos+=2;
  
  RedrawWin(fftsound);
  
  //ResetCursor(topLevel);
  
  EDIT_setUndoRedoMenues();
}


void createUndo(struct FFTSound *fftsound){
  //  fftsound_here=fftsound;
  //  mainpid=getpid();
  //  atexit(UNDO_cleanup);
}

