//--Author	JRM Annand    5th Feb 2004
//--Update	JRM Annand    6th Apr 2005 add ECutMisc, ECutCMisc
//--Update      JRM Annand   14th Apr 2005 Wild-card options for cut
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TTA2Cut
//
// Abstract base class for any condition
// Specify set of procedures which user-defined derivatives
// must implement
// User-defined condition classes must inherit from this one
//

#include "TA2Cut.h"


//-----------------------------------------------------------------------------
TA2Cut::TA2Cut(const char* name, UInt_t link, Double_t* opt )
  :TNamed(name, "Acqu-Root-Cut")
{
  // Set return false, default OR linkage
  // Store any provided wild-card options

  fIsTrue = EFalse;                   // not evaluated yet
  switch(link){
  case ECutAND:
  case ECutOR:
    // result OR, AND with previous result
    fLink = link;
    fIsNOT = EFalse;
    break;
  case ECutNAND:
    // NOT(result) AND with previous result
    fLink = ECutAND;
    fIsNOT = ETrue;
    break;
  case ECutNOR:
    // NOT(result) OR with previous result
    fLink = ECutOR;
    fIsNOT = ETrue;
    break;
  }
  // Look for any wild-card options and store them locally
  fNOpt = 0;
  if( !opt ) fOpt = NULL;
  else{
    while( opt[fNOpt] != (Double_t)EBufferEnd ) fNOpt++;
    fOpt = new Double_t[fNOpt];
    for( Int_t i=0; i<fNOpt; i++ ) fOpt[i] = opt[i];
  }
    
}

//-----------------------------------------------------------------------------
TA2Cut::~TA2Cut()
{
  // Free up allocated memory
  if( fOpt ) delete fOpt;
}

ClassImp(TA2Cut)
