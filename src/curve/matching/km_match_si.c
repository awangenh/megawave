/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {km_match_si};
version = {"1.0"};
author = {"Jose-Luis Lisani, Pablo Muse, Frederic Sur"};
function = {"Compute similitude-invariant matches between pieces of meaningful boundaries of two images"};
usage = {
   maxError1->maxError1 
        "maximum Hausdorff distance between normalized codes allowed",
   maxError2->maxError2 
        "maximum Hausdorff distance (in pixels) between de-normalized codes (to the frame of image 2) allowed",
   minLength->minLength 
        "minimum arclength (in pixels) a matching piece of curve must have to be considered as valid",
   minComplex->minComplex 
        "minimum angle variation (in rad.) a matching piece of curve must have to be considered a valid",
   levlines1->levlines1 
        "meaningful boundaries of image 1",
   levlines2->levlines2 
        "meaningful boundaries of image 2",
   dict1->dict1 
        "dictionary of similitude-invariant codes of image 1",
   dict2->dict2 
        "dictionary of similitude-invariant codes of image 2",
   matchings<-matchings 
        "list containing the indices of the matching codes",
   matching_pieces<-matching_pieces
        "matchings information: index_curve1_in_levlines1, index_curve2_in_levlines2, index_matching_begins_in_curve1, index_matching_ends_in_curve1, index_matching_begins_in_curve2, index_matching_ends_in_curve2, performance"
      };
*/

#include<math.h>
#include<time.h>
#include<float.h>
#include "mw.h" 

extern Flists km_prematchings();

#define FABSF(x) ((float)fabs((double)(x)))

#define qnorm(a,b) ((a)*(a)+(b)*(b))

#define _(a,i,j) ((a)->values[(i)*((a)->dim)+(j)])

struct NormDataSIconcat { 
  int Numcurve_in_llconcat;
  int Numimage;
  int Numcurve; 
  int i_left, i_right; 
  float xC, yC; 
  int iL1, iL2; 
  float xR1, yR1, xR2, yR2; 
  float disc; 
}; 
 
struct matchdata { 
  int NumCurve1, NumCurve2; 
  int ini1, fin1, ini2, fin2; 
  float extended_length1, extended_length2; 
  float performance;
}; 

static char Closed;
static int N_Points;

static float M_SI[2][3]; 


/* Computes the similitude-transform between two frames */

void getMatrixSI(xini1, yini1, xfin1, yfin1, xini2, yini2, xfin2, yfin2, A)
     float xini1, yini1, xfin1, yfin1, xini2, yini2, xfin2, yfin2;
     float A[2][3];
{
  float X1,Y1,X2,Y2,L1sq;

  X1 = (xfin1-xini1);
  Y1 = (yfin1-yini1);
  X2 = (xfin2-xini2);
  Y2 = (yfin2-yini2);

  L1sq = qnorm(X1,Y1);

  A[0][0] = (X2*X1 + Y2*Y1)/L1sq;
  A[0][1] = (X2*Y1 - X1*Y2)/L1sq;
  A[1][0] = -A[0][1];
  A[1][1] = A[0][0];

  A[0][2] = (xini2*qnorm(xfin1,yfin1) + xfin2*qnorm(xini1,yini1) 
	     - (xini2 + xfin2)*(xini1*xfin1 + yini1*yfin1)
	     + Y2*(xfin1*yini1 - xini1*yfin1))/L1sq;
  
  A[1][2] = (yini2*qnorm(yfin1,xfin1) + yfin2*qnorm(yini1,xini1) 
	     - (yini2 + yfin2)*(yini1*yfin1 + xini1*xfin1)
	     + X2*(yfin1*xini1 - yini1*xfin1))/L1sq;

  return;
}

/* transforms the coordinates of a point from 
   one local frame in image 1 to another local frame in image 2. 
   M_SI is a global variable whose computation is done below */

void map1to2_SI(x, y)
     float *x, *y;
{
  float xN, yN;
  
  xN=M_SI[0][0]*(*x)+M_SI[0][1]*(*y)+M_SI[0][2];
  yN=M_SI[1][0]*(*x)+M_SI[1][1]*(*y)+M_SI[1][2];
  
  *x=xN; *y=yN;
}

/******** auxiliairy functions for prematching extension **********/


/* Given a point index, gets the index of the 
   next (type=1) or previous(type=0) point in the curve */

