
#include "transform.h"

static Widget transButton;


void create_transforms(struct FFTSound *fftsound)
{
    XmString text;


    /* CREATE TRANSFORM MENU */
    transButton =
	XtVaCreateManagedWidget("transButton", xmCascadeButtonWidgetClass,
				menuBar, NULL);
    text = XmStringCreateLocalized("Transform");
    XtVaSetValues(transButton, XmNlabelString, text, NULL);
    transMenu = XmCreatePulldownMenu(menuBar, "transMenu", NULL, 0);
    XtVaSetValues(transButton, XmNsubMenuId, transMenu, NULL);


    create_transform_sieve(fftsound);
    create_transform_sshift(fftsound);
    create_transform_shear(fftsound);
    create_transform_blur(fftsound);
    create_transform_comb(fftsound);
    create_transform_mirror(fftsound);
    create_transform_expo(fftsound);
    create_transform_spread(fftsound);
    create_transform_pshift(fftsound);
    create_transform_filter(fftsound);
    create_transform_gain(fftsound);
    create_transform_threshold(fftsound);
    create_transform_noisered(fftsound);
    create_transform_nullphase(fftsound);
    create_transform_trpeaks(fftsound);
    create_transform_ltas(fftsound);
    create_transform_magnet(fftsound);
    create_transform_convolve(fftsound);
    create_transform_warp(fftsound);
    create_transform_sshape(fftsound);

}
