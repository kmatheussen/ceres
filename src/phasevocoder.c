
#include <limits.h>

#include "ceres.h"
#include "phasevocoder.h"
#include "fft.h"
#include "rsynthdata.h"
#include "config.h"

/* PHASE VOCODER INTERNALS */

/*
 * make balanced pair of analysis (A) and synthesis (S) windows;
 * window lengths are Nw, FFT length is N, synthesis interpolation
 * factor is I
 */
void makewindows(double A[], double S[], int Nw,int N,int I)
{
 int i ;
 double sum ;


  if (CONFIG_getBool("Hamming")==true)
    for ( i = 0 ; i < Nw ; i++ )
      A[i] = S[i] = 0.54 - 0.46*cos( TwoPi*i/(Nw - 1) ) ;
  else if (CONFIG_getBool("Hanning")==true)
    for ( i = 0 ; i < Nw ; i++ )
      A[i] = S[i] = 0.5 - 0.5*cos( TwoPi*i/(Nw - 1) ) ;
  if (CONFIG_getBool("Rectangular")==true)
    for ( i = 0 ; i < Nw ; i++ )
      A[i] = S[i] = 1. ;
  else if (CONFIG_getBool("Bartlett")==true)
    for ( i = 0 ; i < Nw ; i++ )
      A[i] = S[i] = 1.-fabs(i-(Nw-1.)/2.)/((Nw-1.)/2.);


/*
 * when Nw > N, also apply interpolating (sinc) windows to
 * ensure that window are 0 at increments of N (the FFT length)
 * away from the center of the analysis window and of I away
 * from the center of the synthesis window
 */
    if ( Nw > N ) {
     double x ;

/*
 * take care to create symmetrical windows
 */
	x = -(Nw - 1)/2. ;
	for ( i = 0 ; i < Nw ; i++, x += 1. )
	    if ( x != 0. ) {
		A[i] *= N*sin( Pi*x/N )/(Pi*x) ;
		S[i] *= I*sin( Pi*x/I )/(Pi*x) ;
	    }
    }
/*
 * normalize windows for unity gain across unmodified
 * analysis-synthesis procedure
 */
    for ( sum = i = 0 ; i < Nw ; i++ )
	sum += A[i] ;

    for ( i = 0 ; i < Nw ; i++ ) {
     double afac = 2./sum ;
     double sfac = Nw > N ? 1./afac : afac ;
	A[i] *= afac ;
	S[i] *= sfac ;
    }

    if ( Nw <= N) {
	for ( sum = i = 0 ; i < Nw ; i += I )
	    sum += S[i]*S[i] ;
	for ( sum = 1./sum, i = 0 ; i < Nw ; i++ )
	    S[i] *= sum ;
    }
}

int shiftin(
	    struct FFTSound *fftsound,
	    int (*waveprovidor)(struct FFTSound *fftsound,void *pointer,double *samples,int num_samples,int ch),
	    void *pointer,
	    double A[],int N, int D,double *max,int ch
	    )
{
  int i;
  double *Aptr = A, mmax=0.;
  
  
  if (si_valid<0) si_valid=N;
  i = N-D;
  while (i--) {
    *Aptr = *(Aptr+D);
    Aptr++;
  }
  

  if (si_valid==N) {
    int sampsread=(*waveprovidor)(fftsound,pointer,Aptr,D,ch);
    si_valid = sampsread+N-D;

    while(sampsread--){
      if (fabs(*Aptr)>mmax) mmax=fabs(*Aptr);
      Aptr++;
    }
  }
  
  if (si_valid<N) {
    for (i=si_valid; i<N; i++) A[i]=0.;
    si_valid-=D;
  }

  *max=mmax;
  return (si_valid<=0);
}


/*
 * multiply current input I by window W (both of length Nw);
 * using modulus arithmetic, fold and rotate windowed input
 * into output array O of (FFT) length N according to current
 * input time n
 */
void fold(double I[],double W[],int Nw,double O[],int N,int n)
{
 
    int i;

    for ( i = 0; i < N; i++ )
	O[i] = 0.;

    while ( n < 0 )
	n += N;
    n %= N;
    for ( i = 0; i < Nw; i++ ) {
	O[n] += I[i]*W[i];
	if ( ++n == N )
	    n = 0;
    }
}


/* S is a spectrum in rfft format, i.e., it contains N real values
   arranged as real followed by imaginary values, except for first
   two values, which are real parts of 0 and Nyquist frequencies;
   convert first changes these into N/2+1 PAIRS of magnitude and
   phase values to be stored in output array C; the phases are then
   unwrapped and successive phase differences are used to compute
   estimates of the instantaneous frequencies for each phase vocoder
   analysis channel; decimation rate D and sampling rate R are used
   to render these frequency values directly in Hz. */

