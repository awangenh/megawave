/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {frandphase};
 version = {"1.6"};
 author = {"Lionel Moisan"};
 function = {"Phase Randomization of a Fimage"};
 usage = {
   'i'->i_flag  "in order NOT to reinitialize the random seed",
   in->in       "input Fimage",
   out<-out     "output Fimage"
}; 
*/
/*----------------------------------------------------------------------
 v1.2: fmeanvar() replaced by faxpb() (L.Moisan)
 v1.3: upgrade faxpb() call (L.Moisan)
 v1.4: upgrade fvar() call (L.Moisan)
 v1.5: added processus number term in the random seed initialization (LM)
 v1.6 (04/2007): fixed non square size bug (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "mw.h"

#ifdef __STDC__
#include <stdlib.h>
#else
extern double  drand48();
extern void    srand48();
extern time_t  time();
#endif

extern void   fft2d();
extern float  fmean(), fvar();
extern void   faxpb();

#define SQR(x) ((x)*(x))


/* NB: Calling this module with out=in is possible */

void frandphase(in,out,i_flag)
     Fimage in,out;
     char   *i_flag;
{
  Fimage re,im;
  double rho,theta;
  int    x,y,nx,ny,ad,ymax;
  float  m,std;

  /*** Initialize random seed if necessary ***/
  if (!i_flag) srand48( (long int) time (NULL) + (long int) getpid() );

  m = fmean(in);
  std = fvar(in,1,1);

  re = mw_new_fimage();
  im = mw_new_fimage();

  /*** FFT ***/
  fft2d(in,NULL,re,im,0);
  nx = re->ncol;
  ny = re->nrow;
  out = mw_change_fimage(out,ny,nx);

  /*** phase randomization ***/
  for (x = -nx/2; x<=0; x++) {
    ymax = ((x==0)||(x==-nx/2))?0:(ny/2-1);
    for (y = -ny/2; y<=ymax; y++) {
      ad = ((y+ny)%ny)*ny + (x+nx)%nx;
      rho = sqrt( (double)( SQR(re->gray[ad]) + SQR(im->gray[ad]) ));
      if ( ((x==0)||(x==-nx/2)) && ((y==0)||(y==-ny/2)) ) {
	re->gray[ad] = (float)rho;
	im->gray[ad] = 0.0;
      } else {
	theta = 2.0*M_PI*drand48();
	re->gray[ad] = (float)( rho*cos( theta ) );
	im->gray[ad] = (float)( rho*sin( theta ) );
	ad = ((-y+ny)%ny)*ny + (-x+nx)%nx;
	re->gray[ad] = (float)( rho*cos( theta ) );
	im->gray[ad] = (float)( -rho*sin( theta ) );
      }
    }
  }
  
  /*** inverse FFT ***/
  fft2d(re,im,out,NULL,1);

  /*** impose mean and variance ***/
  faxpb(out,out,NULL,&std,NULL,&m,NULL,NULL,NULL);

  /*** free memory ***/
  mw_delete_fimage(im);
  mw_delete_fimage(re);
}



