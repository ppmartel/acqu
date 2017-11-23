//--Author	V Lisin      28th Jun 2004  original DAPHNE fortran -> C
//--Rev 	JRM Annand... 8th Jul 2004  AcquRoot C++ class
//--Update	JRM Annand...31st Mar 2005  fPhiCor = layerparm[]
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2CylWire
//
// Helical cathode strips on a cylindrical surface
//

#include "TA2CylWire.h"


//---------------------------------------------------------------------------
TA2CylWire::TA2CylWire( const char* name, Int_t nelem, Int_t maxclust,
			  Int_t maxclsize, void* det, Double_t* layerparm )
  :TA2WCLayer(name, nelem, maxclust, maxclsize, det )
{
  // Store dimensions, alignment and correction factors
  fRadius = layerparm[0];
  fLength = layerparm[1];
  for( Int_t i=0; i<3; i++ ){
    //    fPhiCor[i] = layerparm[i+2]*0.5;
    fPhiCor[i] = layerparm[i+2];
  }
  fPhiSpace = TMath::TwoPi()/fNElement;
}
ClassImp(TA2CylWire)