void convert(
	     struct RSynthData *rsd,
	     double S[], double C[],
	     int N2, int D, int R, int ch
	     )
{
  static int 	first = 1;
  static double fundamental,
		factor;
  double 	phase,
		phasediff;
  int 		real,
		imag,
		amp,
		freq;
  double 	a,
		b;
  int 		i;

/* first pass: allocate zeroed space for previous phase
   values for each channel and compute constants */

    if ( first ) {
      first = 0;
      fundamental = (double) R/(N2<<1);
      factor = R/(D*TwoPi);
    } 

/* unravel rfft-format spectrum: note that N2+1 pairs of
   values are produced */

    for ( i = 0; i <= N2; i++ ) {
      imag = freq = ( real = amp = i<<1 ) + 1;
      a = ( i == N2 ? S[1] : S[real] );
      b = ( i == 0 || i == N2 ? 0. : S[imag] );

/* compute magnitude value from real and imaginary parts */

      C[amp] = hypot( a, b );

/* compute phase value from real and imaginary parts and take
   difference between this and previous value for each channel */

      if ( C[amp] == 0. ){
	phasediff = 0.;
      }else {
	phasediff = ( phase = -atan2( b, a ) ) - rsd->lastphase[ch][i];
	rsd->lastphase[ch][i] = phase;
	
/* unwrap phase differences */

	while ( phasediff > Pi )
	  phasediff -= TwoPi;
	while ( phasediff < -Pi )
	  phasediff += TwoPi;
      }

/* convert each phase difference to Hz */

      C[freq] = phasediff*factor + i*fundamental;
    }
}

/* oscillator bank resynthesizer for phase vocoder analyzer
  uses sum of N+1 cosinusoidal table lookup oscillators to 
  compute I (interpolation factor) samples of output O
  from N+1 amplitude and frequency value-pairs in C;
  frequencies are scaled by P */

#define MAXL (INT_MAX/2)
#define L 8192
#define L2 8192.0f

#if 1

/* This version is nearly 3 times faster. (38s / 105s) */
void oscbank(
	     struct FFTSound *fftsound,
	     struct RSynthData *rsd,
	     double C[],
	     int N,int R,int I,
	     double O[],
	     int ch,int Nw,
	     double coef[],
	     int Np,
	     double a0
	     )
{
  //  static int 	L = 8192,
  static int	first = 1;
  static double	*table;
  int 		amp, freq, n, chan;
  double	Iinv;

  int address1=0;
  int address2=0;

/* first pass: allocate memory for and compute cosine table */

  if ( first ) {
    first = 0;
    fvec(table,L);

    for ( n = 0; n < L; n++ ){
      table[n] =  (double)(N*cos( TwoPi*(double)n/L));
    }
  }

  Iinv=1./I;

/* for each channel, compute I samples using linear
   interpolation on the amplitude and frequency
   control values */

  for ( chan = 0; chan < fftsound->NP; chan++ ) {

    double	 	a,
      ainc,
      f,
      finc,
      address;
    
    freq = ( amp = ( chan << 1 ) ) + 1;
    C[freq] *= fftsound->Pinc;
    finc = ( C[freq] - ( f = rsd->lastfreq[ch][chan] ) )*Iinv;
    if (C[amp]<fftsound->synt){
      C[amp] = 0.;
    }else{
      if (Np){
	C[amp]*=lpamp(chan*fftsound->P*Pi/N, a0, coef, Np)/
	  lpamp(chan*Pi/N, a0, coef, Np);
      }
    }
    ainc = ( C[amp] - ( a = rsd->lastamp[ch][chan] ) )*Iinv;
    address = rsd->indx[ch][chan];

/* accumulate the I samples from each oscillator into
   output array O (initially assumed to be zero);
   f is frequency in Hz scaled by oscillator increment
   factor and pitch (Pinc); a is amplitude; */

    address1=(int)address;
    address2=(int)((address-(float)address1)*MAXL);

    if (ainc!=0. || a !=0.){
      int f1=(int)f;
      int f2=(int)(((f-(float)f1)*MAXL));
      int finc1=(int)finc;
      int finc2=(int)((finc-(float)finc1)*MAXL);

      for ( n = 0; n < I; n++ ) {
	O[n] += a*table[address1];
	//	O[n] += a*table[ (int) address ];

	address1+=f1;
	address2+=f2;
	
	if(address2>=MAXL){
	  address1++;
	  address2-=MAXL;
	}else{
	  if(address2<0){
	    address1--;
	    address2+=MAXL;
	  }
	}
	
	//	address += f;
	
	while ( address1 >= L )
	  address1 -= L;
	
	while ( address1 < 0 )
	  address1 += L;
	
	a += ainc;

	f1+=finc1;
	f2+=finc2;
	if(f2>=MAXL){
	  f1++;
	  f2-=MAXL;
	}else{
	  if(f2<0){
	    f1--;
	    f2+=MAXL;
	  }
	}

	//	a += ainc;
	//	f += finc;
      }
    }
    
    /* save current values for next iteration */
    rsd->lastfreq[ch][chan] = C[freq];
    rsd->lastamp[ch][chan] = C[amp];
    rsd->indx[ch][chan] = (float)address1+(address2/(float)MAXL);
    //    indx[chan] = (float)address1+(address2/(float)MAXL);

  } //end  for ( chan = 0; chan < fftsound->NP; chan++ ) {

}

