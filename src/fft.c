
#include "ceres.h"
#include "fft.h"

/*
void flush_all_underflows_to_zero()
  {
    union fpc_csr f;
    f.fc_word = get_fpc_csr();
    f.fc_struct.flush = 1;
    set_fpc_csr(f.fc_word);
  }
*/

/*
void init_again(void)
{
  int i, j;

  for (i=0; i<4; i++) for (j=0; j<8192; j++) output[i][j]=0.;
  for (i=0; i<4; i++) for (j=0; j<4097; j++) {
    lastphase[i][j]=0.; lastfreq[i][j]=0.; lastamp[i][j]=0.; indx[i][j]=0.;
  }
}
*/

/* FFT ROUTINES */

void cfft(double x[], int NC, Boolean forward);
void bitreverse(double x[], int N);


/* If forward is true, rfft replaces 2*N real data points in x with
   N complex values representing the positive frequency half of their
   Fourier spectrum, with x[1] replaced with the real part of the Nyquist
   frequency value.  If forward is false, rfft expects x to contain a
   positive frequency spectrum arranged as before, and replaces it with
   2*N real values.  N MUST be a power of 2. */

void rfft(double x[], int N, Boolean forward)
{
  double 	c1,c2,
  		h1r,h1i,
		h2r,h2i,
		wr,wi,
		wpr,wpi,
  		temp,
		theta;
  double 	xr,xi;
  int 		i,
		i1,i2,i3,i4,
		N2p1;

    theta = Pi/N;
    wr = 1.;
    wi = 0.;
    c1 = 0.5;
    if ( forward ) {
	c2 = -0.5;
	cfft( x, N, forward );
	xr = x[0];
	xi = x[1];
    } else {
	c2 = 0.5;
	theta = -theta;
	xr = x[1];
	xi = 0.;
	x[1] = 0.;
    }
    wpr = -2.*pow( sin( 0.5*theta ), 2. );
    wpi = sin( theta );
    N2p1 = (N<<1) + 1;
    for ( i = 0; i <= N>>1; i++ ) {
	i1 = i<<1;
	i2 = i1 + 1;
	i3 = N2p1 - i2;
	i4 = i3 + 1;
	if ( i == 0 ) {
	    h1r =  c1*(x[i1] + xr );
	    h1i =  c1*(x[i2] - xi );
	    h2r = -c2*(x[i2] + xi );
	    h2i =  c2*(x[i1] - xr );
	    x[i1] =  h1r + wr*h2r - wi*h2i;
	    x[i2] =  h1i + wr*h2i + wi*h2r;
	    xr =  h1r - wr*h2r + wi*h2i;
	    xi = -h1i + wr*h2i + wi*h2r;
	} else {
	    h1r =  c1*(x[i1] + x[i3] );
	    h1i =  c1*(x[i2] - x[i4] );
	    h2r = -c2*(x[i2] + x[i4] );
	    h2i =  c2*(x[i1] - x[i3] );
	    x[i1] =  h1r + wr*h2r - wi*h2i;
	    x[i2] =  h1i + wr*h2i + wi*h2r;
	    x[i3] =  h1r - wr*h2r + wi*h2i;
	    x[i4] = -h1i + wr*h2i + wi*h2r;
	}
	wr = (temp = wr)*wpr - wi*wpi + wr;
	wi = wi*wpr + temp*wpi + wi;
    }
    if ( forward )
	x[1] = xr;
    else
	cfft( x, N, forward );
}

/* cfft replaces double array x containing NC complex values
   (2*NC double values alternating real, imagininary, etc.)
   by its Fourier transform if forward is true, or by its
   inverse Fourier transform if forward is false, using a
   recursive Fast Fourier transform method due to Danielson
   and Lanczos.  NC MUST be a power of 2. */

void cfft(double x[], int NC, Boolean forward)
{
  double 	wr,wi,
		wpr,wpi,
		theta,
		scale;
  int 		mmax,
		ND,
		m,
		i,j,
		delta;

    ND = NC<<1;
    bitreverse( x, ND );
    for ( mmax = 2; mmax < ND; mmax = delta ) {
	delta = mmax<<1;
	theta = TwoPi/( forward? mmax : -mmax );
	wpr = -2.*pow( sin( 0.5*theta ), 2. );
	wpi = sin( theta );
	wr = 1.;
	wi = 0.;
	for ( m = 0; m < mmax; m += 2 ) {
	  double rtemp, itemp;
	    for ( i = m; i < ND; i += delta ) {
		j = i + mmax;
		rtemp = wr*x[j] - wi*x[j+1];
		itemp = wr*x[j+1] + wi*x[j];
		x[j] = x[i] - rtemp;
		x[j+1] = x[i+1] - itemp;
		x[i] += rtemp;
		x[i+1] += itemp;
	    }
	    wr = (rtemp = wr)*wpr - wi*wpi + wr;
	    wi = wi*wpr + rtemp*wpi + wi;
	}
    }

/* scale output */

    scale = forward ? 1./ND : 2.;
    for (i=0; i<ND; i++) x[i]*=scale;
}

/* bitreverse places double array x containing N/2 complex values
   into bit-reversed order */

void bitreverse(double x[], int N)
{
  double 	rtemp,itemp;
  int 		i,j,
		m;

    for ( i = j = 0; i < N; i += 2, j += m ) {
	if ( j > i ) {
	    rtemp = x[j]; itemp = x[j+1]; /* complex exchange */
	    x[j] = x[i]; x[j+1] = x[i+1];
	    x[i] = rtemp; x[i+1] = itemp;
	}
	for ( m = N>>1; m >= 2 && j >= m; m >>= 1 )
	    j -= m;
    }
}