static int get_next_index(i, iLast, type)
     int i, iLast;
     unsigned char type;
{
  int i_next;
  
  if (type == 1) {
    if ((Closed) && (iLast == 0)) iLast=N_Points-1;
  } else {
    if ((Closed) && (iLast == N_Points-1)) iLast=0;
  }
  
  if (type == 1) {
    i_next=i+1;
    if ((i_next > N_Points-1) && (!Closed)) return -1;
    if ((i_next > N_Points-1) && (Closed)) i_next=1;
  } else {
    i_next=i-1;
    if ((i_next < 0) && (!Closed)) return -1;
    if ((i_next < 0) && (Closed)) i_next=N_Points-2;
  }
  if (i_next == iLast) return -1;
  
  return i_next;
}


/* computes arclength of curve FCRV between indices i1 and i2 */

static float arc_length(fcrv, i1, i2)
     Flist fcrv;
     int i1, i2;
{
  int i;
  float x1, y1, x2, y2, length;
  
  x1 = _(fcrv, i1, 0);
  y1 = _(fcrv, i1, 1);

  length=0;
  if (i2 > i1){ 
    for (i=i1+1; i <= i2; i++) {
      x2 = _(fcrv, i, 0);
      y2 = _(fcrv, i, 1);
      length+=(float) sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
      x1=x2;
      y1=y2;
    }
  } else {
    for (i=i1+1; i < (int) fcrv->size; i++) { 
      x2 = _(fcrv, i, 0);
      y2 = _(fcrv, i, 1);
      length+=(float) sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
      x1=x2;
      y1=y2;
    }
    for (i=0; i <= i2; i++) {
      x2 = _(fcrv, i, 0);
      y2 = _(fcrv, i, 1);
      length+=(float) sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
      x1=x2;
      y1=y2;
    }
  }
  
  return length;
}

/* Given a point (xIO,yIO) of FCRV and its closest index IFIRST (in the
   TYPE direction), computes the point of fcrv at arclength D in the TYPE
   direction.  If this point is farther than the ILAST index, the
   function returns -1. Otherwise, it stores in (xIO,yIO) the searched 
   point and returns its index */

static int get_next_point_length(fcrv, xI0, yI0, iFirst, iLast, d, type)
     Flist fcrv;
     float *xI0, *yI0;
     int iFirst, iLast;
     float d;
     unsigned char type;
{
  int i, i_last;
  float x, y, xP, yP, s, t;
  float splust;
  
  s=0.0f;
  
  i_last=get_next_index(iLast, iLast, type);
  xP=*xI0;
  yP=*yI0;
  i=iFirst;
  do {
    x=_(fcrv,i,0);
    y=_(fcrv,i,1);
    t=(float)hypot((double)(x-xP),(double)(y-yP));
    splust=s+t; 
    if (splust < d) {
      s=splust;
      xP=x;
      yP=y;
      i=get_next_index(i, i_last, type);
      if (i < 0) return -1;
    }
  } while (splust < d);
  *xI0=xP+(d-s)*(x-xP)/t;
  *yI0=yP+(d-s)*(y-yP)/t;
  
  return i;
}


/* This function performs the extension of a couple of "pre-matched"
   (see MW module KM_PREMATCHINGS) pieces of curves.  FCRV1 and FCRV2 are
   two curves, the first one in image 1 and the other in image 2.  Let S1
   (in FCRV1) and S2 (in FCRV2) be the "pre-matched" sub-curves (their
   encoding information has been stored in INFO1 and INFO2).  The
   extension is done in the "TYPE" direction. It begins at S1 and S2's
   "central points" : (info1->xC,info1->yC) for FCRV1 and
   (info1->xC,info1->yC) for FCRV2; it ends where the Hausdorff distance
   between them (in image2's frame, i.e. measured in pixels) exceeds
   ERRORMAX.  It can also end if FCRV1 reaches index ILAST1 or FCRV2
   reaches index ILAST2.  (this is done in order to stop the extension
   when it has reached the end of an open curve, or to avoid that a
   closed curved is extended more than one tour). The indices
   corresponding to the end of the extension are stored in IL1 (for
   FCRV1) and IL2 (for FCRV2).  */

