
#include "ceres.h"
#include "clip.h"


void CLIP_paste(
		struct Clip *clip,
		struct FFTSound *fftsound,
		int to_area_start,int to_area_end
		)
{
  long point;
  int i, j, ch;
  int real_area_end=min(to_area_end,clip->area_end);

  rewind(clip->megampfile->file);
  rewind(clip->megfreqfile->file);

  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=to_area_start*fftsound->numchannels + ch*fftsound->numchannels*fftsound->horiz_num;
    for (i=to_area_start; i<real_area_end; i++) {
      for (j=clip->area_y1; j<clip->area_y2; j++){
	float temp;
	if(fread(&temp,1,sizeof(float),clip->megampfile->file)!=sizeof(float)){
	  fprintf(stderr,"Could not read from file\n");
	  return;
	}
	MEGAMP_PUT(point+j,temp);

	if(fread(&temp,1,sizeof(float),clip->megfreqfile->file)!=sizeof(float)){
	  fprintf(stderr,"Could not read from file\n");
	  return;
	}
	MEGFREQ_PUT(point+j,temp);

      }
      point+=fftsound->numchannels;

//      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }

  /*
 end:
  return;
  */

}


void CLIP_delete(struct Clip *clip){
  if(clip->megampfile!=NULL) TF_delete(clip->megampfile);
  if(clip->megfreqfile!=NULL) TF_delete(clip->megfreqfile);
  free(clip);
  return;
}


struct Clip *CLIP_new(
		      struct FFTSound *fftsound,
		      int area_start,int area_end,
		      int area_y1,int area_y2
		      )
{
  struct Clip *clip;
  long point;
  int i, j, ch;


  clip=calloc(1,sizeof(struct Clip));
  if(clip==NULL) return NULL;

  clip->area_start=area_start;
  clip->area_end=area_end;
  clip->area_y1=area_y1;
  clip->area_y2=area_y2;

  clip->megampfile=TF_new("clip_megamp");
  if(clip->megampfile==NULL){
    fprintf(stderr,"Could not make copy. Probably out of disk-space or illegal temporary directory.\n");
    goto failed;
  }

  clip->megfreqfile=TF_new("clip_megfreq");
  if(clip->megfreqfile==NULL){
    fprintf(stderr,"Could not make copy. Probably out of disk-space.\n");
    goto failed;
  }


  for (ch=0; ch<fftsound->samps_per_frame; ch++) {
    point=area_start*fftsound->numchannels + ch*fftsound->numchannels*fftsound->horiz_num;
    for (i=area_start; i<area_end; i++) {
      for (j=areaf1; j<areaf2; j++){
	float temp;
	temp=MEGAMP_GET(point+j);
	if(fwrite(&temp,1,sizeof(float),clip->megampfile->file)!=sizeof(float)){
	  fprintf(stderr,"Could not make copy. Probably out of disk-space\n");
	  goto failed;
	}
	temp=MEGFREQ_GET(point+j);
	if(fwrite(&temp,1,sizeof(float),clip->megfreqfile->file)!=sizeof(float)){
	  fprintf(stderr,"Could not make copy. Probably out of disk-space\n");
	  goto failed;
	}
      }
      point+=fftsound->numchannels;

//      if(GUI_UpdateProgress(ctransform,area_start,i,area_end)==false) goto end;
    }
  }

  /*
 end:
  return;
  */

  return clip;

failed:
  if(clip->megampfile!=NULL) TF_delete(clip->megampfile);
  if(clip->megfreqfile!=NULL) TF_delete(clip->megfreqfile);
  free(clip);
  return NULL;
}
