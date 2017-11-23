//--Author	JRM Annand   10th Jul 2008   New class
//--Rev         JRM Annand
//--Update      JRM Annand

//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCDgpInt
//
// This provides a 2D density function for the TFoam N-dimensional
// Monte Carlo generator.
// Calculate 2-Body CMS d(g,p) differential cross section.
// Use parametrisation of Rossi et al, PR C40(1989),2412
// or Jenkins et al, PR C50(1994),74
// Foam vector elements are:
// 0  Egamma
// 1  Theta proton
// 2  Phi proton (polarisation not yet implemented

#include "TMCDgpInt.h"


// Legendre-Coefficient parameters D.A.Jenkins et al., PRC50(1994),74
Double_t A0Je[] = {12.17, -10.70, 137.90, -67.78, 6.82, -8.70, 0.2792, 91.1};
Double_t AIJe[4][4] = {
  {32.40, -67.00,  3.325,-5.81},
  {-131.40, -62.90, -2.585,-4.49},
  {-47.30, -82.70, -3.85,-12.07},
  {-1.48,  -2.59,  0.82,  0.00}
};
// Legendre-Coefficient parameters Rossi et al. PRC40(1989),2412
Double_t A0Ro[] = {27.57, -21.69, 143.19, -84.93, 9.69, -16.43, 0.29, 68.46};
Double_t AIRo[4][4] = {
  {21.62, -59.42,  3.33, -5.70},
  {-128.60, -63.30, -2.53, -4.41},
  {-20.14, -50.01, -2.40, -9.81},
  {-1.48,  -9.75,  0.27, -0.12}
};   

//-----------------------------------------------------------------------------
TMCDgpInt::TMCDgpInt( Int_t n, Double_t* scale, TA2System* sys,
			    Int_t idens )
: TMCFoamInt( n, scale, sys, idens )
{
  // Initial setup of N-dim generator for TFoam
}


//---------------------------------------------------------------------------
TMCDgpInt::~TMCDgpInt()
{
}

//---------------------------------------------------------------------------
Double_t TMCDgpInt::Dgp(Double_t E, Double_t costheta, Int_t parm)
{
  // Calculate differential cross section according to Jenkins or Rossi
  // parametrisation.
  // Switched by input parameter parm (default = Jenkins)

  Double_t a[5]; // Legendre coefficients
  Double_t p[5]; // Legendre polynomials
  //
  //     Calculate Legendre coefficients
  //
  if( parm == EDgpJenkins ){
    a[0] = A0Je[0] * TMath::Exp(A0Je[1]*E) 
         + A0Je[2] * TMath::Exp(A0Je[3]*E)
         + (A0Je[4] + A0Je[5]*E)/(1.0 + A0Je[7]*(E-A0Je[6])*(E-A0Je[6]));
    for( Int_t i=1; i<5; i++ ){
      Int_t j = i-1;
      a[i] = AIJe[j][0]*TMath::Exp(AIJe[j][1]*E)
           + AIJe[j][2]*TMath::Exp(AIJe[j][3]*E);
    }
  }
  else{
    a[0] = A0Ro[0] * TMath::Exp(A0Ro[1]*E) 
         + A0Ro[2] * TMath::Exp(A0Ro[3]*E)
         + (A0Ro[4] + A0Ro[5]*E)/(1.0 + A0Ro[7]*(E-A0Ro[6])*(E-A0Ro[6]));
    for( Int_t i=1; i<5; i++ ){
      Int_t j = i-1;
      a[i] = AIRo[j][0]*TMath::Exp(AIRo[j][1]*E)
	   + AIRo[j][2]*TMath::Exp(AIRo[j][3]*E);
    }
  }
  // Calculate Legendre polynomials and diff cross section
  Double_t x;
  p[0] = 1.0;
  //  p[1] = x = TMath::Cos(theta * TMath::DegToRad());
  if( fDensOpt == EDgpThNeutron ) x = -costheta;
  else x = costheta;
  p[1] = x;
  for( Int_t j=2; j<5; j++ ){
    p[j] = 2.0*x*p[j-1] - p[j-2] - (x*p[j-1] - p[j-2])/j;
  }
  Double_t sigma = 0.0;
  for( Int_t j=0; j<5; j++ ) sigma = sigma + a[j]*p[j];
  return sigma;
}

//---------------------------------------------------------------------------
void TMCDgpInt::ReadData( Char_t* file )
{
  // Energy 10 - 1000 MeV
  fScale[0] = 0.01;
  fScale[1] = 1.0;
  // Polar angle
  //  fScale[2] = 0.0;
  //  fScale[3] = TMath::Pi();
  // Cos( polar angle )
  fScale[2] = 1.0;             // 0 deg.
  fScale[3] = -1.0;            // 180 deg.
  // Asimuthal angle
  fScale[4] = 0.0;
  fScale[5] = TMath::TwoPi();
}
ClassImp(TMCDgpInt)