static void get_last_index_matching(errorMax, info1, info2, fcrv1, fcrv2, iL1, iL2, iLast1, iLast2, type)
     struct NormDataSIconcat *info1;
     struct NormDataSIconcat *info2; 
     Flist fcrv1, fcrv2;
     int *iL1;
     int *iL2; 
     int iLast1, iLast2;
     float errorMax;
     unsigned char type;
{
  float x1, y1, x2, y2, x1p, y1p, ee;
  int i1, i2;
  
  x1=info1->xC;
  y1=info1->yC;
  x2=info2->xC;
  y2=info2->yC;
  x1p=x1; y1p=y1;
  map1to2_SI(&x1p, &y1p); /*central point of sub-curve1 is mapped to image2's frame.*/
  i1=*iL1;
  i2=*iL2;
  ee=(x2-x1p)*(x2-x1p)+(y2-y1p)*(y2-y1p); /*distance between both central points*/
  while (ee < errorMax*errorMax) {
    N_Points=fcrv1->size;
    Closed=((_(fcrv1,0,0)==_(fcrv1,fcrv1->size-1,0))&&(_(fcrv1,0,1)==_(fcrv1,fcrv1->size-1,1)));
    i1=get_next_point_length(fcrv1, &x1, &y1, i1, iLast1, info1->disc, type);
    N_Points=fcrv2->size;
    Closed=((_(fcrv2,0,0)==_(fcrv2,fcrv2->size-1,0))&&(_(fcrv2,0,1)==_(fcrv2,fcrv2->size-1,1)));
    i2=get_next_point_length(fcrv2, &x2, &y2, i2, iLast2, info2->disc, type);
    x1p=x1; y1p=y1;
    map1to2_SI(&x1p, &y1p);     /*map curve1 current point to image2's frame*/
    ee=(x2-x1p)*(x2-x1p)+(y2-y1p)*(y2-y1p); /*distance between corresponding current points*/
    if (ee < errorMax*errorMax) {
      if (i1 >= 0) *iL1=i1;
      if (i2 >= 0) *iL2=i2;
      if ((i1 < 0) || (i2 < 0)) return; /*stop when one of the curves has reached its iLast index*/
    } /*extend while Hausdorff distance between curves is less than errorMax*/ 
  }
}


/* This function performs the extension in both directions using the
   previous function.  If the total extended length in fcrv1 or fcrv2 is
   less than MINLENGTH the matching is not considered a a valid one, and
   the function returns 0. Otherwise, the matching is valid; a matching
   performance value is calculated, matching information is stored in a
   MATCHDATA structure, and function returns 1. */

unsigned char extend_matchingSI(errorMax, minLength, info1, info2, fcrv1, fcrv2, ini1, fin1, ini2, fin2,matchdataaux)
     float errorMax, minLength;
     struct NormDataSIconcat *info1; 
     struct NormDataSIconcat *info2; 
     struct matchdata *matchdataaux;
     Flist fcrv1, fcrv2; 
     int *ini1;
     int *fin1;
     int *ini2;
     int *fin2;
{
  float L1, L2;
  float L1total, L2total;
  
  *ini1=info1->i_left;
  *fin1=info1->i_right;
  *ini2=info2->i_left;
  *fin2=info2->i_right;
  
  get_last_index_matching(errorMax, info1, info2, fcrv1, fcrv2, fin1, fin2, *ini1, *ini2, 1);
  
  get_last_index_matching(errorMax, info1, info2, fcrv1, fcrv2, ini1, ini2, *fin1, *fin2, 0);
  
  L1=arc_length(fcrv1, *ini1, *fin1);
  if (L1 < minLength) {
    return 0;
  }
  
  L2=arc_length(fcrv2, *ini2, *fin2); 
  if (L2 < minLength) {
    return 0;
  }
  
  L1total = arc_length(fcrv1,0,fcrv1->size-1);
  L2total = arc_length(fcrv2,0,fcrv2->size-1);
  
  if (L1>=arc_length(fcrv1,0,fcrv1->size-2)) 
    mwdebug("complete tour in 1 at curve %d (%d) with L = %f, (%f,%f) (%f,%f) \n",info1->Numcurve,info2->Numcurve,L1,info1->xC,info1->yC,info2->xC,info2->yC);
  
  if (L2>=arc_length(fcrv2,0,fcrv2->size-2)) 
    mwdebug("complete tour in 2 at curve %d  (%d) with L = %f, (%f,%f) (%f,%f)\n",info2->Numcurve,info1->Numcurve,L2,info2->xC,info2->yC,info1->xC,info1->yC);
  
  matchdataaux->NumCurve1 = info1->Numcurve_in_llconcat;
  matchdataaux->NumCurve2 = info2->Numcurve_in_llconcat;
  matchdataaux->ini1 = *ini1; 
  matchdataaux->fin1 = *fin1; 
  matchdataaux->ini2 = *ini2; 
  matchdataaux->fin2 = *fin2; 
  matchdataaux->extended_length1 = L1;
  matchdataaux->extended_length2 = L2;
  matchdataaux->performance = L1*L2/(L1total*L2total);
  
  return 1;
}


