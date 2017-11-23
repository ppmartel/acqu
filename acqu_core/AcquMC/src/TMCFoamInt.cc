//--Author	JRM Annand   17th Mar 2007   Separate integrand class
//--Rev         JRM Annand
//--Rev         JRM Annand    5th May 2008   Debug YComp()
//--Rev         JRM Annand   25th Jun 2008   General weighting density fn.
//--Rev         JRM Annand   30th Jun 2008   ReadData move from TMCFoamGenerator
//--Update      JRM Annand   17th Sep 2008   SetWgt() extended
//
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCFoamInt
//
// This provides a density function for the TFoam N-dimensional
// Monte Carlo generator.
// The default density evaluation method is by linear interpolation of
// an N-dimension function (e.g an N-fold differential cross section)
// which is evaluated on a regular grid of points
// Interpolation method:
// Multidimensional Interpolation Methods
// InterpMethods.pdf version 04/09/2006, K.C. Johnson

#include "TMCFoamInt.h"
#include "ARFile_t.h"


//-----------------------------------------------------------------------------
TMCFoamInt::TMCFoamInt( Int_t n, Double_t* scale, TA2System* sys, Int_t idens ) :
  TFoamIntegrand( )
{
  // Initial setup of N-dim generator for TFoam
  fSys = sys;                         // save AcquRoot facilities ptr
  fNDim = n;                          // # dimensions
  fNInterp = 1;                       // default 1 variable interpolated
  fScale = scale;                     // scaling factors for x[j]
  fM0 = new Int_t[fNDim];             // nearest-&-less-than grid indices
  fM1 = new Int_t[fNDim];             // fM1[j] = fM0[j] + s, s=0,1
  fM2 = new Int_t[fNDim];             // for linear interpolation compensation
  fIwgt = new Int_t[fNDim];           // weighting of interp output
  for( Int_t i=0; i<fNDim; i++ ) fIwgt[i] = EWgtUnity; // default weighting off
  fUj = new Double_t[fNDim];          // Linear "Cardinal" functions
  fXd = new Double_t[fNDim];          // x[j] - x[fM0[j]]
  fXscaled = new Double_t[fNDim];     // scaled variable vector
  fXarg = new Double_t[fNDim];        // vector of variables [0,1] interval
  fDensOpt = idens;                   // evaluation option
  fPolB = fPolT = fPolR = NULL;
  flRecPol = NULL;
  fCMtoLAB = NULL;
}


//---------------------------------------------------------------------------
TMCFoamInt::~TMCFoamInt()
{
}

//-----------------------------------------------------------------------------
Double_t TMCFoamInt::LinInterpN( Double_t* x, Bool_t comp )
{
  // Multi-linear interpolation of N-dimensional function evaluated on
  // a regular grid of points.
  //  Multidimensional Interpolation Methods
  //  InterpMethods.pdf version 04/09/2006, K.C. Johnson
  // fYN    are the evaluations on the grid
  // fXN[j] is the array of grid point values for each dimension
  // fSN[j] is the number of grid points for each dimension
  // x[j]  is the array of values at which to evaluate the linear interpolation
  // N     is the number of dimensions

  Double_t* xn;                    // grid values for particular dimension
  Int_t j,m;
  // Determine the grid points which "enclose" the input value
  // for each dimension
  for( j=0; j<fNDim; j++ ){
    xn = fXN[j];
    for( m=0; m<fSN[j]; m++ ){
      if( x[j] < xn[m] ) break;
    }
    m--;
    if( (m < 0) || (m == fSN[j]) ){
      printf(" Variable value %lf of dimension j = %d out of range\n",
	     x[j], j );
      return 0.0;
    }
    fM0[j] = m;
    fXd[j] = (x[j] - xn[m])/(xn[m+1] - xn[m]);
  }
  Double_t yFit = 0.0;
  LinIntpAccum(0,yFit,comp);
  return yFit;
}

//-----------------------------------------------------------------------------
void TMCFoamInt::Test()
{
  // Test 5-dimensional linear interpolation
  //
  TRandom *rand = new TRandom3();        // random number generator
  fNDim = 5;
  fXN = new Double_t*[fNDim];
  fM0 = new Int_t[fNDim];
  fM1 = new Int_t[fNDim];
  fM2 = new Int_t[fNDim];
  fSN = new Int_t[fNDim];
  fYN = new Double_t[128000];
  fUj = new Double_t[fNDim];
  fXd = new Double_t[fNDim];
   //
  Double_t ei[10];
  fXN[0] = ei;
  Double_t ej[10];
  fXN[1] = ej;
  Double_t ek[10];
  fXN[2] = ek;
  Double_t el[10];
  fXN[3] = el;
  Double_t em[10];
  fXN[4] = em;
  Double_t* y;
  y = fYN;
  for( Int_t i=0; i<10; i++ ){
    ei[i] = 0.3 + 0.02*i;
    ej[i] = 0.01 + 0.02*i;
    ek[i] = el[i] = em[i] = i * TMath::Pi()/9.;
    //    ei[i] = 0.3 + 0.002*i;
    //    ej[i] = 0.01 + 0.002*i;
    //    ek[i] = el[i] = em[i] = i * 0.1 * TMath::Pi()/9.;
  }
  fSN[0] = 10;
  fSN[1] = 10;
  fSN[2] = 10;
  fSN[3] = 10;
  fSN[4] = 10;

  for( Int_t i=0; i<10; i++ ){
    fM0[0] = i;
    for(Int_t j=0; j<10; j++ ){
      fM0[1] = j;
      for(Int_t k=0; k<10; k++ ){
	fM0[2] = k;
	for(Int_t l=0; l<10; l++ ){
	  fM0[3] = l;
	  for(Int_t m=0; m<10; m++ ){
	    fM0[4] = m;
	    Int_t ind = IJK(fM0);
	    y[ind] = GenFn(ei[i],ej[j],ek[k],el[l],em[m]);
	  }
	}
      }
    }
  }
  rand->SetSeed(43575);
  Double_t x[5];
  Double_t y1,y2,y3,y4;
  Int_t jy;
  for(Int_t i=0; i<10; i++){
    x[0] = rand->Uniform(ei[0], ei[9]);
    x[1] = rand->Uniform(ej[0], ej[9]);
    x[2] = rand->Uniform(ek[0], ek[9]);
    x[3] = rand->Uniform(el[0], el[9]);
    x[4] = rand->Uniform(em[0], em[9]);
    y1 = GenFn(x[0],x[1],x[2],x[3],x[4]);
    y2 = LinInterpN( x );
    y3 = LinInterpN( x,1 );
    y4 = GenFn(ei[fM0[0]],ej[fM0[1]],ek[fM0[2]],el[fM0[3]],em[fM0[4]]);
    jy = IJK(fM0);
    printf("%6.2lf %6.2lf %6.2lf %6.2lf %6.2lf %d %d %d %d %d (%d)-> %6.3lf  %6.3lf  %6.3lf  %6.3lf\n",
	   x[0],x[1],x[2],x[3],x[4],
	   fM0[0],fM0[1],fM0[2],fM0[3],fM0[4],jy,y1,y2,y3,y4);
  }
  delete fYN;
  delete fXN;
}

