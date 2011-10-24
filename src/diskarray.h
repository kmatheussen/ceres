
#include "tempfile.h"

struct DiskArraySlice{
  int start;
  int end;
  float *mem;
};

struct DiskArrayChannel{
  struct DiskArraySlice s[4];

  int start; // Necesarry for disk read/write
  int end;  //  ---

  int memsize;
  int totalsize;

  int numtwos;

  FILE *file;
  float *mem;

  int num_slices;
};

struct DiskArray{
  int totalsize; // Same as fftsound->total_channels*fftsound->horiz_num

  //  struct DiskArrayChannel **dacs; // num_channels*samps_per_frame elements.
  struct DiskArrayChannel dacs[(4096/2+1)* MAX_SAMPS_PER_FRAME ]; // Max number possible. ("channel" mode). Used by the *_C macros.

  struct DiskArrayChannel dac; // "Point" mode. (default, and generally the fastest)
  

  int numchannels; // Same as fftsound->total_channels

  struct TempFile *tempfile; // The file containing the data. Is not used if fftsound->usemem is true.

  struct FFTSound *fftsound; // Only a pointer to the fftsound struct we belong to.

};

#define DA_initialized(fftsound) ((fftsound)->megamp.dac.s[0].mem!=NULL)

#define DA_memIsHere_C(diskarray,ch,place,slice) \
((diskarray).dacs[ch].s[(slice)].start<=(place) && (diskarray).dacs[ch].s[(slice)].end>(place))

#define DA_memIsHere(diskarray,place,slice) \
((diskarray).dac.s[(slice)].start<=(place) && (diskarray).dac.s[(slice)].end>(place))

#define DAC_memIsHere(dac,place,slice) \
((dac)->s[(slice)].start<=(place) && (dac)->s[(slice)].end>(place))


////////////////////////////////////////

#define DA_getDiskMem(diskarray,place) ( \
DA_memIsHere(diskarray,place,0) \
? \
(diskarray).dac.s[0].mem[(place)-(diskarray).dac.s[0].start] \
: \
DAC_getDiskMemHard(&(diskarray).dac,place) \
)

#define DA_getMem(diskarray,place) ( \
DA_memIsHere(diskarray,place,0) \
? \
(diskarray).dac.s[0].mem[(place)-(diskarray).dac.s[0].start] \
: \
DAC_getMemHard(&(diskarray).dac,place) \
)

#define DA_putMem(diskarray,place,val)  \
do{ \
if( DA_memIsHere(diskarray,place,0) ) \
(diskarray).dac.s[0].mem[(place)-(diskarray).dac.s[0].start]=(val);  \
else \
DAC_putMemHard(&(diskarray).dac,place,val); \
}while(0)

#define DA_putDiskMem(diskarray,place,val)  \
do{ \
if( DA_memIsHere(diskarray,place,0) ) \
(diskarray).dac.s[0].mem[(place)-(diskarray).dac.s[0].start]=(val);  \
else \
DAC_putDiskMemHard(&(diskarray).dac,place,val); \
}while(0)

#define DA_putMemMul(diskarray,place,val)  \
do{ \
if( DA_memIsHere(diskarray,place,0) ) \
(diskarray).dac.s[0].mem[(place)-(diskarray).dac.s[0].start]*=(val);  \
else \
DAC_putMemMulHard(&(diskarray).dac,place,val); \
}while(0)

#define DA_putMemDiv(diskarray,place,val)  \
do{ \
if( DA_memIsHere(diskarray,place,0) ) \
(diskarray).dac.s[0].mem[(place)-(diskarray).dac.s[0].start]/=(val);  \
else \
DAC_putMemDivHard(&(diskarray).dac,place,val); \
       }while(0)

#define DA_putMemAdd(diskarray,place,val)  \
do{ \
if( DA_memIsHere(diskarray,place,0) ) \
(diskarray).dac.s[0].mem[(place)-(diskarray).dac.s[0].start]+=(val);  \
else \
DAC_putMemAddHard(&(diskarray).dac,place,val);  \
       }while(0)


#define DA_putMemSub(diskarray,place,val)  \
do{ \
if( DA_memIsHere(diskarray,place,0) ) \
(diskarray).dac.s[0].mem[(place)-(diskarray).dac.s[0].start]-=(val);  \
else \
DAC_putMemSubHard(&(diskarray).dac,place,val);  \
       }while(0)


#define DA_neg(diskarray,place) \
do{ \
if( DA_memIsHere(diskarray,place,0) ) \
(diskarray).dac.s[0].mem[(place)-(diskarray).dac.s[0].start]=-(diskarray).dac.s[0].mem[(place)-(diskarray).dac.s[0].start];  \
else \
DAC_negHard(&(diskarray).dac,place);  \
       }while(0)



