//--Author	JRM Annand   13th Apr 2005
//--Update	JRM Annand
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2H
// Base AcquRoot wrapper class for ROOT histograms.
// Contains further information on any condition which must be met
// before histogram is incremented and the histogram increment weight
// Default increment weight is 1.0

#include "TA2H.h"


//-----------------------------------------------------------------------------
TA2H::TA2H(TA2Cut* cut, Stat_t* wgt )
{
  // Save cut pointer (default cut = NULL for no applied conditions
  // Save increment weight variable pointer...if default NULL ptr supplied
  // Create default weight = 1
  fCut = cut;
  if( wgt ){ fWeight = wgt; fIsWeight = 1; }
  else{ fWeight = new Double_t[1]; *fWeight = 1.0; fIsWeight = 0; }
}

//-----------------------------------------------------------------------------
TA2H::~TA2H()
{
  // Free up allocated memory
  if( !fIsWeight ) delete fWeight;
}

ClassImp(TA2H)
