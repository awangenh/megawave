/* ---------------- M E G A W A V E 2  h e a d e r ------------------------- */
/* mwcommand
name = {fencode};
version={"0.0"};
author = {"Jacques Froment"};
function = {"Encode a fimage : return bit rate for lossless compression"};
usage = {
U->U "input fimage",
brate<-fencode "best rate (bit per pixel)"
        };
*/
/* ------------------------------------------------------------------------- */

#include <stdio.h>
#include "mw.h"

/* MegaWave2 modules used */
#ifdef __STDC__

extern void arencode2(int *, long *, int *, long *, int *, Fsignal, int *, 
		      Fimage, double *, Cimage);
extern float fentropy(Fimage);

#else

extern void arencode2();
extern float fentropy();

#endif

double fencode(U)

Fimage U;

{
  int optdef;    /* to set the option */
  double arate;  /* Arithmetic coding rate */
  double parate; /* Predictive arithmetic coding rate */
  float erate;   /* Entropy rate */
  double brate;   /* Best rate */

  mwdebug("Enter fencode. Image U of size (%d,%d)\n",U->nrow,U->ncol);
  erate = fentropy(U);
  mwdebug("Rates in bits per pixel to encode the image :\n");
  mwdebug("Entropy rate  = %.4f\n",erate);

  arencode2(&optdef, NULL, NULL, NULL, NULL, NULL, NULL, 
	    U, &arate, (Cimage) NULL);
  mwdebug("Arithmetic rate  = %.4f\n",arate);

  arencode2(&optdef, NULL, NULL, NULL, &optdef, NULL, NULL, 
	    U, &parate, (Cimage) NULL);
  mwdebug("Predictive Arithmetic rate  = %.4f\n",parate);

  brate = erate;
  if (brate > arate) brate = arate ;
  if (brate > parate) brate = parate;
  return(brate);
}


