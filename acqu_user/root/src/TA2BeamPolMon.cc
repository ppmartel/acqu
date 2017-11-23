//--Author	JRM Annand   20th Dec 2002
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand... 1st Oct 2003...Incorp. TA2DataManager
//--Rev 	JRM Annand...12th Dec 2003...fMulti 2D plots
//--Rev 	JRM Annand...19th Feb 2004...User code
//--Update	JRM Annand...12th May 2004...General CB,TAPS etc. use
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2BeamPolMon
//
// For general description of calorimeter-type systems

#include "TA2BeamPolMon.h"

// Default list of detector classes that the TA2BeamPolMon
// apparatus may contain
enum { EPairSpec, EMoeller };
static Map_t kValidDetectors[] = {
  {"TA2PairSpec",       EPairSpec},
  {"TA2Moeller",        EMoeller},
  {NULL, 		-1}
};


//-----------------------------------------------------------------------------
TA2BeamPolMon::TA2BeamPolMon( const char* name, TA2System* analysis  )
  :TA2Apparatus( name, analysis, kValidDetectors )
{
  // Ensure detector class pointers recognised as uninitialised
  fPairSpec = NULL;
  fMoeller  = NULL;
}


//-----------------------------------------------------------------------------
TA2BeamPolMon::~TA2BeamPolMon()
{
  // Free up allocated memory
  if(fPairSpec) delete fPairSpec;  
  if(fMoeller)  delete fMoeller;  
}

//-----------------------------------------------------------------------------
TA2DataManager*  TA2BeamPolMon::CreateChild(const char* name, Int_t dclass)
{
  // Create a TA2Detector class

  if( !name ) name = Map2String( dclass );
  switch (dclass){
  case EPairSpec:
    fPairSpec = new TA2PairSpec( name, this );
    return fPairSpec;
  case EMoeller:
    fMoeller = new TA2Moeller( name, this );
    return fMoeller;
  }
  
  PrintError(name,"Unknown detector for apparatus...", EErrFatal);
  return NULL;
}


ClassImp(TA2BeamPolMon)
