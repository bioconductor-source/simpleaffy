#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "R.h"
#include "R_ext/Boolean.h"


// T test stuff from Numerical Recipes in C

double log2(double v) {
  return log(v)/log(2);
}


// find the k lowest values in data[]
void partial_sort(double  *data, int N, int k) {
  double v,t;
  int i,j,l,r;
  l=0; r = N-1;
  while(r > l) {
    v = data[r]; i = l -1; j = r;
    for(;;){
      while(data[++i] < v) ;
      while((j > 0) && data[--j] > v) ;
      if( i>= j) break;
      t=data[i]; data[i] = data[j]; data[j] = t;
    }
    t = data[i]; data[i] = data[r]; data[r] = t;
    if(i >= k) r = i-1;
    if(i <= k) l = i+1;
  }
}

void quicksort_i(double  *data, int l, int r) {
  double v,t;
  int i,j;
  if(r > l) {
    v = data[r]; i = l - 1; j = r;
    for(;;){
      while(data[++i] < v) ;
      while((j > 0) && (data[--j] > v)) ;
      if( i>= j) break; 
      t=data[i]; data[i] = data[j]; data[j] = t;
    }
    t = data[i]; data[i] = data[r]; data[r] = t;
    quicksort_i(data,l,i-1);
    quicksort_i(data,i+1,r);
  }
}

void quicksort(double  *data, int N) {
  quicksort_i(data,0,N-1);
}

double median(double *data, int N) {
  int half;
  double r = 0;
  if(N % 2  == 1) {
    half = (N-1)/2;
    quicksort(data,N);
    r = data[half];
  }
  else {
    half = N/2 - 1;
    quicksort(data,N);
    r = data[half];
    half = N/2;
    quicksort(data,N);
    r+=data[half];
    r =r /2;
  }
  return r;
}

//a and b are matrices containg replicate data for each half of the
//test. nx gives the number of columns in the arrays, ny, the number
//of rows.
// returns means in mx, my, fold change in fc, and t-test in tt
// NB median messes with the array!!!

double get_ave(double *a, double nx, Rboolean isLogged,int meth) {
  int    i = 0;
  double r = 0;
  if(meth == 3) { // median
    r = median(a,nx);
  }
  else {
    for(i = 0; i < nx; i++) {
      if(isLogged) {
  if(meth == 1)   r += pow(2,a[i]);
  else            r +=   a[i];
      }
      else {
  if(meth == 1)  r += a[i];
  else           r += log2(a[i]);
      }
    }

    r = r/(double)nx;

    if(isLogged) {
      if(meth == 1) {
  r = log2(r);
      }
    }
  }
  return r;
}

/* Computes the variance of an array given its mean */
/* data the thing to find the variance of */
/* mean the mean of the array */
/* variance of the array */

double variance( double *data, int l, double mean){
    double var = 0;
    double ep  = 0;
    double   n = ( double ) l;
    int      i = 0;
    for(i = 0 ; i < l ; i++ ) {
      double s = data[ i ] - mean;
      ep  += s ;
      var += s * s ;
    }
    var = ( var - ep * ep / n ) / ( n - 1 ) ;
    return var ;
  }

double sqr( double a ) {
  return a * a ;
}



/* Computes the covariance of an array given its mean */
/* data the thing to find the variance of */
/* mean the mean of the array */
/* varience of the array */

double covariance( double *dataa, double *datab, int l, double meana, double meanb){
    double covar = 0;
    double   n = ( double ) l;
    int      i = 0;
    for(i = 0 ; i < l ; i++ ) {
      covar += (dataa[ i ] - meana) * (datab[ i ] - meanb);
    }
    covar = covar / (n-1);
    return covar ;
  }

/** and the standard distribution */
double standard_deviation(double *dataa, double *datab, int l, double meana, double meanb){
  double cov = covariance(dataa,datab,l,meana,meanb);
  double vara= variance(dataa,l,meana);
  double varb= variance(datab,l,meanb);
  double sd = sqrt((vara + varb - 2* cov)/(double)l);
  return sd;
}


