//--Author	JRM Annand   20th Dec 2002
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand... 1st Oct 2003...Incorp. TA2DataManager
//--Rev 	JRM Annand...12th Dec 2003...fMulti 2D plots
//--Rev 	JRM Annand...19th Feb 2004...User code
//--Rev 	JRM Annand...12th May 2004...General CB,TAPS etc. use
//--Update	JRM Annand...10th Feb 2005...gcc 3.4 compatible
//--Update	JRM Annand...12th Jul 2005...New TAPS detector class (R.Gregor)
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2Calorimeter
//
// For general description of calorimeter-type systems

#include "TA2Calorimeter.h"

#include "TA2CylMwpc.h"
#include "TA2PlasticPID.h"
#include "TA2CalArray.h"
#include "TA2TAPS_BaF2.h"

// Default list of detector classes that the TA2CrystalBall
// apparatus may contain
enum { ECylMWPC,
       EPlasticPID, ECalArray, ETAPS_BaF2 };
static Map_t kValidDetectors[] = {
  {"TA2CylMWPC",        ECylMWPC},
  {"TA2PlasticPID",     EPlasticPID},
  {"TA2CalArray",       ECalArray},
  {"TA2TAPS_BaF2",      ETAPS_BaF2},
  {NULL, 		-1}
};


//-----------------------------------------------------------------------------
TA2Calorimeter::TA2Calorimeter( const char* name, TA2System* analysis  )
  :TA2Apparatus( name, analysis, kValidDetectors )
{
  fCal = NULL;
  fPID = NULL;
  fPTracker = NULL;
}


//-----------------------------------------------------------------------------
TA2Calorimeter::~TA2Calorimeter()
{
  // Free up allocated memory
}

//-----------------------------------------------------------------------------
TA2DataManager*  TA2Calorimeter::CreateChild(const char* name, Int_t dclass)
{
  // Create a TA2Detector class for use with the TA2CrystalBall
  // Valid detector types are...
  // 1. TA2Ladder
  // 2. TA2BeamMonitor

  if( !name ) name = Map2String( dclass );
  switch (dclass){
  default:
    return NULL;
  case ECylMWPC:
    fPTracker = new TA2CylMwpc( name, this );
    return fPTracker;
  case EPlasticPID:
    fPID = new TA2PlasticPID( name, this );
    return fPID;
  case ECalArray:
    fCal = new TA2CalArray( name, this );
    return fCal;
  case ETAPS_BaF2:
    fCal = new TA2TAPS_BaF2( name, this );
    return fCal;
  }
}

//-----------------------------------------------------------------------------
void TA2Calorimeter::LoadVariable( )
{
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup.
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scaler variable pointers need the preceeding &
  //    array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  //           EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  //           MultiX  for a multi-valued variable

  //                            name        pointer          type-spec
  TA2DataManager::LoadVariable("NphotMinv", fM_Nphoton,      EDSingleX);
  //
  TA2Apparatus::LoadVariable();
}

//-----------------------------------------------------------------------------
void TA2Calorimeter::PostInit( )
{
  // Store pointers to ADC's, scalers etc.
  // Set up histograms if desired.
  if( !fParticleID )
    PrintError("",
	       "<Configuration aborted: ParticleID class MUST be specified>",
	       EErrFatal);
  //
  fMaxParticle = fCal->GetMaxCluster();
  TA2Apparatus::PostInit();
  fP4_Nphoton = new TLorentzVector[fMaxParticle];
  fM_Nphoton =  new Double_t[fMaxParticle];
}
ClassImp(TA2Calorimeter)
