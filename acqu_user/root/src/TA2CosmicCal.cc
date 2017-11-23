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
// TA2CosmicCal
//
// For general description of calorimeter-type systems

#include "TA2CosmicCal.h"

// Default list of detector classes that the TA2CrystalBall 
// apparatus may contain
enum { EPlasticPID, ELongScint };
static Map_t kValidDetectors[] = {
  {"TA2LongScint",      ELongScint},
  {"TA2PlasticPID",     EPlasticPID},
  {NULL, 		-1}
};


//-----------------------------------------------------------------------------
TA2CosmicCal::TA2CosmicCal( const char* name, TA2System* analysis  )
  :TA2Apparatus( name, analysis, kValidDetectors )
{
  fLScint = NULL;
  fLTrig = NULL;
  fTrig = NULL;
}


//-----------------------------------------------------------------------------
TA2CosmicCal::~TA2CosmicCal()
{
  // Free up allocated memory
}

//-----------------------------------------------------------------------------
TA2DataManager*  TA2CosmicCal::CreateChild(const char* name, Int_t dclass)
{
  // Create a TA2Detector class for use with the Cosmic calibration
  // Valid detector types are...
  // 1. TA2LongScint   scintillator-bar array to calibrate
  // 2. TA2PlasticPID  any auxiliary trigger detectors

  if( !name ) name = Map2String( dclass );
  switch (dclass){
  default:
    return NULL;
  case ELongScint:
    fLScint = new TA2LongScint( name, this );
    return fLScint;
  case EPlasticPID:
    fTrig = new TA2PlasticPID( name, this );
    return fTrig;
  }
}

//-----------------------------------------------------------------------------
void TA2CosmicCal::LoadVariable( )
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
  //  TA2DataManager::LoadVariable("NphotMinv", fM_Nphoton,      EDSingleX);
  //
  TA2Apparatus::LoadVariable();
}

//-----------------------------------------------------------------------------
void TA2CosmicCal::PostInit( )
{
  // Store pointers to ADC's, scalers etc.
  // Set up histograms if desired.
  TA2Apparatus::PostInit();
}
ClassImp(TA2CosmicCal)