double cof[6] = { 76.18009172947146,     -86.5053203032941677,
      24.01409824083091,     -1.231739572450155,
      0.1208650973866179e-2, -0.5395239384953e-5  } ;
/**
 * Computes the ln of the gamma function --
 * "the gamma function is like factorial but for doubles (ish)" (c)C.Miller 2003
 */
double gammln( double xx ) {
  double x, y, tmp, ser ;
  int j;
  y = x = xx ;
  tmp = x + 5.5 ;
  tmp -= ( x + 0.5 ) * log( tmp ) ;
  ser = 1.000000000190015 ;
  for(j = 0 ; j < 6 ; j++ ) {
    ser += cof[ j ] / ++y ;
  }
  return -tmp + log( 2.5066282746310005 * ser / x ) ;
}

double betacf_new( double a, double b, double x )
  {
    double eps=3E-7;
    double fpmin=1E-30;
    int maxIterations = 100;
    int m,m2;
    double aa,c,d,del,h,qab,qam,qap;
    qab=a+b;
    qap=a+1.0;
    qam=a-1.0;
    c=1.0;
    d=1.0-qab*x/qap;
    if(fabs(d) < fpmin) d=fpmin;
    d=1.0/d;
    h=d;
    for(m=1;m<=maxIterations;m++) {
      m2=2*m;
      aa=m*(b-m)*x/((qam+m2)*(a+m2));
      d=1.0+aa*d;
      if(fabs(d) < fpmin) d=fpmin;
      c=1.0+aa/c;
      if(fabs(c) < fpmin) c=fpmin;
      d=1.0/d;
      h *= d*c;
      aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
      d  = 1.0+aa*d;
      if(fabs(d) < fpmin) d=fpmin;
      c = 1.0+aa/c;
      if(fabs(c) < fpmin) c=fpmin;
      d=1.0/d;
      del=d*c;
      h *=del;
      if(fabs(del-1.0) <= eps) break;
    }
    if(m > maxIterations) error("a or b too big or not enough iterations in betacf");
    return h;
  }


/**
 * Returns the incomplete beta function Ix(a,b)
 */
double betai( double a, double b, double x ) {
  double bt ;
  if(x <0.0 || x > 1.0) error("Bad x in batai routine.");
  if( x == 0.0 || x == 1.0 )
    bt = 0.0 ;

  else
    bt = exp( gammln( a + b ) - gammln( a ) - gammln( b ) +
        a * log( x ) + b * log( 1.0 - x ) ) ;

  if( x < ( a + 1.0 ) / ( a + b + 2.0 ) )
    return bt * betacf_new( a, b, x ) / a ;

  else
    return 1.0 - bt * betacf_new( b, a, 1.0 - x ) / b ;
}


double mean(double *a, double nx) {
  int    i = 0;
  double r = 0;
  for(i = 0; i < nx; i++) { r +=   a[i]; }
  r = r/(double)nx;
  return r;
}

/**
 * Compute the student's t-test with unequal variances between arrays a and b
 */
double get_pairwise_t_test( double* a, int na, double* b)  {
    double meana = mean( a, na ) ;
    double meanb = mean( b, na ) ;
    double sd  = standard_deviation(a, b, na, meana,meanb ) ;
    double t = ( meana - meanb ) / sd;
    double df = (double) (na - 1);
    double prob = betai( 0.5 * df, 0.5, df / ( df + t*t ) ) ;
    return prob ;
  }


/**
 * Compute the student's t-test with unequal variances between arrays a and b
 */
