//--Author	JRM Annand... 1st Sep 2004  AcquRoot C++ class
//--Update	JRM Annand...
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Track
//
// Charged particle tracks, reconstructed from position sensitive detector
// information
//

#include "TA2Track.h"


//---------------------------------------------------------------------------
TA2Track::TA2Track( const char* name )
  :TA2System(name, NULL)
{
  fOrigin = new TVector3(0,0,0);
  fDirCos = new TVector3(0,0,0);
  fVertex = new TVector3(0,0,0);
}

//---------------------------------------------------------------------------
TA2Track::~TA2Track()
{
  // Free up all allocated memory
  // ...arrays created at the initialisation stage
  delete fOrigin;
  delete fDirCos;
  delete fVertex;
}
ClassImp(TA2Track)
