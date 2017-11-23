//--Author	V Lisin      28th Jun 2004  original DAPHNE fortran -> C
//--Rev 	JRM Annand... 8th Jul 2004  AcquRoot C++ class
//--Update	JRM Annand...31st Mar 2005  fPhiCor mod
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2CylStrip
//
// Helical cathode strips on a cylindrical surface
//

#include "TA2CylStrip.h"


//---------------------------------------------------------------------------
TA2CylStrip::TA2CylStrip( const char* name, Int_t nelem, Int_t maxclust,
			  Int_t maxclsize, void* det, Double_t* layerparm )
  :TA2WCLayer(name, nelem, maxclust, maxclsize, det )
{
  // Store dimensions, alignment and correction factors
  fRadius = layerparm[0];
  fLength = layerparm[1];
  fTgWC = layerparm[2];
  fZ0 = layerparm[3];
  fPitch = layerparm[4];
  for( Int_t i=0; i<3; i++ ){
    fZCor[i] = layerparm[i+5];
    fPhiCor[i] = layerparm[i+8]/TMath::TwoPi();
  }
  fEffLength = 0.5 * fLength + fPitch*TMath::Sqrt(1 + fTgWC*fTgWC);
}
ClassImp(TA2CylStrip)
