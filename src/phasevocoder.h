
void makewindows(double A[], double S[], int Nw,int N,int I);

int shiftin(
	    struct FFTSound *fftsound,
	    int (*waveprovidor)(struct FFTSound *fftsound,void *pointer,double *samples,int num_samples,int ch),
	    void *pointer,
	    double A[],int N, int D,double *max,int ch
	    );

void fold(double I[],double W[],int Nw,double O[],int N,int n);

void convert(
	     struct RSynthData *rsd,
	     double S[], double C[],
	     int N2, int D, int R, int ch
	     );

void unconvert(
	       struct RSynthData *rsd,
	       double C[], double S[],
	       int N2, int I, int R, int ch
	       );

void rightbins(struct FFTSound *fftsound);

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
	     );

void overlapadd(double I[],int N,double W[],double O[],int Nw,int n);

void makeWaves(
	       struct FFTSound *fftsound,
	       struct RSynthData *rsd,
	       void(*waveconsumer)(struct FFTSound *fftsound,void *pointer,double **samples,int num_samples),
	       bool (*progressupdate)(int pr,void *pointer),
	       void *pointer,
	       int start,int end,
	       bool obank,
               bool keep_formants
	       );
