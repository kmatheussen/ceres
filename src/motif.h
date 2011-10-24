
#include <Xm/Xm.h>

#undef LESSTIF

#if defined(LESSTIF_VERSION)
# define USELESSTIF
#endif

#ifdef USELESSTIF

#include <Xm/MainW.h>
#include <Xm/Display.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/CascadeB.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <Xm/DrawingA.h>
#include <Xm/Scale.h>
#include <Xm/SelectioB.h>
#include <Xm/BulletinB.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <Xm/DialogS.h>

#else


#include <Xm/MainW.h>
#include <Xm/Display.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/CascadeB.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <Xm/DrawingA.h>
#include <Xm/Scale.h>


// #include <Xm/SlideC.h>

#include <Xm/SelectioBP.h>
/*
#include <Xm/DialogS.h>
*/
#include <Xm/SelectioB.h>
#include <Xm/BulletinB.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#endif