#else 

void oscbank(
	     struct FFTSound *fftsound,
	     struct RSynthData *rsd,
	     double C[],
	     int N,int R,int I,
	     double O[],
	     int ch,int Nw,
	     double coef[],
	     int Np,
	     double a0
	     )
{

  //  static int 	L = 8192,
  static int first = 1;
  static double	*table;
  int 		amp, freq, n, chan;
  double	Iinv;

/* first pass: allocate memory for and compute cosine table */

    if ( first ) {
	first = 0;
	fvec(table,L);

	for ( n = 0; n < L; n++ )
	    table[n] = N*cos( TwoPi*(double)n/L);
    }

  Iinv=1./I;

/* for each channel, compute I samples using linear
   interpolation on the amplitude and frequency
   control values */

    for ( chan = 0; chan < fftsound->NP; chan++ ) {

      double	 	a,
			ainc,
			f,
			finc,
			address;

	freq = ( amp = ( chan << 1 ) ) + 1;
	C[freq] *= fftsound->Pinc;
	finc = ( C[freq] - ( f = rsd->lastfreq[ch][chan] ) )*Iinv;
	if (C[amp]<fftsound->synt) C[amp] = 0.;
        else if (Np) C[amp]*=lpamp(chan*fftsound->P*Pi/fftsound->N, a0, coef, Np)/
          lpamp(chan*Pi/N, a0, coef, Np);
	ainc = ( C[amp] - ( a = rsd->lastamp[ch][chan] ) )*Iinv;
	address = rsd->indx[ch][chan];

/* accumulate the I samples from each oscillator into
   output array O (initially assumed to be zero);
   f is frequency in Hz scaled by oscillator increment
   factor and pitch (Pinc); a is amplitude; */

	if (ainc!=0. || a !=0.) for ( n = 0; n < I; n++ ) {
	  //	    O[n] += a*table[ (int) address ];
	  O[n] += a * (N*cos( TwoPi*address/L));

	    address += f;

	    while ( address >= L )
		address -= L;

	    while ( address < 0 )
		address += L;

	    a += ainc;
	    f += finc;
	}

/* save current values for next iteration */
	rsd->lastfreq[ch][chan] = C[freq];
	rsd->lastamp[ch][chan] = C[amp];
	rsd->indx[ch][chan] = address;
    }
}

#endif

#undef MAXL
#undef L
#undef L2


/* unconvert essentially undoes what convert does, i.e., it
  turns N2+1 PAIRS of amplitude and frequency values in
  C into N2 PAIR of complex spectrum data (in rfft format)
  in output array S; sampling rate R and interpolation factor
  I are used to recompute phase values from frequencies */

void unconvert(
	       struct RSynthData *rsd,
	       double C[], double S[],
	       int N2, int I, int R, int ch
	       )
{
  static int 	first = 1;
  static double fundamental,
		factor;
  int 		i,
		real,
		imag,
		amp,
		freq;
  double 	mag,
		phase;

/* first pass: allocate memory and compute constants */

    if ( first ) {
	first = 0;
	fundamental = (double) R/(N2<<1);
        factor = TwoPi*I/R;
    } 

/* subtract out frequencies associated with each channel,
   compute phases in terms of radians per I samples, and
   convert to complex form */

    for ( i = 0; i <= N2; i++ ) {
	imag = freq = ( real = amp = i<<1 ) + 1;
	if ( i == N2 )
	    real = 1;
	mag = C[amp];
	rsd->lastphase[ch][i] += C[freq] - i*fundamental;
	phase = rsd->lastphase[ch][i]*factor;
	S[real] = mag*cos( phase );
	if ( i != N2 )
	    S[imag] = -mag*sin( phase );
    }
}


