//--Author	JRM Annand    5th Feb 2004
//--Update	JRM Annand   14th Apr 2005 GetCutList
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TTA2MultiCut
//
//	Base class for any detector system...cannot be instantiated
//	Specify set of procedures which user-defined derivative
//	must implement
//	User-defined detector classes must inherit from this one
//

#include "TA2MultiCut.h"


//-----------------------------------------------------------------------------
TA2MultiCut::TA2MultiCut(const char* name, UInt_t link )
  :TA2Cut(name, link)
{
  // Create the list to hold the single cuts
  fCutList = new TList;
}

//-----------------------------------------------------------------------------
TA2MultiCut::~TA2MultiCut()
{
  // Free up allocated memory
  TIter nextcut( fCutList );
  TA2Cut* cut;

  while( ( cut = (TA2Cut*)nextcut() ) ) delete cut;
  delete fCutList;;
}
ClassImp(TA2MultiCut)