double get_t_test( double* a, int na, double* b, int nb )  {
    double meana = mean( a, na ) ;
    double vara = variance( a, na, meana ) ;
    double meanb = mean( b, nb ) ;
    double varb = variance( b, nb, meanb ) ;
    double t = ( meana - meanb ) / sqrt( vara / na + varb / nb ) ;
    double df = sqr( vara / na + varb / nb ) /
                ( sqr( vara / na ) / ( na - 1 ) + sqr( varb / nb ) / ( nb - 1 ) ) ;
    double prob = betai( 0.5 * df, 0.5, df / ( df + sqr( t ) ) ) ;
    return prob ;
  }

void FCMandTT(double *a, double *b, int *nacol, int *nbcol, int *ngene, Rboolean *isLogged, Rboolean *doPairwise, int *meth, double *ma, double *mb, double*fc, double *tt) {
  int idx = 0;
  int gene_idx_a=0;
  int gene_idx_b=0;
  double *tmpa = NULL;
  double *tmpb = NULL;
  int j = 0;
  if(*meth == 3) {
    tmpa = (double *) R_alloc(*nacol,sizeof(double));
    tmpb = (double *) R_alloc(*nbcol,sizeof(double));
  }

  while(gene_idx_a < ((*nacol) * (*ngene))) {
    if(*meth == 3) { // median
      for(j =0; j < *nacol; j++) {
  tmpa[j] = a[gene_idx_a + j];
      }
      for(j =0; j < *nbcol; j++) {
  tmpb[j] = b[gene_idx_b + j];
      }
    }
    else { //mean before or after logging
      tmpa = &a[gene_idx_a];
      tmpb = &b[gene_idx_b];
    }
    ma[idx] = get_ave(tmpa,*nacol,*isLogged,*meth);
    mb[idx] = get_ave(tmpb,*nbcol,*isLogged,*meth);
    fc[idx] = ma[idx] - mb[idx];
    if(!*doPairwise) {
      if((*nacol  <2) || (*nbcol < 2)) tt[idx] = 0.0;
      else tt[idx] = get_t_test(&a[gene_idx_a],*nacol,&b[gene_idx_b],*nbcol);
    }
    else {
      if(*nacol != *nbcol) { error("Need the same number of replicates in each group for paired t-tests");}
      if((*nacol  <2) || (*nbcol < 2)) tt[idx] = 0.0;
      else tt[idx] = get_pairwise_t_test(&a[gene_idx_a],*nacol,&b[gene_idx_b]);
    }
    idx++;
    gene_idx_a += *nacol;
    gene_idx_b += *nbcol;
  }
}

// This is a numerical approximation to the normal distribution as described in
// Abramowitz and Stegun: Handbook of Mathematical functions
// see page 931: 26.2.1, 932:26.2.17
double pnorm_approx(double z) {
    double b1 =  0.31938153;
    double b2 = -0.356563782;
    double b3 =  1.781477937;
    double b4 = -1.821255978;
    double b5 =  1.330274429;
    double p  =  0.2316419;
    double c2 =  0.3989423;
    double a =fabs(z);
    double t = 1.0/(1.0+a*p);
    double b = c2*exp((-z)*(z/2.0));
    double n = ((((b5*t+b4)*t+b3)*t+b2)*t+b1)*t;
    n = 1.0-b*n;
    if (z >  6.0) { return 1.0; };
    if (z < -6.0) { return 0.0; };
    if ( z < 0.0 ) n = 1.0 - n;
    return n;
}

// Given a double array length nx, rank it, and put the results in 'r'
void rank(double *x, int nx, double *r) {
  int i       = 0;
  int rank    = 1;
  int ranksum = 1;
  int ntie    = 1;
  int prev    = 0;
  r[0] = 1.0;
  for(i = 1; i < nx; i++) {
    if(x[i] == x[prev]) {
      ntie++;
      rank++;
      ranksum += rank;
    }
    else {
      if(ntie > 1) {
  while(prev < i) {
    r[prev] = (double) ranksum/ (double) ntie;
    prev++;
  }
      }
      rank++;
      ranksum = rank;
      r[i] = rank;
      prev = i;
      ntie = 1;
    }
  }
}

