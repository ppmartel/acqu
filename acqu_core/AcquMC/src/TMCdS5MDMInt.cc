//--Author	JRM Annand   17th Mar 2007   Separate integrand class
//--Rev         JRM Annand
//--Rev         JRM Annand    5th May 2008   Debug YComp()
//--Rev         JRM Annand   25th Jun 2008   General weighting density fn.
//--Update      JRM Annand   30th Jun 2008   Extra dimension for pion phi

//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCdS5MDMInt
//
// This provides a density function for the TFoam N-dimensional
// Monte Carlo generator.
// The default density evaluation method is by linear interpolation of
// an N-dimension function (e.g an N-fold differential cross section)
// which is evaluated on a regular grid of points
// Interpolation method:
// Multidimensional Interpolation Methods
// InterpMethods.pdf version 04/09/2006, K.C. Johnson

#include "TMCdS5MDMInt.h"
#include "ARFile_t.h"


//-----------------------------------------------------------------------------
TMCdS5MDMInt::TMCdS5MDMInt( Int_t n, Double_t* scale, TA2System* sys,
			    Int_t idens )
: TMCFoamInt( n, scale, sys, idens )
{
  // Initial setup of N-dim generator for TFoam
  fDataSize = 1;
  fIJKoffset = 0;
  fPSO = new Double_t[2];
  for( Int_t i=0; i<2; i++ ) fPSO[i] = 0.0;
  fDataSize = 1;
}


//---------------------------------------------------------------------------
TMCdS5MDMInt::~TMCdS5MDMInt()
{
}

//-----------------------------------------------------------------------------
void TMCdS5MDMInt::ReadData( Char_t* file )
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
  Int_t nInterp = 1;
  Int_t nmax = 1;
  Double_t val[32];
  Double_t* xj;
  while( (line = infile.ReadLine()) ){
    switch( iline ){
    case 1:
      // read # dimensions 1st & # data-base variables
      sscanf( line, "%d", &n );
      if( n != 5 )
	fprintf(fSys->GetLogStream(),
		"TMCdS5MDMInt: <Density file dimension>");
      nInterp = fNDim - n + 1;
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
      fDataSize *= fSN[j];              // update the Y array size
      j++;
      if( j == 5 ){                // finished the X input?
	nmax = fDataSize * nInterp;
	fYN = new Double_t[nmax];  // yes have final Y array size so make space
	if( nInterp == 2 ){
	  fSN[j] = 2;
	  fXN[j] = new Double_t[fSN[j]];
	  fXN[j][0] = 0.0; fXN[j][1] = TMath::TwoPi();
	}
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
		" TMCdS5MDMInt: Too many Y values supplied %d, %d\n", n, nmax);
      }
      break;
    }
  }
  // any mismatch in the sizes is not tolerated
  if( n != nmax ){
    fprintf(fSys->GetLogStream(),
	    " TMCdS5MDMInt: <Incompatible # Y values %d, %d>\n", n, nmax);
  }
  // save the scaling factors required to convert from foam space
  // which operates on a {0,1} interval to the desired range of the
  // particular kinematic variable e.g. theta = 0 - 180 deg.
  for( j=0; j<fNDim; j++ ){
    xj = fXN[j];
    fScale[2*j] = xj[0];
    fScale[2*j+1] = xj[fSN[j]-1];
  }
  fNInterp = nInterp;
  j = 0;
}
ClassImp(TMCdS5MDMInt)
