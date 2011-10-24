#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct { double re; double im; } complex;
#define CABS(x) hypot(x.re, x.im)

complex zero={0., 0.};
complex one={1., 0.};

#define TwoPi 6.283185308
#define Pi 3.141592654


complex cadd(complex x, complex y)
{
  complex z;

  z.re=x.re+y.re; z.im=x.im+y.im;
  return(z);
}

complex csub(complex x, complex y)
{
  complex z;

  z.re=x.re-y.re; z.im=x.im-y.im;
  return(z);
}

complex cmult(complex x, complex y)
{
  complex z;

  z.re=x.re*y.re-x.im*y.im;
  z.im=x.re*y.im+x.im*y.re;
  return(z);
}

complex scmult(double s, complex x)
{
  complex z;

  z.re=s*x.re; z.im=s*x.im;
  return(z);
}

complex cdiv(complex x, complex y)
{
  complex z;
  double mag, ang;

  mag=CABS(x)/CABS(y);
  if (x.re!=0. && y.re!=0.) ang=atan2(x.im, x.re)-atan2(y.im, y.re);
  else ang=0.;
  z.re=mag*cos(ang); z.im=mag*sin(ang);
  return(z);
}

complex conjg(complex x)
{
  complex y;

  y.re=x.re; y.im=-x.im;
  return(y);
}

static complex my_csqrt(complex x)
{
  complex z;
  double mag, ang;

  mag=sqrt(CABS(x));
  if (x.re!=0.) ang=atan2(x.im, x.re)/2.;
  else ang=0.;
  z.re=mag*cos(ang); z.im=mag*sin(ang);
  return(z);
}

complex laguerre(complex *a, int M, complex x, double eps, int P)
{
  complex dx, x1, b, d, f, g, h, sq, gp, gm, g2, q;
//  complex mh;
  int i, j, npol=0;
  double dxold=0.0, cdx, tiny=1.e-15;

  if (P) { dxold=CABS(x); npol=0; }
  for (i=0; i<200; i++) {
    b=a[M];
    d=zero; f=zero;
    for (j=M-1; j>=0; j--) {
      f=cadd(cmult(x, f), d);
      d=cadd(cmult(x, d), b);
      b=cadd(cmult(x, b), a[j]);
    }
    if (CABS(b)<=tiny) dx=zero;
    else if (CABS(d)<=tiny && CABS(f)<=tiny) {
      q=cdiv(b, a[M]);
      dx.re=pow(CABS(q), 1./M); dx.im=0.;
    } else {
      g=cdiv(d, b);
      g2=cmult(g, g);
      h=csub(g2, scmult(2., cdiv(f, b)));
      sq=my_csqrt(scmult((double)M-1, csub(scmult((double)M, h), g2)));
      gp=cadd(g, sq); gm=csub(g, sq);
      if (CABS(gp)<CABS(gm)) gp=gm;
      q.re=M; q.im=0.;
      dx=cdiv(q, gp);
    }
    x1=csub(x, dx);
    if (x1.re==x.re && x1.im==x.im) return(x);
    x=x1;
    if (P) {
      npol++;
      cdx=CABS(dx);
      if (npol>9 && cdx>=dxold) return(x);
      dxold=cdx;
    } else if (CABS(dx)<=eps*CABS(x)) return(x);
  }
  fprintf(stderr, "laguerre: Didn't converge\n");
  return(x);
}

void findroots(complex *a, complex *r, int M)
{
  complex x, b, c;
  double eps=1.e-6;
  int i, j, jj;
  static complex *ad;

  ad=(complex *)malloc((M+1)*sizeof(complex));

  for (i=0; i<=M; i++) ad[i]=a[i];
  for (j=M; j>0; j--) {
    x=zero;
    x=laguerre(ad, j, x, eps, 0);
    if (fabs(x.im)<=pow(2.*eps, 2.*fabs(x.re))) x.im=0.;
    r[j-1]=x;
    b=ad[j];
    for (jj=j-1; jj>=0; jj--) {
      c=ad[jj];
      ad[jj]=b;
      b=cadd(cmult(x, b), c);
    }
  }
  for (j=0; j<M; j++) r[j]=laguerre(a, M, r[j], eps, 1);
  for (i=0; i<M-1; i++) {
    for (j=i+1; j<M; j++) {
      if (r[j].re<r[i].re) {
        x=r[i];
        r[i]=r[j];
        r[j]=x;
      }
    }
  }
  free(ad);
}

void checkroots(double *coef, int M, int S)
{
  int i, j;
  double x;
  complex *a, *roots;

  a=(complex *)malloc((M+1)*sizeof(complex));
  roots=(complex *)malloc((M+1)*sizeof(complex));
  for (i=0; i<=M; i++) {
    a[M-i].re=coef[i];
    a[i].im=0.;
  }
  findroots(a, roots, M);
  for (i=0; i<M; i++) {
    if ((x=CABS(roots[i]))>1.0) {
      if (S==1) roots[i]=cdiv(one, conjg(roots[i]));
      else if (S==2) {
        roots[i].re/=x; roots[i].im/=x;
      }
    }
  }
  a[0]=csub(zero, roots[0]); a[1]=one;
  for (j=1; j<M; j++) {
    a[j+1]=one;
    for (i=j; i>0; i--) a[i]=csub(a[i-1], cmult(roots[j], a[i]));
    a[0]=csub(zero, cmult(roots[j], a[0]));
  }
  for (i=0; i<M; i++) coef[M-i]=a[i].re;
  free(a);
  free(roots);
}

double lpa(double *x, int N, double *b, int M, int S)
{
  int i, j;
  double s, at, a0, *rx, *rc;

  rx=(double *)malloc((M+2)*sizeof(double));
  rc=(double *)malloc((M+2)*sizeof(double));
  for (i=0; i<=M+1; i++)
    for (rx[i]=j=0; j<N-i; j++)
      rx[i]+=x[j]*x[i+j];
  b[0]=1.;
  b[1]=rc[0]=rx[0]!=0.?-rx[1]/rx[0]:0.;
  for (i=2; i<=M; i++) b[i]=0;
  a0=rx[0]+rx[1]*rc[0];

  for (i=1; i<M; i++) {
    for (s=j=0; j<=i; j++) s+=rx[i-j+1]*b[j];
    rc[i]=a0!=0.?-s/a0:0.;
    for (j=1; j<=(i+1)>>1; j++) {
      at=b[j]+rc[i]*b[i-j+1];
      b[i-j+1]+=rc[i]*b[j];
      b[j]=at;
    }
    b[i+1]=rc[i];
    a0+=rc[i]*s;
    if (a0<0.) fprintf(stderr, "lpa: Singular matrix\n");
  }
  free(rx);
  free(rc);
  if (S>0) checkroots(b, M, S);
  return(a0);
}

double lpamp(double omega, double a0, double *coef, int M)
{
  double wpr, wpi, wr, wi, re, im, temp, lp;
  int i;

  if (a0==0.) return(0.);
  wpr=cos(omega); wpi=sin(omega);
  re=wr=1.; im=wi=0.;
  for (coef++, i=1; i<=M; i++, coef++) {
    wr=(temp=wr)*wpr-wi*wpi;
    wi=wi*wpr+temp*wpi;
    re+=*coef*wr; im+=*coef*wi;
  }
  if (re==0. && im==0.) lp=0.;
  else lp=sqrt(a0/(re*re+im*im));
  return(lp);
}