// a straight translation of relevant bits of the wilcox.test method in the R base library
double wilcox(double *x, int nx, double mu) {
  int i = 0;
  int j = 0;
  double *r    = 0;
  double *absx = 0;
  int    *xidx = 0;
  double STATISTIC = 0;
  double NTIES_SUM = 0;
  int    prev      = 0;
  int    ntie      = 0;
  double z         = 0;
  double SIGMA     = 0;
  double PVAL      = 0;
  for(i = 0; i < nx; i++) {
    x[j] = x[i] - mu;
    if(x[j] != 0) j++; // eliminate zeros
  }

  nx = j;
  r      = (double *) R_alloc(nx,sizeof(double));
  absx   = (double *) R_alloc(nx,sizeof(double));
  xidx   = (int *) R_alloc(nx,sizeof(int));

  for(i = 0 ; i < nx; i++) {
    absx[i] = fabs(x[i]);
    xidx[i] = i;
  }
  rsort_with_index(absx,xidx,nx);
  rank(absx,nx,r);
  for(i = 0; i < nx; i++) {
    r[i] = (x[xidx[i]] > 0) ? r[i] : -r[i];
  }

  for(i =0; i < nx; i++) {
    if(r[i] > 0) {
      STATISTIC += r[i];
    }
  }
  for(i = 1; i < nx; i++) {
    if(r[prev] == r[i]) {
      ntie++;
    }
    else {
      if(ntie > 1) {
  NTIES_SUM += ntie * ntie * ntie - ntie;
      }
      ntie = 0;
      prev = i;
    }
  }
  z     = STATISTIC - nx * (nx + 1)/4;
  SIGMA = sqrt(nx * (nx + 1) * (2 * nx + 1) / 24
         - NTIES_SUM / 48);
  PVAL  = pnorm_approx(z / SIGMA);
  PVAL    = 1 - PVAL;
  return(PVAL);
}


// compute the detection p-value for a particular probe using the algorithm described in
// Liu et al. Bioinformatics(2002) 1593-1599
// pms is a list of probe perfect matches, mms is a list of mismatches n, the number of probe-pairs.
// tao and sat are parameters, as desccribed in the Liu et al. paper
double pma(double *pms, double*mms, int n, double tao,double sat) {
  int i = 0;
  int *ignore  = 0;
  int totalSat = 0;
  int last     = 0;
  double *dv   = 0;
  double p     = 0;
if( sat >= 0 )
{
ignore = ( int * )R_alloc( n, sizeof( int ) ) ;
//dodgy saturation correction from the paper
totalSat = 0 ;
for( i = 0 ; i < n ; i++ )
{
  if( mms[ i ] > sat )
  {
    ignore[ i ] = 1 ;
    totalSat++ ;
  }
  else
    ignore[ i ] = 0 ;
}
last = 0 ;
if( ( totalSat > 0 ) & ( totalSat < n ) )
{ // ignore probes with saturated mms unless they're all saturated
  for( i = 0 ; i < n ; i++ )
  {
    if( !ignore[ i ] )
    {
      pms[ last ] = pms[ i ] ;
      mms[ last ] = mms[ i ] ;
      last++ ;
    }
  }
  n = last ;
}
}
dv = ( double * )R_alloc( n, sizeof( double ) ) ;
for( i = 0 ; i < n ; i++ )
{
dv[ i ] = ( pms[ i ] - mms[ i ] ) / ( pms[ i ] + mms[ i ] ) ;
}
p = wilcox( dv, i, tao ) ;
return( p ) ;
}


