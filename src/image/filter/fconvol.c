/*----------------------Megawave2 Module-----------------------------------*/
/*mwcommand
  name = {fconvol};
  version = {"1.0"};
  author = {"Jacques Froment"};
  function = {"2D-direct convolution of a fimage"};
  usage = {
            in->in "Input fimage",
            filter->filtre "convolution filter (fimage)",
            out<-out "Output fimage"
          };
*/
/*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
 
#include "mw.h"

#define min(a,b)        (((a)>(b)) ? (b) : (a))
#define max(a,b)        (((a)>(b)) ? (a) : (b))

void fconvol(in,filtre,out)
 
Fimage in,filtre,out;

{
 
 int n,m,k,l,dxS,kmax,kmin,lmax,lmin,K2,L2;
 double S;
 float *ptrO,*ptrI,*ptrF;

 out=mw_change_fimage(out,in->nrow,in->ncol);
 if (!out) mwerror(FATAL,1,"not enough memory\n"); 

 K2 = filtre->ncol / 2;
 L2 = filtre->nrow / 2;
 dxS = in->ncol;
 ptrO = out->gray;
 ptrI = in->gray;

 for(m=0;m<in->nrow;m++) 
   for (n=0;n<in->ncol;n++) 
     {
       S = 0.0;
       kmax = min(filtre->ncol-1,n+K2);
       kmin = max(0,n+K2-dxS);
       lmax = min(filtre->nrow-1,m+L2);
       lmin = max(0,m+L2-dxS);

       ptrF = filtre->gray;
       for (l=0;l<=lmax;l++) 
	 for (k=0;k<=kmax;k++) 
	   S += ptrI[dxS*(m-l+L2) + (n-k+K2)] * ptrF[filtre->ncol*l+k];
       *ptrO++ = (float) S;
     }
}

