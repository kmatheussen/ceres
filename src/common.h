
#ifdef max
#undef max
#endif

#define max(a,b) (((a)>(b))?(a):(b))

#ifdef min
#undef min
#endif

#define min(a,b) (((a)<(b))?(a):(b))

#define FORWARD 1
#define INVERSE 0
#define MAX_FFT_SIZE 4096
#define MAX_PAINT 5000
#define MAXNOD 800

#define fvec(name, size)\
if ((name=(double *)calloc(size, sizeof(double)))==NULL) {\
  printf("Insufficient memory\n");\
}
#define ffvec(name, size)\
if ((name=(float *)calloc(size, sizeof(float)))==NULL) {\
  printf("Insufficient memory\n");\
}
#define LSIZE 512
#define FLUSH_LIST(list, count, showGC, theGC)        \
          XDrawPoints(theDisplay,XtWindow(sketchpad),showGC,list,count, \
	    CoordModeOrigin);       \
          XDrawPoints(theDisplay,bitmap,theGC,list,count,     \
	    CoordModeOrigin);       \
        count=0

#define ADD_POINT(xx, yy, list, count, showGC, theGC) \
      list[count].x = xx;     \
      list[count].y = yy;     \
      if (++(count)==LSIZE) { \
              FLUSH_LIST(list,count,showGC,theGC);    \
      }       

#if 0
#define ADD_POINT2(xx,yy,theGC) \
          XDrawPoint(theDisplay,XtWindow(sketchpad),theGC,xx,yy); \
          XDrawPoint(theDisplay,bitmap,theGC,xx,yy)


#undef ADD_POINT
#define ADD_POINT(xx,yy,list,count,showGC,theGC) ADD_POINT2(xx,yy,theGC)
#undef FLUSH_LIST
#define FLUSH_LIST /* */
#endif

#define Pi 3.141592654
#define TwoPi 6.2831853