// compute for all probes
// assumes that pm mm pairs line up in the arrays and that the names do to. Also assumes that probes within a set are contiguous in each array.
// pm, mm and names are all length n long, and are, effectively, three columns from a matrix
// returns with 'dpval' containing the detection p values for each probeset.
void DetectionPValue (double *pm, double *mm, char **names, int *nprobes, double *tao, double *sat, double *dpval, int *nprobesets) {
  int start = 0;
  int i = 0;
  int j = 0;
  for(i = 1; i < *nprobes; i++) {
    if(strcmp(names[i],names[start]) != 0) {
  dpval[j] = pma(&(pm[start]),&(mm[start]),i-start,*tao,*sat);
      start = i;
      j++;
      if(j > *nprobesets) { error("Expecting %d unique probesets, found %d\n",*nprobesets,j); }
    }
  }
  dpval[j] = pma(&(pm[start]),&(mm[start]),i - start,*tao,*sat);
}

// given a spot on a chip, which grid cell is it in?
void grid(int x, int y, int nrow, int ncol, int gridx, int gridy, int *gx, int *gy) {
  *gx = (int) ((double) (gridx * x) / (double) nrow);
  *gy = (int) ((double) (gridy * y) / (double) ncol);
}


// given an idx array, a probe number and a column size get an x y location and value out of raw
void lookup(int *idx, int no, double *raw, int ncol, int *x, int *y, double *val) {
    *x = idx[no] % ncol;
    *y = idx[no] / ncol;
    *val = raw[idx[no]];
}


// given a load of probes in the array raw, chip order specified by idx with rws rows and cls cols
// divide the chip into xgridsize by ygridsize squares and compute the background
// put the background and sd of each grid square in zonebg,
void bgmas(int *idx, int *nidx,
           double *raw, int *nraw,
           int *rws, int *cls, int *xgridsize, int *ygridsize, double *zonebg, double *zonesd, double *corrected) {
  int i,j,x,y,gx,gy,gn,pn;
  double tmp;
  double val;
  double w,wsum,bg,nz;
  double nfrac = 0.5;
  int n  = *nidx;
  int rw = *nraw;
  int nrow = *rws;
  int ncol = *cls;
  int gxn = *xgridsize;
  int gyn = *ygridsize;
  int ngrid = gxn * gyn;
  double **scratches= (double **) R_alloc(ngrid,sizeof(double*));
  int    *ends      = (int *)     R_alloc(ngrid,sizeof(int));
  double *xc        = (double *)  R_alloc(ngrid,sizeof(double));
  double *yc        = (double *)  R_alloc(ngrid,sizeof(double));

  // Initialise the arrays containing the computed backgrounds for each cell
  for(i = 0;i<ngrid; i++) {
    zonebg[i] = 0;
    zonesd[i] = 0;
    ends[i]   = 0;
    scratches[i] = (double *) R_alloc(ncol*nrow/ngrid,sizeof(double));
  }
  // count the number of spots in each cell and store consecutively in scratch the values from each grid cell
  for(i = 0; i < n; i++) {
    lookup(idx, i, raw, ncol, &x, &y, &val);
    grid(x,y,nrow,ncol,gxn,gyn,&gx,&gy);
    gn = gx + gxn * gy;

    scratches[gn][ends[gn]] = val;

    ends[gn]++;  // store the number of spots in each cell, and also use this to find out where the stuff is in scratch
  }
  for(i = 0; i < ngrid; i++) {
    pn = (2.0 * ends[i]) / 100;             // 2% of the probes in each square
    partial_sort(scratches[i],ends[i], pn); // find lowest 2%
    for(j = 0; j < pn; j++) {
      zonebg[i] += scratches[i][j];         // mean
    }
    zonebg[i]   /= (double) pn;
    for(j = 0; j < pn; j++) {
      zonesd[i] += (scratches[i][j] - zonebg[i]) * (scratches[i][j] - zonebg[i]); // sd
    }
    zonesd[i]   = sqrt(zonesd[i] / (double) (pn - 1));
  }


  for(i = 0; i < gxn;i++) {
    xc[i] = (int) (((double) i + 0.5) * (double) ncol / (double) gxn);
  }
  for(i = 0; i < gxn;i++) {
    yc[i] = (int) (((double) i + 0.5) * (double) nrow / (double) gyn);
  }

  for(i = 0; i < rw; i++) {
    corrected[i] = raw[i];
  }

  for(i = 0; i < n; i++) {
    lookup(idx, i, raw, ncol, &x, &y, &val);
    wsum = 0;
    bg   = 0;
    nz   = 0;
    for(gy =0; gy < gyn;gy++) {
      for(gx =0; gx < gxn;gx++) {
  w = 1.0 / ((x - xc[gx]) * (x - xc[gx]) + (y - yc[gy]) * (y - yc[gy])+ 100.0);
  wsum += w;
  bg += w * zonebg[gx + gxn * gy];
  nz += w * zonesd[gx + gxn * gy];
      }
    }
    bg /= wsum;
    nz /= wsum;
    if(val < 0.5) val = 0.5;
    tmp = val - bg;
    if(tmp < nfrac * nz) tmp = nfrac * nz;
    corrected[idx[i]] = tmp;
  }
}