/*
 * input I is a folded spectrum of length N; output O and
 * synthesis window W are of length Nw--overlap-add windowed,
 * unrotated, unfolded input data into output O
 */
void overlapadd(double I[],int N,double W[],double O[],int Nw,int n)
{
 int i ;
    while ( n < 0 )
	n += N ;
    n %= N ;
    for ( i = 0 ; i < Nw ; i++ ) {
	O[i] += I[n]*W[i] ;
	if ( ++n == N )
	    n = 0 ;
    }
}


void shiftout(
	      struct FFTSound *fftsound,
	      void(*waveconsumer)(struct FFTSound *fftsound,void *pointer,double **samples,int num_samples),
	      void *pointer,
	      double **output,
	      int N,int I,int n
	      )
{
    int i,ch;

    if (n>=0) {
      (*waveconsumer)(
		      fftsound,
		      pointer,
		      output,
		      I
		      );
    }

    for (ch=0; ch<fftsound->samps_per_frame; ch++) {
      for (i=0; i<N-I; i++) output[ch][i]=output[ch][i+I];
      for (i=N-I; i<N; i++) output[ch][i]=0.;
    }
}




void rightbins(struct FFTSound *fftsound)
{
   double nyquist;
   long point=0;
   int i,  j, thebin;
   double tempamp[fftsound->numchannels];
   double tempfreq[fftsound->numchannels];

   memset(tempfreq,0.0,sizeof(double)*fftsound->numchannels);

   nyquist=fftsound->R/2.;
   for (i=0; i<fftsound->horiz_num*fftsound->samps_per_frame; i++) {
     memset(tempamp,0.0,sizeof(double)*fftsound->numchannels);
     for (j=0; j<fftsound->numchannels; j++) {
       float freqtemp=MEGFREQ_GET(point+j);
       if (
	   (freqtemp>0.)
	   && (freqtemp<nyquist)
	   )
	 {
	   float amptemp=MEGAMP_GET(point+j);
	   thebin=(int)(0.5+freqtemp/fftsound->binfreq);
	   if (thebin>=fftsound->numchannels) fprintf(stderr,"OOPS: %d\n", thebin);
	   if (amptemp>tempamp[thebin]) {
	     tempamp[thebin]=amptemp;
	     tempfreq[thebin]=freqtemp;
	   }
	 }
     }
     for (j=0; j<fftsound->numchannels; j++) {
       MEGFREQ_PUT(point+j,tempfreq[j]);
       MEGAMP_PUT(point+j,tempamp[j]);
     }
     point+=fftsound->numchannels;
   }
}

void clipfreq(struct FFTSound *fftsound)
{
  double nyquist;
  long point=0;
  int i, j;
  // int thebin;

  nyquist=fftsound->R/2.;

  for (i=0; i<fftsound->horiz_num*fftsound->samps_per_frame; i++) {
    for (j=0; j<fftsound->numchannels; j++) {
      if(
	 (MEGFREQ_GET(point)<0.)
	 || (MEGFREQ_GET(point)>=nyquist)
	 )
	{
	  MEGAMP_PUT(point,0.0);
	}
      point++;
    }
  }
} 



