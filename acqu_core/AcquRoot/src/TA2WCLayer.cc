//--Author	V Lisin      28th Jun 2004  original DAPHNE fortran -> C
//--Update	JRM Annand... 8th Jul 2004  AcquRoot C++ class
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2WCLayer
//
// General methods for single layer wire chamber
//

#include "TA2WCLayer.h"
#include "TA2WireChamber.h"


//---------------------------------------------------------------------------
  TA2WCLayer::TA2WCLayer( const char* name, Int_t nelem,
			Int_t maxclust, Int_t maxclsize,
			void* det )
  :TA2System(name, NULL)
{
  //
  TA2WireChamber* cham = (TA2WireChamber*)det;
  fNElement = nelem;
  fIstart = cham->GetNelem();
  fIend = fIstart + nelem;
  fHits = cham->GetHits();
  fEnergy = cham->GetEnergy();
  fTime = cham->GetTime();
  fPos = cham->GetPosition();
  fMaxClust = maxclust;
  fMaxClustSize = maxclsize;
  fLayerHits = new Int_t[nelem+1];           // +1 is space for end-marker
  fClust = new Int_t[maxclust+1];
  fLenClust = new Int_t[maxclust+1];
  fCGClust = new Double_t[maxclust+1];

  fIdist = 2;
    
}

//---------------------------------------------------------------------------
TA2WCLayer::~TA2WCLayer()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  for( Int_t i=0; i<fMaxClust; i++ ){
    delete fClust;
    delete fLenClust;
    delete fCGClust;
  }
}
ClassImp(TA2WCLayer)
