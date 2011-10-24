
#include "ceres.h"
#include "cachememhandler.h"


void DAC_new(
	     struct FFTSound *fftsound,
	     struct DiskArrayChannel *dac,
	     FILE *file,
	     int start,
	     int end,
	     int memsize,
	     int num_slices,
	     bool readfromfile
	     )
{
  int lokke;

  //struct DiskArrayChannel *dac=&to->dacs[lokke2];

  dac->memsize=memsize/num_slices;
  dac->num_slices=num_slices;

  dac->numtwos=0;

  dac->start=start;
  dac->end=end;
  dac->totalsize=dac->end - dac->start;

  dac->file=file;

  for(lokke=0;lokke<4;lokke++){
    if(lokke>=num_slices){
      dac->s[lokke].end=0;
    }else{
      if(lokke*dac->memsize>=dac->totalsize){
	printf("Break in the function DAC_New: not supposed to happen!\n");
	break;
      }
      dac->s[lokke].start=lokke*dac->memsize;
      dac->s[lokke].end=min(dac->s[lokke].start+dac->memsize,dac->totalsize);

      if(readfromfile==false){
	dac->s[lokke].mem=CH_calloc(fftsound,dac->memsize);
      }else{
	dac->s[lokke].mem=CH_malloc(fftsound,dac->memsize);
	fseek(dac->file,(dac->start+dac->s[lokke].start)*sizeof(float),SEEK_SET);
	fread(
	      dac->s[lokke].mem,
	      sizeof(float),
	      dac->s[lokke].end - dac->s[lokke].start,
	      dac->file
	      );
      }
    }
  }
}

void DAC_writeBack(struct DiskArrayChannel *dac){
  int lokke;

  for(lokke=0;lokke<dac->num_slices;lokke++){
    if(dac->s[lokke].end>0){
      fseek(dac->file,(dac->s[lokke].start+dac->start)*sizeof(float),SEEK_SET);
      //      printf("writing here %x %d\n",dac->file,dac->s[lokke].start+dac->start);
      fwrite(
	     dac->s[lokke].mem,
	     sizeof(float),
	     dac->s[lokke].end - dac->s[lokke].start,
	     dac->file
	     );
    }
  }
  
}

static void DAC_swapDisk(struct DiskArrayChannel *dac,int place){
  int lokke;

  /* Writing back memory to file. */
  fseek(dac->file,(dac->s[0].start+dac->start)*sizeof(float),SEEK_SET);
  fwrite(
	 dac->s[0].mem,
	 sizeof(float),
	 dac->s[0].end - dac->s[0].start,
	 dac->file
	 );
  /* Setting up new start/endposition for memory array. Using 10% overlap. */

  if(place<dac->s[1].start){
    dac->s[0].start=place - (dac->memsize*9/10);
  }else{
    dac->s[0].start=place - (dac->memsize/10);
  }
  dac->s[0].end=dac->s[0].start+dac->memsize;

  /*
    Legalize the new start/end value.
    It doesn't matter much if the length is unnecesarry less than memsize.
    (Happens very seldom)
  */

  dac->s[0].start=max(0,dac->s[0].start);
  dac->s[0].end=min(dac->s[0].end,dac->totalsize);

  for(lokke=1;lokke<4;lokke++){
    if( dac->s[lokke].start < dac->s[0].end ){
      if( dac->s[lokke].end > dac->s[0].start ){
	if( dac->s[lokke].end <= place ){
	  dac->s[0].start = dac->s[lokke].end;
	}else{
	  dac->s[0].end   = dac->s[lokke].start;
	}
      }
    }
  }


  /* Reading new slice. */

  fseek(dac->file,(dac->s[0].start+dac->start)*sizeof(float),SEEK_SET);

  //  if(dac->start!=0) printf("d: %d\n",dac->start);

  fread(
	dac->s[0].mem,
	sizeof(float),
	dac->s[0].end - dac->s[0].start,
	dac->file
	);
}

static void DAC_moveSlice(struct DiskArrayChannel *dac,int to,int from){
  dac->s[to].mem=dac->s[from].mem;
  dac->s[to].start=dac->s[from].start;
  dac->s[to].end=dac->s[from].end;
}