double t_scr[200];
double t_u[200];
double t_w[200];

double tukey(double *data, int N, double c) {
  int i;
  double t = 0;
  double m = 0;
  double s = 0;
  double wsum = 0;
  double epsilon = 0.0001;

  m = median(data,N);
  for(i = 0 ;i  < N; i++) {
    t_scr[i] = fabs(data[i] - m);
  }
  s = median(t_scr,N);
  for(i = 0; i < N;i++) {
    t_u[i] = (data[i] - m)/(c*s + epsilon);
    t_w[i] = (fabs(t_u[i]) <= 1.0) ? ((1.0 - t_u[i] * t_u[i]) * (1.0 - t_u[i] * t_u[i])) : 0.0;
    t += t_w[i] * data[i];
    wsum += t_w[i];
  }
  t /= wsum;
  return t;
}

double sb_scr[200];

double sb(double *pm, double *mm, int n) {
  int i;
  double r;
  for(i = 0; i < n; i++) {
    sb_scr[i] = log2(pm[i]) - log2(mm[i]);
  }
  r = tukey(sb_scr,n,5.0);
  return r;
}


double im(double pm, double mm, double sb, double ct, double st) {
  if(mm < pm) return mm;
  if((mm >= pm ) && (sb > ct)) return  pm / pow(2.0,sb);
  else return pm / (pow(2.0,(ct/ (1+(ct-sb)/st)) ));
}

double pv_scr[200];

double expcall(double *pm, double *mm, int n, double ct, double st) {
  double imv,v;
  double delta = 9.536743e-07;
  int i;
  double sbv = sb(pm,mm,n);
  double t;
  for(i = 0; i < n; i++) {
    imv = im(pm[i],mm[i],sbv,ct,st);
    v  = (pm[i] - imv);
    if(v < delta) v = delta;
    pv_scr[i] = log2(v);
  }
  t=tukey(pv_scr,n,5.0);
  return t;
}


// compute for all probes
// assumes that pm mm pairs line up in the arrays and that the names do to. Also assumes that probes within a set are contiguous in each array.
// pm, mm and names are all length n long, and are, effectively, three columns from a matrix
// returns with 'expressions' containing mas5 style calls
void GetExpressionLevels(double *pm, double *mm, char **names, int *nprobes, double *ct, double *st, double *expressions, int *nprobesets) {
  int start = 0;
  int i = 0;
  int j = 0;
  for(i = 1; i < *nprobes; i++) {
    if(strcmp(names[i],names[start]) != 0) {
      expressions[j] = expcall(&(pm[start]),&(mm[start]),i-start,*ct,*st);
      start = i;
      j++;
      if(j > *nprobesets) { error("Expecting %d unique probesets, found %d\n",*nprobesets,j); }
    }
  }
  expressions[j] = expcall(&(pm[start]),&(mm[start]),i - start,*ct,*st);
}
