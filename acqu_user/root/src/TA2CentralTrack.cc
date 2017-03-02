#include "TA2CentralTrack.h"

//_________________________________________________________________________________________
//  Particle track in the central part of the A2CB detector
//  (PID->MWPC0->MWPC1->NaI)
//

const Int_t TA2CentralTrack::kNullHit = ENullHit;
const Double_t TA2CentralTrack::kNullFloat = ENullFloat;

//_________________________________________________________________________________________
TA2CentralTrack::TA2CentralTrack()
{
  // Default constructor
  Reset();
}

ClassImp(TA2CentralTrack)
