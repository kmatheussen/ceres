
#include "ceres.h"
#include "cachememhandler.h"

#define CLEARSIZE (256*1024)

static struct TempFile *DA_createEmptyFile(struct FFTSound *fftsound){
  int lokke;
  float clear[CLEARSIZE]={0};
  struct TempFile *tf;
  int totalsize=fftsound->total_channels*fftsound->horiz_num;

  tf=TF_new("fftdata");

  if(tf==NULL){
    exit(5);
  }

  SetWatchCursor(topLevel);

  XtVaSetValues(progressScale, XmNvalue, 0, NULL);
  XmUpdateDisplay(topLevel);
  XtVaSetValues(progressScale, XmNvalue, 0, NULL);
  XtManageChild(progressDialog);

  
  for(lokke=0;lokke<totalsize/CLEARSIZE;lokke++){
    if(fwrite(clear,sizeof(float),CLEARSIZE,tf->file)!=CLEARSIZE){
      fprintf(
	      stderr,
	      "Could not make temporary file \"%s\". Probably out of disk space. Exiting.\n",
	      tf->name
	      );
      TF_delete(tf);
      exit(6);
    }

    XtVaSetValues(progressScale, XmNvalue, 100*lokke/(totalsize/CLEARSIZE), NULL);
    XmUpdateDisplay(topLevel);
  }

  if(totalsize%CLEARSIZE>0 && fwrite(clear,sizeof(float),totalsize%CLEARSIZE,tf->file)!=totalsize%CLEARSIZE){
    fprintf(
	    stderr,
	    "Could not make temporary file \"%s\". Probably out of disk space. Exiting.\n",
	    tf->name
	    );
    TF_delete(tf);
    exit(7);
  }

  XtVaSetValues(progressScale, XmNvalue, 100, NULL);
  XmUpdateDisplay(topLevel);

  XtUnmanageChild(progressDialog);
  ResetCursor(topLevel);

  return tf;
}


static void DA_new(
		   struct DiskArray *to,
		   struct FFTSound *fftsound	     
)
{

  //  fftsound->usemem=true;

  to->fftsound=fftsound;

  to->numchannels=fftsound->total_channels;

  to->totalsize=fftsound->horiz_num*to->numchannels;

  to->tempfile=DA_createEmptyFile(fftsound);

  DAC_new(fftsound,&to->dac,to->tempfile->file,0,to->totalsize,CH_getSize(fftsound)/2,4,false);

}

void DA_closeFile(struct FFTSound *fftsound){
  if(fftsound->usemem==true){
    free(fftsound->megfreq.dac.s[0].mem);
    free(fftsound->megamp.dac.s[0].mem);
  }
  TF_cleanup();
  CH_clear(fftsound);  
}


void DA_newFile(struct FFTSound *fftsound){
  if(
     CONFIG_getInt("max_mem_sound_size") >=
     (fftsound->horiz_num*fftsound->total_channels)*sizeof(float)*2
     )
    {

      CH_free(fftsound);
    
      fftsound->usemem=true;

      fftsound->megfreq.dac.s[0].mem=calloc(
				   sizeof(float),
				   fftsound->horiz_num*fftsound->total_channels
				   );
      fftsound->megfreq.dac.s[0].start=0;
      fftsound->megfreq.dac.s[0].end=fftsound->horiz_num*fftsound->total_channels;

      fftsound->megamp.dac.s[0].mem=calloc(
				  sizeof(float),
				  fftsound->horiz_num*fftsound->total_channels
				  );
      fftsound->megamp.dac.s[0].start=0;
      fftsound->megamp.dac.s[0].end=fftsound->horiz_num*fftsound->total_channels;

      return;
    }

  fftsound->usemem=false;

  fftsound->tempfile=DA_createEmptyFile(fftsound);

  DA_new(&fftsound->megfreq,fftsound);
  DA_new(&fftsound->megamp,fftsound);

}


void DA_setupCacheMem(
		      struct DiskArray *diskarray,
		      bool channel
		      )
{
  struct FFTSound *fftsound=diskarray->fftsound;

  if(channel==true){
    int lokke;
    int tosize=CH_getSize(fftsound)/(diskarray->numchannels*2*4);
    for(lokke=0;lokke<diskarray->numchannels;lokke++){
      DAC_new(
	      fftsound,
	      &diskarray->dacs[lokke],
	      diskarray->tempfile->file,
	      lokke*fftsound->horiz_num,
	      (lokke+1)*fftsound->horiz_num,
	      tosize,
	      4,
	      true
	      );
    }
  }else{
    int tosize=CH_getSize(fftsound)/(2*4);
    DAC_new(
	    fftsound,
	    &diskarray->dac,
	    diskarray->tempfile->file,
	    0,
	    diskarray->totalsize,
	    tosize,
	    4,
	    true
	    );
  }
}


/* From "point" mode to "channel" mode if tochannel==true, else from "point" to "channel". */

