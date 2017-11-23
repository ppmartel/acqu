#include "TA2TrackLine.h"

//_________________________________________________________________________________________
//  TA2TrackLine
//

//_________________________________________________________________________________________
Bool_t TA2TrackLine::BuildTrack(const TVector3 &r0, const TVector3 &r1, const Double_t &maxDirCosMag, const Double_t *limitsPsVertex)
{
  // Calculate fDirCos, fOrigin, fPsVertex from r1 and r0
  
  fOrigin = r0;
  fDirCos = r1 - fOrigin;
  
  // Check fDirCos length
  if (maxDirCosMag >= 0.)
  {
    if (fDirCos.Mag() > maxDirCosMag) return kFALSE;
  }
  
  // Calculate fPsVertex
  TVector3 rz; // dummy
  TA2Math::DistanceBetweenTwoLines(TVector3(0,0,0), TVector3(0,0,1), fOrigin, fDirCos, rz, fPsVertex);
  
  // Check pseudo vertex RZ limits
  if (limitsPsVertex)
  {
    if (fPsVertex.Perp() > limitsPsVertex[0]) return kFALSE;
    if (fPsVertex.Z() > limitsPsVertex [1]) return kFALSE;
    if (fPsVertex.Z() < limitsPsVertex [2]) return kFALSE;
  }
  
  // Return true in case of a good track
  return kTRUE;
}

ClassImp(TA2TrackLine)
