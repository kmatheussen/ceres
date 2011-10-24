
#include "motif.h"
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/*********************************************************************
          Gui structures and definitions
 ********************************************************************/


#define MAXSLIDERVAL 100000

struct CSlider{
  Boolean isdouble;
  Boolean islog;
  double logval;
  double minval;
  double maxval;
  int resolution;
  Widget textwidget;
  Widget sliderwidget;
  Widget freqtextwidget;
  Widget togglewidget;
  void *callbackfunc;
  Boolean isfunc;
  struct FFTSound *fftsound;
  int graphnum; // For toggle.
  void *pointer;
};

struct CTransform{
  bool doprogress;
  void (*Transform)(struct CTransform *ctransform,void *pointer,struct FFTSound *fftsound,int area_start,int area_end,int funcstartdiff);
  int lastprogress;
  int lasttime;
  struct FFTSound *fftsound;
  void *pointer;
  Widget parentwidget;
  struct CSlider *funcs[3];
  bool usejack;
};

struct VarInput{
  Widget parentwidget;
  Widget managedwidget;
  int x1;
  int x2;
  int y;
  Widget radio;
  struct FFTSound *fftsound;
  void *pointer;
  struct CSlider *funcs[3];
};

/* Gui */

void GUI_generalMult_pr_horiz(double val,void *pointer);

struct VarInput *GUI_MakeMenuButton(
				    struct FFTSound *fftsound,
				    void *pointer,
				    Widget menu,
				    char *name,
				    int x1,int x2
				    );

void GUI_CloseMenuButtonOkCancelPreview(
			  struct VarInput *sm,
			  void (*func)(struct CTransform *ctransform,void *pointer,struct FFTSound *fftsound,int area_start,int area_end,int funcstartdiff),
			  bool doprogress
			  );

bool GUI_UpdateProgress(struct CTransform *ctransform,int startval,int nowval,int endval);

void GUI_setVarInputRadioStart(struct VarInput *sm,Boolean onoff);
void GUI_setVarInputRadioStop(struct VarInput *sm);

struct CSlider *GUI_MakeInputVarDouble(
		struct VarInput *sm,
		char *vartext,
		double *func,
		//		Widget *textfieldwidget,
		double minval,
		double maxval,
		double dasdefault
		);

struct CSlider *GUI_MakeInputVarDouble_func(
		struct VarInput *sm,
		char *vartext,
		void (*func)(double val,void *pointer),
		//		Widget *textfieldwidget,
		double minval,
		double maxval,
		double dasdefault
		);

struct CSlider *GUI_MakeInputVarLog(
		struct VarInput *sm,
		char *vartext,
		double *func,
		//		Widget *textfieldwidget,
		double minval,
		double maxval,
		double dasdefault
		);

struct CSlider *GUI_MakeInputVarLog_func(
		struct VarInput *sm,
		char *vartext,
		void (*func)(double val,void *pointer),
		//		Widget *textfieldwidget,
		double minval,
		double maxval,
		double dasdefault
		);

struct CSlider *GUI_MakeInputVarInt(
		struct VarInput *sm,
		char *vartext,
		//		Widget *textfieldwidget,
		int *func,
		int minval,
		int maxval,
		int dasdefault
		);

struct CSlider *GUI_MakeInputVarInt_func(
		struct VarInput *sm,
		char *vartext,
		//		Widget *textfieldwidget,
		void (*func)(int val,void *pointer),
		int minval,
		int maxval,
		int dasdefault
		);

void GUI_MakeInputVarToggle_func(
			struct VarInput *sm,
			char *vartext,
			//			Widget *togglewidget,
			void (*func)(bool val,void *pointer),
			bool dasdefault
			);

void GUI_MakeInputVarToggle(
			struct VarInput *sm,
			char *vartext,
			//			Widget *togglewidget,
			bool *func,
			bool dasdefault
			);


void GUI_MakeInputVarToggle_graph(
				  struct VarInput *sm,
				  char *vartext,
				  bool *func,
				  bool dasdefault,
				  int graphnum,
				  float minval,
				  float maxval
				  );

			  
void GUI_MakeInputVarFreq(
		struct VarInput *sm,
		char *vartext,
		//		Widget *textfieldwidget,
		//		void (*func)(double val,void *pointer),
		double *func,
		double minval,
		double maxval,
		double dasdefault
		);

