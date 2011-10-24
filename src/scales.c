#include "ceres.h"
#include "scales.h"


void makepenta(void)
{
  float val;
  int i;

  val=CONFIG_getDouble("grid_starting_frequency");

  gridfreqs[0]=val; gridfreqs[1]=gridfreqs[0]*1.122;
  gridfreqs[2]=gridfreqs[0]*1.335;
  gridfreqs[3]=gridfreqs[0]*1.498;
  gridfreqs[4]=gridfreqs[0]*1.682;
  for (i=5; i<500; i++) {
    gridfreqs[i]=gridfreqs[i-5]*2.;
    numfreqs=i+1;
    if (gridfreqs[i]>23000) break;
    }
}


void makemajor(void)
{
  float val;
  int i;

  val=CONFIG_getDouble("grid_starting_frequency");

  gridfreqs[0]=val; gridfreqs[1]=gridfreqs[0]*1.122;
  gridfreqs[2]=gridfreqs[0]*1.260;
  gridfreqs[3]=gridfreqs[0]*1.335;
  gridfreqs[4]=gridfreqs[0]*1.498;
  gridfreqs[5]=gridfreqs[0]*1.682;
  gridfreqs[6]=gridfreqs[0]*1.888;
  for (i=7; i<500; i++) {
    gridfreqs[i]=gridfreqs[i-7]*2.;
    numfreqs=i+1;
    if (gridfreqs[i]>23000) break;    
    }
}


void makeminor(void)
{
  float val;
  int i;

  val=CONFIG_getDouble("grid_starting_frequency");

  gridfreqs[0]=val; gridfreqs[1]=gridfreqs[0]*1.122;
  gridfreqs[2]=gridfreqs[0]*1.189;
  gridfreqs[3]=gridfreqs[0]*1.335;
  gridfreqs[4]=gridfreqs[0]*1.498;
  gridfreqs[5]=gridfreqs[0]*1.587;
  gridfreqs[6]=gridfreqs[0]*1.888;
  for (i=7; i<500; i++) {
    gridfreqs[i]=gridfreqs[i-7]*2.;
    numfreqs=i+1;
    if (gridfreqs[i]>23000) break;
    }
}


void makechroma(void)
{
  float val;
  int i;

  val=CONFIG_getDouble("grid_starting_frequency");

  gridfreqs[0]=val; gridfreqs[1]=gridfreqs[0]*1.0594631;
  gridfreqs[2]=gridfreqs[0]*1.1224621;
  gridfreqs[3]=gridfreqs[0]*1.189;
  gridfreqs[4]=gridfreqs[0]*1.260;
  gridfreqs[5]=gridfreqs[0]*1.335;
  gridfreqs[6]=gridfreqs[0]*1.414;
  gridfreqs[7]=gridfreqs[0]*1.498;
  gridfreqs[8]=gridfreqs[0]*1.587;
  gridfreqs[9]=gridfreqs[0]*1.682;
  gridfreqs[10]=gridfreqs[0]*1.782;
  gridfreqs[11]=gridfreqs[0]*1.888;
  for (i=12; i<500; i++) {
    gridfreqs[i]=gridfreqs[i-12]*2;
    numfreqs=i+1;
    if (gridfreqs[i]>23000) break;
    }
}


void makewhole(void)
{
  float val;
  int i;

  val=CONFIG_getDouble("grid_starting_frequency");

  gridfreqs[0]=val; gridfreqs[1]=gridfreqs[0]*1.1224621;
  gridfreqs[2]=gridfreqs[0]*1.260;
  gridfreqs[3]=gridfreqs[0]*1.414;
  gridfreqs[4]=gridfreqs[0]*1.587;
  gridfreqs[5]=gridfreqs[0]*1.782;
  for (i=6; i<500; i++) {
    gridfreqs[i]=gridfreqs[i-6]*2;
    numfreqs=i+1;
    if (gridfreqs[i]>23000) break;
    }
}


void makeharmseries(void)
{
  float val;
  int i;

  val=CONFIG_getDouble("grid_starting_frequency");

  gridfreqs[0]=val;
  numfreqs=1;
  for (i=1; i<500; i++) {
    gridfreqs[i]=gridfreqs[i-1]+val; 
    numfreqs=i+1;
    if (gridfreqs[i]>23000) break;
    }  
}


void makeevenharmseries(void)
{
  float val;
  int i;

  val=CONFIG_getDouble("grid_starting_frequency");

  gridfreqs[0]=val;
  gridfreqs[1]=2*val;
  for (i=2; i<500; i++) {
    gridfreqs[i]=gridfreqs[i-1]+(2*val);
    numfreqs=i+1;
    if (gridfreqs[i]>23000) break;
    }
}


void makeoddharmseries(void)
{
  float val;
  int i;

  val=CONFIG_getDouble("grid_starting_frequency");

  gridfreqs[0]=val;
  for (i=1; i<500; i++) {
    gridfreqs[i]=gridfreqs[i-1]+(2*val);
    numfreqs=i+1;
    if (gridfreqs[i]>23000) break;
    }
}