//-----------------------------------------------------------------------------
void TMCFoamInt::SetPol(Double_t x, Double_t y, Double_t z, Int_t sw)
{
  // Set polarisation vector...create it if it doesn't exist
  switch(sw){
  case EBeamPol:
  default:
    if( !fPolB ) fPolB = new TVector3(x,y,z);
    else fPolB->SetXYZ(x,y,z);
    break;
  case ETargetPol:
    if( !fPolT ) fPolT = new TVector3(x,y,z);
    else fPolT->SetXYZ(x,y,z);
    break;
  case ERecoilPol:
    if( !fPolR ) fPolR = new TVector3(x,y,z);
    else fPolR->SetXYZ(x,y,z);
    break;
  }
}
//-----------------------------------------------------------------------------
void TMCFoamInt::ReadData( Char_t* file )
{
  // Read n-dimensional grid of values of function Y = f(x0,x1,...xn)
  // 1st read n
  // then read grid-point values for each dimension (n lines)
  // then read the Y = f(X) values
  //
  ARFile_t infile(file,"r",fSys);
  Char_t* line;
  Int_t iline = 1;
  Int_t j = 0;
  Int_t n;
  Int_t nmax = 1;
  Double_t val[32];
  Double_t* xj;
  while( (line = infile.ReadLine()) ){
    switch( iline ){
    case 1:
      // read # dimensions 1st & # data-base variables
      if( sscanf( line, "%d", &n ) < 1 )
	fprintf(fSys->GetLogStream(),"TFoamInt: <Density file parse>");
      if( n != fNDim )
	fprintf(fSys->GetLogStream(),"TFoamInt: <Density file dimension>");
      n = 0;
      fXN = new Double_t*[fNDim];
      fSN = new Int_t[fNDim];
      iline++;
      break;
    case 2:
      // read array of grid X values for each dimension
      // X0, X1,....XN    N = fSN[j]
      // Max 32 values read per line
      fSN[j] = sscanf( line,
		       "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
		       val,   val+1, val+2, val+3, val+4, val+5, val+6, val+7,
		       val+8, val+9, val+10,val+11,val+12,val+13,val+14,val+15,
		       val+16,val+17,val+18,val+19,val+20,val+21,val+22,val+23,
		       val+24,val+25,val+26,val+27,val+28,val+29,val+30,val+31);
      fXN[j] = xj = new Double_t[fSN[j]];
      for(Int_t i=0; i<fSN[j]; i++) xj[i] = val[i];
      nmax *= fSN[j];              // update the Y array size
      j++;
      if( j == fNDim ){           // finished the X input?
	fYN = new Double_t[nmax];  // yes have final Y array size so make space
	iline++;
      }
      break;
    case 3:
      // Now read the Y values until EOF
      // Check that the array sizes tally
      // Max 32 values read per line
      j = sscanf( line,
		  "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
		  val,   val+1, val+2, val+3, val+4, val+5, val+6, val+7,
		  val+8, val+9, val+10,val+11,val+12,val+13,val+14,val+15,
		  val+16,val+17,val+18,val+19,val+20,val+21,val+22,val+23,
		  val+24,val+25,val+26,val+27,val+28,val+29,val+30,val+31);
      for( Int_t i=0; i<j; i++ ) fYN[n+i] = val[i];
      n += j;
      if( n > nmax ){
	fprintf(fSys->GetLogStream(),
		" Too many Y values %d, %d\n", n, nmax);
	fprintf(fSys->GetLogStream(),"<Density-file accounting>");
      }
      break;
    }
  }
  // any mismatch in the sizes is not tolerated
  if( n != nmax ){
    fprintf(fSys->GetLogStream()," Incorrect # Y values %d, %d\n", n, nmax);
    fprintf(fSys->GetLogStream()," TFoamInt: <Density-file accounting>");
  }
  // save the scaling factors required to convert from foam space
  // which operates on a {0,1} interval to the desired range of the
  // particular kinematic variable e.g. theta = 0 - 180 deg.
  for( j=0; j<fNDim; j++ ){
    xj = fXN[j];
    fScale[2*j] = xj[0];
    fScale[2*j+1] = xj[fSN[j]-1];
  }
  j = 0;
}
ClassImp(TMCFoamInt)
