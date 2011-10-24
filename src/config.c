
#include "Python.h"

#include "ceres.h"


/*

Analysys:

-window-size
-window function (Hanning(d)/Rectangular(bad)/Haming(bad)/Bartlett)
-Overlap factor (8)

-LPC poles
-Stabilize poles (slow) (false)

-Min. no. extract nodes: (50

Disk:
disk cache size
temp files path
Max preview length

File:
Save Format (wav/aiff/iff/IRCAM/ceres/etc.) (8b/16b/24b/32b/float/double)
*/



static PyObject *getconffunc;
static PyObject *showconffunc;


PyObject *_wrap_CONFIG_init(PyObject *self, PyObject *args) {
    PyObject *resultobj;

    if(!PyArg_ParseTuple(args,(char *)"OO:CONFIG_init",&getconffunc,&showconffunc)) return NULL;

    Py_INCREF(Py_None);
    resultobj = Py_None;
    return resultobj;
}

bool CONFIG_getBool(char *confname){
  int ret;

  PyObject *arglist;
  PyObject *result;

  arglist=Py_BuildValue("(s)",confname);

  result=PyEval_CallObject(getconffunc,arglist);
  Py_DECREF(arglist);

  if(result==NULL){
    fprintf(stderr,"Error in program. Please report. CONFIG_getBool, result==NULL for %s\n",confname);
    exit(70);
  }

  ret=PyInt_AsLong(result);

    if(result!=NULL){
     Py_DECREF(result);
   }

  return ret==1?true:false;
}

char *CONFIG_getGeneral(char *confname){
  char *ret;
  char *temp;

  PyObject *arglist=Py_BuildValue("(s)",confname);
  PyObject *result=PyEval_CallObject(getconffunc,arglist);
  Py_DECREF(arglist);

  if(result==NULL){
    fprintf(stderr,"Error in program. Please report. CONFIG_getGeneral, result==NULL for %s\n",confname);
    exit(60);
  }

  temp=PyString_AsString(result);
  ret=malloc(strlen(temp)+1);
  sprintf(ret,"%s",temp);
  if(result!=NULL){
    Py_DECREF(result);
  }

  return ret;
}

int CONFIG_getInt(char *confname){
  int ret;
  char *result=CONFIG_getGeneral(confname);

  ret=atoi(result);
  free(result);

  return ret;
}

double CONFIG_getDouble(char *confname){
  double ret;
  char *result=CONFIG_getGeneral(confname);
  ret=atof(result);
  free(result);
  return ret;
}

/* The string returned should be freed with free() after use. */
char *CONFIG_getChar(char *confname){
  return(CONFIG_getGeneral(confname));
}

void CONFIG_show(void){
  PyObject *arglist=Py_BuildValue("()");
  PyObject *result=PyEval_CallObject(showconffunc,arglist);
  Py_DECREF(arglist);
  if(result!=NULL){
    Py_DECREF(result);
  }
}


int CONFIG_getMaxPreviewLength(struct FFTSound *fftsound){
  int seconds=CONFIG_getInt("max_preview_length");
  return (seconds*fftsound->R/fftsound->Dn)+fftsound->overlap;
}