/***** auxiliary functions for the complexity check of a piece of curve ******/

/* angle between two vectors */
static float angle(u0x,u0y,v0x,v0y)
     float u0x,u0y,v0x,v0y; 
{
  float c,s;

  c=(u0x*v0x+u0y*v0y)/((float)sqrt(qnorm(u0x,u0y)*qnorm(v0x,v0y)));
  s=(u0x*v0y-u0y*v0x)/((float)sqrt(qnorm(u0x,u0y)*qnorm(v0x,v0y)));
  return ((float) atan2(s,c));
}

/* computes the total angle variation of a curve (i.e. its complexity) */
static float get_complexity(fcrv, i1, i2)
     Flist fcrv;
     int i1, i2;
{
  float x1, y1, x2, y2, vx, vy, vxP, vyP, cmpx;
  int i;
  
  N_Points = fcrv->size;
  Closed=((_(fcrv,0,0)==_(fcrv,fcrv->size-1,0))
	  &&(_(fcrv,0,1)==_(fcrv,fcrv->size-1,1)));
  cmpx=0.0f;
  x1 = _(fcrv, i1, 0);
  y1 = _(fcrv, i1, 1);
  i=get_next_index(i1, i2, 1);
  if (i < 0) 
    return cmpx;
  x2 = _(fcrv, i, 0);
  y2 = _(fcrv, i, 1);
  vxP=x2-x1;
  vyP=y2-y1;
  do {
    x1=x2;
    y1=y2;
    i=get_next_index(i, i2, 1);
    if (i < 0)
      return cmpx;
    x2 = _(fcrv, i, 0);
    y2 = _(fcrv, i, 1);
    vx=x2-x1;
    vy=y2-y1;
    cmpx+=FABSF(angle(vxP, vyP, vx, vy));
    vxP=vx;
    vyP=vy;
  } while (1);
  
  return cmpx;
}

/* This function performs the extension of a couple of pre-matched
   curves. If the extended curves are long enough (larger than MINLENGTH)
   and their complexity is larger than MINCOMPLEX, they are considered to
   match and the function returns 1. Otherwise, the matching is no valid
   and the function returns 0.  
   All the matching information is stored in a MATCHDATA structure 
   (see function extend_matchingSI) .*/

unsigned char check_matchingSI(fcrv1, fcrv2, info1, info2, minComplex, errorMax, minLength, A, matchdataaux)
     Flist fcrv1, fcrv2; 
     struct NormDataSIconcat *info1, *info2;
     struct matchdata *matchdataaux;
     float minComplex, errorMax, minLength;
     float A[3][3];
{
  int i1, f1, i2, f2;

  getMatrixSI(info1->xR1, info1->yR1, info1->xR2, info1->yR2,info2->xR1, info2->yR1, info2->xR2, info2->yR2, A);
  
  if (!extend_matchingSI(errorMax, minLength, info1, info2, fcrv1, fcrv2, &i1, &f1, &i2, &f2,matchdataaux)) 
    return 0;
  
  if (get_complexity(fcrv1, i1, f1) < minComplex) return 0;
  
  if (get_complexity(fcrv2, i2, f2) < minComplex) return 0;
  
  return 1;
}


/*------------------------------ MAIN MODULE ------------------------------*/

/* for every similitude-invariant piece of code in dict1, gets its
   pre-matching codes in dict2.  Then matching extension is performed for
   all pre-matchings. Finally, for each piece of curve of image 1 that
   has one or more valid matching curves in image 2, only the one that
   provides the bigger performance value is kept.  Two outputs are
   returned. MATCHING contains all couples (index_in_dict1,
   index_in_dict2). MATCHING_PIECES contains, for each matching, the
   following information, in the following order:
   
   index_curve1_in_levlines1, index_curve2_in_levlines2,
   index_matching_begins_in_curve1, index_matching_ends_in_curve1,
   index_matching_begins_in_curve2, index_matching_ends_in_curve2,
   performance.
  
   This last output is to be used with MW module SAVE_MATCH. The result
   will be two FLISTS that contains all pieces of curves in image1 and
   image2 that match; they can be displayed using FKVIEW.
*/