#define DA_copyOne(diskarray,to,from)  \
DA_putMem(diskarray,to,DA_getMem(diskarray,from))

#define DA_copyOneFromDisk(diskarray,to,from)  \
DA_putMem(diskarray,to,DA_getDiskMem(diskarray,from))

#define DA_copyOneToDisk(diskarray,to,from)  \
DA_putDiskMem(diskarray,to,DA_getMem(diskarray,from))


///////////////////////////

#define DA_getMem_C(diskarray,ch,place) ( \
DA_memIsHere_C(diskarray,ch,place,0) \
? \
(diskarray).dacs[ch].s[0].mem[(place)-(diskarray).dacs[ch].s[0].start] \
: \
DAC_getMemHard(&(diskarray).dacs[ch],place) \
)

/* Carefule with this one (if/else stuff). */
#define DA_putMem_C(diskarray,ch,place,val)  \
do{ \
if( DA_memIsHere_C(diskarray,ch,place,0) ) \
(diskarray).dacs[ch].s[0].mem[(place)-(diskarray).dacs[ch].s[0].start]=(val);  \
else \
DAC_putMemHard(&(diskarray).dacs[ch],place,val);  \
       }while(0)


#define DA_putMemMul_C(diskarray,ch,place,val)  \
do{ \
if( DA_memIsHere_C(diskarray,ch,place,0) ) \
(diskarray).dacs[ch].s[0].mem[(place)-(diskarray).dacs[ch].s[0].start]*=(val);  \
else \
DAC_putMemMulHard(&(diskarray).dacs[ch],place,val);  \
       }while(0)


#define DA_putMemDiv_C(diskarray,ch,place,val)  \
do{ \
if( DA_memIsHere_C(diskarray,ch,place,0) ) \
(diskarray).dacs[ch].s[0].mem[(place)-(diskarray).dacs[ch].s[0].start]/=(val);  \
else \
DAC_putMemDivHard(&(diskarray).dacs[ch],place,val);  \
       }while(0)


#define DA_putMemAdd_C(diskarray,ch,place,val)  \
do{ \
if( DA_memIsHere_C(diskarray,ch,place,0) ) \
(diskarray).dacs[ch].s[0].mem[(place)-(diskarray).dacs[ch].s[0].start]+=(val);  \
else \
DAC_putMemAddHard(&(diskarray).dacs[ch],place,val);  \
       }while(0)


#define DA_putMemSub_C(diskarray,ch,place,val)  \
do{ \
if( DA_memIsHere_C(diskarray,ch,place,0) ) \
(diskarray).dacs[ch].s[0].mem[(place)-(diskarray).dacs[ch].s[0].start]-=(val);  \
else \
DAC_putMemSubHard(&(diskarray).dacs[ch],place,val);  \
       }while(0)


#define DA_copyOne_C(diskarray,toch,to,fromch,from)  \
DA_putMem_C(diskarray,toch,to,DA_getMem_C(diskarray,fromch,from))


struct FFTSound;

void DA_closeFile(struct FFTSound *fftsound);
void DA_newFile(struct FFTSound *fftsound);

void DA_convertToChannel(struct FFTSound *fftsound);
void DA_convertToPoint(struct FFTSound *fftsound);

void DAC_new(
	     struct FFTSound *fftsound,
	     struct DiskArrayChannel *dac,
	     FILE *file,
	     int start,
	     int end,
	     int memsize,
	     int num_slices,
	     bool readfromfile
	     );
void DA_setupCacheMem(
		      struct DiskArray *diskarray,
		      bool channel
		      );

void DAC_writeBack(struct DiskArrayChannel *dac);
float DAC_getMemHard(struct DiskArrayChannel *diskarray,int place);
float DAC_getDiskMemHard(struct DiskArrayChannel *diskarray,int place);
void DAC_putMemHard(struct DiskArrayChannel *diskarray,int place,float val);
void DAC_putDiskMemHard(struct DiskArrayChannel *diskarray,int place,float val);
void DAC_putMemMulHard(struct DiskArrayChannel *diskarray,int place,float val);
void DAC_putMemDivHard(struct DiskArrayChannel *diskarray,int place,float val);
void DAC_putMemAddHard(struct DiskArrayChannel *diskarray,int place,float val);
void DAC_putMemSubHard(struct DiskArrayChannel *diskarray,int place,float val);
void DAC_negHard(struct DiskArrayChannel *dac,int place);
void DA_copy(struct DiskArray *from,struct DiskArray *to,int source,int dest,int len);