void makeWaves(
	       struct FFTSound *fftsound,
	       struct RSynthData *rsd,
	       void(*waveconsumer)(struct FFTSound *fftsound,void *pointer,double **samples,int num_samples),
	       bool (*progressupdate)(int pr,void *pointer),
	       void *pointer,
	       int start,int end,
	       bool obank,
               bool keep_formants
	       )
{
  double a0=0.0;
  long point=0, point2=0;
  int ch;
  int on=(-fftsound->Nw*fftsound->I)/fftsound->Dn, in=-fftsound->Nw;
  int j,i;
  double coef[fftsound->numcoef2+2];
  int keepform=0;

  if(obank){
    fprintf(stderr,"\n\nWarning! The result of the additiv resynthesis might be buggy. Please listen to the result to check that its okey or use the IFFT type.\n\n");
  }

  //  init_again();
  //  rsd=getRSynthData(fftsound);
  
  memset(coef,0.0,sizeof(double)*(fftsound->numcoef2+2));

  if (keep_formants  && fftsound->numcoef2!=0)
    keepform=1;
  
  for (j=start; j<end; j++) {
    on+=fftsound->I; in+=fftsound->Dn;

    for (ch=0; ch<fftsound->samps_per_frame; ch++) {
      point = 
	ch*fftsound->horiz_num*fftsound->numchannels 
	+ j*fftsound->numchannels;
      point2=ch*lpc_horiz_num*(fftsound->numcoef2+2)+j*(fftsound->numcoef2+2);

      if(fftsound->usemem==false) pthread_mutex_lock(&fftsound->mutex);
      for (i=0; i<fftsound->numchannels; i++) {
	rsd->channel[i+i]=MEGAMP_GET(point+i); //ch*fftsound->horiz_num + i,j
	rsd->channel[i+i+1]=MEGFREQ_GET(point+i);
      }
      if(fftsound->usemem==false) pthread_mutex_unlock(&fftsound->mutex);
      
      //	printf("j2: %d\n",j);
      if (obank==true) {

        if (j==start){
	  for (i=0; i<fftsound->N2+1; i++) {
	    rsd->lastfreq[ch][i]=rsd->channel[i+i+1];
	    rsd->lastamp[ch][i]=rsd->channel[i+i];
	    rsd->indx[ch][i]=0.;
	  }
	}

        if (j>=lpc_horiz_num) keepform=0;

        if (keepform) {
          for (i=0; i<fftsound->numcoef2+2; i++) {
            coef[i]=lpc_coef[point2]; point2++;
          }
          a0=coef[fftsound->numcoef2+1];
        }

        oscbank(
		fftsound,
		rsd,rsd->channel,
		fftsound->N2, fftsound->R, fftsound->I,
		rsd->output[ch], ch, 
		fftsound->Nw,coef, 
		fftsound->numcoef2*keepform,a0
		);

      } else {
        unconvert(rsd, rsd->channel,  rsd->buffer,  fftsound->N2,  fftsound->I,  fftsound->R, ch);
        rfft(rsd->buffer,  fftsound->N2,  INVERSE);
        overlapadd(rsd->buffer,  fftsound->N,  fftsound->Wsyn,  rsd->output[ch],  fftsound->Nw,  on);
      }

    } // end for(ch=0; ch<fftsound->samps_per_frame; ch++)


    if (obank==true){
      shiftout(
	       fftsound,
	       waveconsumer,
	       pointer,
	       rsd->output, 
	       fftsound->Nw,  
	       fftsound->I,
	       in+fftsound->Nw2+fftsound->Dn
	       );
    }else{
      shiftout(
	       fftsound,
	       waveconsumer,
	       pointer,
	       rsd->output,
	       fftsound->Nw,
	       fftsound->I,
	       on
	       );
    }

    if(progressupdate!=NULL){
      if((*progressupdate)(j,pointer)==false)break;
    }

  } // end for(j=start; j<end; j++)

  //  returnRSynthData(rsd);
}

void create_phasevocoder(struct FFTSound *fftsound,int windowsize){
  /* INITIALIZE PHASE VOCODER */

  /* FFT length */
  fftsound->N=windowsize;
  if ((fftsound->N!=512) && (fftsound->N!=1024) && (fftsound->N!=2048) && (fftsound->N!=4096)) {
    printf("Illegal value for N. Selected 1024.\n");
    fftsound->N=1024;
  }

  fftsound->overlap=CONFIG_getInt("overlap_factor");
  if (fftsound->overlap<1)
    fftsound->overlap=1;
  if (fftsound->overlap>32)
    fftsound->overlap=32;

  fftsound->Nw=fftsound->N;    /* Window size */
  fftsound->Dn=fftsound->Nw/fftsound->overlap;  /* Decimation factor */

  fftsound->I=(int)(fftsound->Dn * CONFIG_getDouble("time_stretch_factor")); /* Interpolationi factor */

  fftsound->N2=fftsound->N>>1;
  theheight=fftsound->N2;
  fftsound->P=CONFIG_getDouble("pitch_shift");    /* Oscbank pitch factor */
  fftsound->Pinc = fftsound->P*8192./fftsound->R;

  if (fftsound->P>1.){
    fftsound->NP=fftsound->N2/fftsound->P;
  }else{
    fftsound->NP=fftsound->N2;
  }

  fftsound->synt=CONFIG_getDouble("threshold")/fftsound->N; /* Synthesis threshold */
  fftsound->binfreq = (double)fftsound->R/fftsound->N;

  fftsound->Nw2=fftsound->Nw>>1;
  fftsound->numchannels=fftsound->N/2+1;
  areaf2=fftsound->numchannels;
  fvec(fftsound->Wanal,  fftsound->Nw);
  fvec(fftsound->Wsyn,  fftsound->Nw);

  /*
  fvec(input,  fftsound->Nw);
  fvec(buffer,  fftsound->N);
  fvec(channel,  fftsound->N+2);
  */

  //  init_again();

  //  fvec(tempamp,fftsound->numchannels);



  

}