void km_match_si(maxError1, maxError2,minLength, minComplex, levlines1, levlines2, dict1, dict2, matchings, matching_pieces)
     float maxError1, maxError2;
     float minLength, minComplex;
     Flists levlines1, levlines2, dict1, dict2;
     Flist matchings, matching_pieces; 
{
  struct matchdata *matchdataaux;
  int m,n;
  Flists prematchings;
  
  int t1,t2;
  float performance;
  int code2match;
  
  /*minComplex = 2.7925; */
  
  if(maxError1 < 0.0) 
    mwerror(FATAL,1,"invalid argument type: maxError1 must be non-negative\n");
  if(maxError2 < 0.0) 
    mwerror(FATAL,1,"invalid argument type: maxError2 must be non-negative\n");
  if(minLength < 0.0) 
    mwerror(FATAL,1,"invalid argument type: minLength must be non-negative\n");
  if(levlines1->size == 0) 
    mwerror(FATAL,1,"error: Flists levlines1 is empty\n");
  if(levlines2->size == 0) 
    mwerror(FATAL,1,"error: Flists levlines2 is empty\n");
  if(dict1->size == 0) mwerror(FATAL,1,"error : Flists dict1 is empty\n");
  if(dict2->size == 0) mwerror(FATAL,1,"error : Flists dict2 is empty\n");
  if(dict1->list[0]->size != dict2->list[0]->size)
    mwerror(FATAL,1,"error: codes in dict1 and dict2 don't have the same number of points\n");
  if(dict1->list[0]->data_size != sizeof(struct NormDataSIconcat)) 
    mwerror(FATAL,1,"error: dict1 is not a similitude-invariant dictionary code\n");
  if(dict2->list[0]->data_size != sizeof(struct NormDataSIconcat)) 
    mwerror(FATAL,1,"error: dict2 is not a similitude-invariant dictionary code\n");
  
  matchdataaux = (struct matchdata*)malloc(sizeof(struct matchdata));
  
  t1=clock();
  if ((matchings=mw_change_flist(matchings,dict1->size+1,0,2))==NULL) 
    mwerror (FATAL,1,"error, not enough memory\n");
  if ((matching_pieces=mw_change_flist(matching_pieces,dict1->size+1,0,7))==NULL) mwerror (FATAL,1,"error, not enough memory\n");
  
  prematchings = mw_new_flists();
  km_prematchings(maxError1,dict1,dict2,prematchings); /*compute pre-matchings from both dictionaries*/
  
  for (m=0;m<prematchings->size;m++) {
    performance = -1.0;
    for (n=0;n<prematchings->list[m]->size;n++) {
      
      if(check_matchingSI(levlines1->list[((struct NormDataSIconcat*)(dict1->list[m]->data))->Numcurve],
			  levlines2->list[((struct NormDataSIconcat*)(dict2->list[(int)_(prematchings->list[m],n,0)]->data))->Numcurve],
			  (struct NormDataSIconcat*)(dict1->list[m]->data), 
			  (struct NormDataSIconcat*)(dict2->list[(int)_(prematchings->list[m],n,0)]->data), 
			  minComplex,maxError2, minLength, M_SI, matchdataaux)){	  
	if(matchdataaux->performance > performance){
	  performance = matchdataaux->performance;
	  code2match = (int)_(prematchings->list[m],n,0);
	}
      } 
      
    }
    
    if (performance > -1.0) {
      _(matchings,matchings->size,0)=m; 
      _(matchings,matchings->size++,1) = code2match;
      _(matching_pieces,matching_pieces->size,0)=matchdataaux->NumCurve1; 
      _(matching_pieces,matching_pieces->size,1)=matchdataaux->NumCurve2; 
      _(matching_pieces,matching_pieces->size,2)=matchdataaux->ini1;
      _(matching_pieces,matching_pieces->size,3)=matchdataaux->fin1;
      _(matching_pieces,matching_pieces->size,4)=matchdataaux->ini2;
      _(matching_pieces,matching_pieces->size,5)=matchdataaux->fin2;
      _(matching_pieces,matching_pieces->size++,6)=matchdataaux->performance;
    }
  } 
  
  mw_delete_flists(prematchings);
  t2=clock();
  mw_realloc_flist(matchings,matchings->size);
  mw_realloc_flist(matching_pieces,matching_pieces->size);

  mwdebug("nb. of matchings : %d\n", matchings->size);
  mwdebug("time : %f\n",(float)(t2-t1)/CLOCKS_PER_SEC);
}