static void DA_convert(
		       struct DiskArray *diskarray,
		       bool tochannel
		       )
{
  int lokke;
  int pointsize;
  int channelsize;
  int i;
  struct FFTSound *fftsound=diskarray->fftsound;
  struct DiskArrayChannel *dac_point;

  struct TempFile *t1=diskarray->tempfile;
  diskarray->tempfile=fftsound->tempfile;
  fftsound->tempfile=t1;


  /* Set up memory caches for the "channel" file */
  channelsize=CH_getSize(fftsound)/(diskarray->numchannels*2);
  for(lokke=0;lokke<diskarray->numchannels;lokke++){
    DAC_new(
	    fftsound,
	    &diskarray->dacs[lokke],
	    tochannel==true?diskarray->tempfile->file:fftsound->tempfile->file,
	    lokke*fftsound->horiz_num,
	    (lokke+1)*fftsound->horiz_num,
	    channelsize,
	    1,
	    tochannel==true?false:true
	    );
  }


  /* Set up memory cache for the "point" file. */
  pointsize=CH_getSize(fftsound)/(2);
  DAC_new(
	  fftsound,
	  &diskarray->dac,
	  tochannel==true?fftsound->tempfile->file:diskarray->tempfile->file,
	  0,
	  diskarray->totalsize,
	  pointsize,
	  1,
	  tochannel==true?true:false
	  );



  /* Copy data */

  dac_point=&diskarray->dac;


  if(tochannel==true){


    int point=0;

    for(i=0;i<fftsound->horiz_num;i++){
      int j;
      for(j=0;j<fftsound->total_channels;j++){
	float val;
	struct DiskArrayChannel *dac_ch=&diskarray->dacs[j];
	
	if(dac_point->s[0].end > point){
	  val=dac_point->s[0].mem[point - dac_point->s[0].start];
	}else{
	  int si;
	  fseek(dac_point->file,point*sizeof(float),SEEK_SET);
	  si=fread(
		   dac_point->s[0].mem,
		   sizeof(float),
		   dac_point->memsize,
		   dac_point->file
		   );
	  val=dac_point->s[0].mem[0];
	  dac_point->s[0].start=point;
	  dac_point->s[0].end=dac_point->s[0].start+si;
	}
      
	if(dac_ch->s[0].end > i){
	  dac_ch->s[0].mem[ i - dac_ch->s[0].start]=val;
	}else{
	  fseek(dac_ch->file,(dac_ch->s[0].start + dac_ch->start)*sizeof(float),SEEK_SET);
	  fwrite(
		 dac_ch->s[0].mem,
		 sizeof(float),
		 dac_ch->s[0].end - dac_ch->s[0].start,
		 dac_ch->file
		 );
	  
	  dac_ch->s[0].start=i;
	  dac_ch->s[0].end=min(dac_ch->s[0].start + dac_ch->memsize,dac_ch->totalsize);
	  dac_ch->s[0].mem[0]=val;
	}
	
	point++;
      }
    }


    /* Writing back cache for "channel" file */
    for(lokke=0;lokke<fftsound->total_channels;lokke++){
      DAC_writeBack(&diskarray->dacs[lokke]);
    }



  }else{  // else if(tochannel==true){


    int point=0;

    for(i=0;i<fftsound->horiz_num;i++){
      int j;
      for(j=0;j<fftsound->total_channels;j++){
	float val;
	struct DiskArrayChannel *dac_ch=&diskarray->dacs[j];
      
	if(dac_ch->s[0].end > i){
	  val=dac_ch->s[0].mem[ i - dac_ch->s[0].start];
	}else{
	  int si;
	  fseek(dac_ch->file,(i+dac_ch->start)*sizeof(float),SEEK_SET);
	  si=fread(
		dac_ch->s[0].mem,
		sizeof(float),
		dac_ch->s[0].end - dac_ch->s[0].start,
		dac_ch->file
		);
	  dac_ch->s[0].start=i;
	  dac_ch->s[0].end=min(dac_ch->s[0].start+dac_ch->memsize,dac_ch->totalsize);
	  val=dac_ch->s[0].mem[0];
	}

	if(dac_point->s[0].end > point){
	  dac_point->s[0].mem[point - dac_point->s[0].start]=val;
	}else{
	  fseek(dac_point->file,dac_point->s[0].start*sizeof(float),SEEK_SET);
	  fwrite(
		 dac_point->s[0].mem,
		 sizeof(float),
		 dac_point->s[0].end - dac_point->s[0].start,
		 dac_point->file
		 );
	  dac_point->s[0].start=point;
	  dac_point->s[0].end=min(dac_point->s[0].start + dac_point->memsize,dac_point->totalsize);
	  dac_point->s[0].mem[0]=val;
	}
	
	point++;
      }
    }


    /* Writing back cache for "point" file */
    DAC_writeBack(&diskarray->dac);

  } // end if(tochannel==true){


}



static void DA_convertBoth(
			   struct FFTSound *fftsound,
			   bool tochannel
			   )
{

  printf("writing back cache\n");
  /* Writing back cache. */
  if(tochannel==true){
    DAC_writeBack(&fftsound->megfreq.dac);
    DAC_writeBack(&fftsound->megamp.dac);
  }else{
    int lokke;
    for(lokke=0;lokke<fftsound->total_channels;lokke++){
      DAC_writeBack(&fftsound->megfreq.dacs[lokke]);
    }
    for(lokke=0;lokke<fftsound->total_channels;lokke++){
      DAC_writeBack(&fftsound->megamp.dacs[lokke]);
    }
  }

  printf("converting\n");
  /* Converting */
  CH_clear(fftsound);
  DA_convert(&fftsound->megfreq,tochannel);

  CH_clear(fftsound);
  DA_convert(&fftsound->megamp,tochannel);

  printf("settuing up cache\n");
  /* Setting up cache. */
  CH_clear(fftsound);
  DA_setupCacheMem(&fftsound->megfreq,tochannel);
  DA_setupCacheMem(&fftsound->megamp,tochannel);
}


void DA_convertToChannel(struct FFTSound *fftsound){
  if(fftsound->usemem==true) return;
  DA_convertBoth(fftsound,true);
}

void DA_convertToPoint(struct FFTSound *fftsound){
  if(fftsound->usemem==true) return;
  DA_convertBoth(fftsound,false);
}


void DA_copy(struct DiskArray *from,struct DiskArray *to,int source,int dest,int len){
  int lokke;

  for(lokke=0;lokke<len;lokke++){
    DA_putMem(*to,dest+lokke,DA_getMem(*from,source+lokke));
  }
}


