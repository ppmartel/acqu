#include "TA2CylMwpcStrip.h"
#include "TA2Math.h"


//___________________________________________________________________________________________
TA2CylMwpcStrip::TA2CylMwpcStrip(const char* name, Int_t nelem, Int_t maxclust, Int_t maxclsize, void* det, Double_t* layerparm)
                :TA2CylMwpcLayer(name, nelem, maxclust, maxclsize, det )
{
  // Store dimensions, alignment and correction factors
  
  // Read params
  fRadius = layerparm[0];
  fLength = layerparm[1];
  fTgWC   = layerparm[2];
  fPitch  = layerparm[3];
  
  // Calc effective length
  fEffLength = 0.5*fLength + fPitch*TMath::Sqrt(1.+fTgWC*fTgWC);
  
  // Allocate memory
  fClustEn = new Double_t[maxclust+1];
  
  //
  fNClust = 0;
}

//___________________________________________________________________________________________
Double_t TA2CylMwpcStrip::CGClusterFromPhiZ(const Double_t &z, const Double_t &phi, const EHelicity type)
{
  // Find CG from known Phi and Z
  
  //
  Double_t phi1 = z/(2.*kPi*fRadius/fTgWC);
  Double_t phi2 = phi/(2.*kPi);
  
  //
  Double_t s;	// (CG - 1)/Ntot
  Int_t kMin, kMax;
  if( type == kPlus )
  {
    s = phi1 + phi2;
    kMin = -1;
    kMax =  1;
  }
  else
  {
    s = phi1 - phi2;
    kMin = -2;
    kMax =  0;
  }
  
  //
  Double_t clCG = ENullFloat;
  for (Int_t k=kMin; k<=kMax; ++k)
  {
    clCG = (s-k);
    if (clCG < 1. && clCG >= 0.)
    {
      fCGClust[fNClust++] = clCG;
      break;
    }
  }
  
  return clCG;
}

ClassImp(TA2CylMwpcStrip)