static void DAC_swapSlices(struct DiskArrayChannel *dac,int one,int two){
  float *mem=dac->s[one].mem;
  int start=dac->s[one].start;
  int end=dac->s[one].end;

  DAC_moveSlice(dac,one,two);

  dac->s[two].mem=mem;
  dac->s[two].start=start;
  dac->s[two].end=end;
}

static float *DAC_getPointer2(struct DiskArrayChannel *dac,int place){
  float *mem;
  int start;
  int end;

  dac->numtwos=0;

  if(DAC_memIsHere(dac,place,2)){
    mem=dac->s[2].mem;
    start=dac->s[2].start;
    end=dac->s[2].end;

    DAC_moveSlice(dac,2,1);
    DAC_moveSlice(dac,1,0);

    dac->s[0].mem=mem;
    dac->s[0].start=start;
    dac->s[0].end=end;

  }else{
    mem=dac->s[3].mem;
    start=dac->s[3].start;
    end=dac->s[3].end;
    
    DAC_moveSlice(dac,3,2);
    DAC_moveSlice(dac,2,1);
    DAC_moveSlice(dac,1,0);
    
    dac->s[0].mem=mem;
    dac->s[0].start=start;
    dac->s[0].end=end;
    
    if( ! DAC_memIsHere(dac,place,0)){
      DAC_swapDisk(dac,place);
    }
  }
  return &(dac->s[0].mem[place-dac->s[0].start]);  
}

static __inline float *DAC_getPointer(struct DiskArrayChannel *dac,int place){

  if(DAC_memIsHere(dac,place,1)){
    dac->numtwos++;
    if(dac->numtwos==20){ // Time to let the second slice be the primary.
      DAC_swapSlices(dac,0,1);
      dac->numtwos=0;
      return &(dac->s[0].mem[place-dac->s[0].start]);
    }
    return &(dac->s[1].mem[place-dac->s[1].start]);
  }

  // This should not happen that often, hopefully the function will not be inlined.
  return DAC_getPointer2(dac,place);

}



float DAC_getMemHard(struct DiskArrayChannel *dac,int place){
    return *(DAC_getPointer(dac,place));
}

float DAC_getDiskMemHard(struct DiskArrayChannel *dac,int place){
  float ret;
  if(
     DAC_memIsHere(dac,place,1)
     || DAC_memIsHere(dac,place,2)
     || DAC_memIsHere(dac,place,3)
     )
    {
      return DAC_getMemHard(dac,place);
    }

  fseek(dac->file,(place+dac->start)*sizeof(float),SEEK_SET);
  fread(
	&ret,
	sizeof(float),
	1,
	dac->file
	);
  return ret;
}

void DAC_putMemHard(struct DiskArrayChannel *dac,int place,float val){
    *(DAC_getPointer(dac,place))=val;
}

void DAC_putDiskMemHard(struct DiskArrayChannel *dac,int place,float val){
  if(
     DAC_memIsHere(dac,place,1)
     || DAC_memIsHere(dac,place,2)
     || DAC_memIsHere(dac,place,3)
     )
    {
      DAC_putMemHard(dac,place,val);
    }

  fseek(dac->file,(place+dac->start)*sizeof(float),SEEK_SET);
  fwrite(
	&val,
	sizeof(float),
	1,
	dac->file
	);
}

void DAC_putMemMulHard(struct DiskArrayChannel *dac,int place,float val){
    *(DAC_getPointer(dac,place))*=val;
}

void DAC_putMemDivHard(struct DiskArrayChannel *dac,int place,float val){
    *(DAC_getPointer(dac,place))/=val;
}

void DAC_putMemAddHard(struct DiskArrayChannel *dac,int place,float val){
    *(DAC_getPointer(dac,place))+=val;
}

void DAC_putMemSubHard(struct DiskArrayChannel *dac,int place,float val){
    *(DAC_getPointer(dac,place))-=val;
}

void DAC_negHard(struct DiskArrayChannel *dac,int place){
  float *p=DAC_getPointer(dac,place);
  *(p)=-*(p);
}
