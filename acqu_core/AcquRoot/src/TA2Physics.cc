//--Author	JRM Annand    4th Feb 2003
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand...30th Sep 2003...Incorporate TA2DataManager
//--Rev 	JRM Annand...18th Oct 2004...Periodic, data save, reconstr.
//--Rev 	JRM Annand...22nd Oct 2004...inherit TA2Apparatus
//--Rev 	JRM Annand...21st Jan 2007   4v0 update
//--Update	JRM Annand...25th Nov 2008   EPhysUndefined
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Physics
//
// Base case for Physics analysis, ie generally starting with 4-vectors
// and assembling these into possible reaction kinematics and determining
// observables.
// "General-purpose" physics impractical, users are expected to provide
// "the beef". Various utilities defined here
// TA2Physics is a Super-Apparatus
//
//

#include "TA2Physics.h"
#include "TAcquRoot.h"
#include "TA2Analysis.h"

// Valid Keywords for command-line setup of apparatus
enum { EPhysCondition=100, EPhysInitialise, EPhysDisplay, EPhysMisc, 
       EPhysTarget, EPhysPerm, EPhysPeriod, EPhysEndOfFile, EPhysFinish  };
static const Map_t kPhysKeys[] = {
  {"Target:",       EPhysTarget},
  {"Permutations:", EPhysPerm},
  {"Finish:",       EPhysFinish},
  {NULL,            -1}
};


//-----------------------------------------------------------------------------
TA2Physics::TA2Physics( const char* name, TA2Analysis* analysis )
  :TA2Apparatus( name, analysis, NULL, NULL )
{
  // Save pointer to TAcquRoot Acqu-to-Root interface class after
  // checking its an TAcquRoot type of class.
  // Pass main command string/key list to TA2System and histogram list
  // (if any) to TA2HistManager
  // Zero or NULL private variables
  //
  fP4miss = NULL;
  fP4target = NULL;
  fMmiss = NULL;
  fPmiss = NULL;
  fNperm = 0;
  fNpermutation = NULL;
  fIsReconstruct = ETrue;
  fChildren = analysis->GetChildren();
  AddCmdList( kPhysKeys );                  // physics-specific cmds
}


//-----------------------------------------------------------------------------
TA2Physics::~TA2Physics()
{
  // Free up allocated memory...after checking its allocated
  // detector and cuts lists
  //
  if( fP4miss ) delete fP4miss;
  if( fP4target ) delete fP4target;
  if( fMmiss ) delete fMmiss;
  if( fPmiss ) delete fPmiss;
  if( fNpermutation ) delete fNpermutation;
}

//---------------------------------------------------------------------------
void TA2Physics::PostInit()
{
  // Some further initialisation after all setup parameters read in
  // default Cut setup

  // ...followed by default apparatus procedure
  //
  TA2Apparatus::PostInit();
}

//-----------------------------------------------------------------------------
void TA2Physics::SetConfig(char* line, int key)
{
  // Generic apparatus setup procedure to perform
  // Loading and setup of TA2Physics classes
  //
  Double_t tm[4],tx,ty,tz;
  Int_t perm[8];
  Int_t n;
  switch (key){
  case EPhysPerm:
    // Wild card initialisation
    n = sscanf( line, "%d%d%d%d%d%d%d%d",
		perm,perm+1,perm+2,perm+3,perm+4,perm+5,perm+6,perm+7);
    if( n < 1 ){
      PrintError(line,"<Permutations specification>");
      return;
    }
    fNpermutation = new Int_t[n];
    fNperm = n;
    for(Int_t i=0; i<fNperm; i++) fNpermutation[i] = perm[i];
    break;
  case EPhysTarget:
    // Target parameters
    n = sscanf( line, "%lf%lf%lf%lf%lf%lf%lf", &tx,&ty,&tz,tm,tm+1,tm+2,tm+3 );
    if( n < 4 ){
      PrintError(line,"<Target specification>");
      return;
    }
    fNtarget = n - 3;
    fP4target = new TLorentzVector[fNtarget];
    fTargetPos.SetXYZ(tx,ty,tz);                  // Target Center Position
    for(Int_t i=0; i<fNtarget; i++)
      fP4target[i].SetPxPyPzE(0.0,0.0,0.0,tm[i]);
    break;
  default:
    // Not found here...
    // call default apparatus setup
    TA2Apparatus::SetConfig( line, key );
    break;
  }
  return;
}

//-----------------------------------------------------------------------------
void TA2Physics::LoadVariable( )
{
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup
  // LoadVariable( "name", pointer-to-variable, type-spec );
  // NB scaler variable pointers need the preceeding &
  //    array variable pointers do not.
  // type-spec ED prefix for a Double_t variable
  //           EI prefix for an Int_t variable
  // type-spec SingleX for a single-valued variable
  //           MultiX  for a multi-valued variable
  //
  TA2Apparatus::LoadVariable();
  if( fMmiss ) TA2DataManager::LoadVariable("Mmiss", fMmiss,        EDMultiX);
  if( fPmiss ) TA2DataManager::LoadVariable("Pmiss", fPmiss,        EDMultiX);
  TA2DataManager::LoadVariable("Ptheta",&fPtheta,      EDSingleX);
  TA2DataManager::LoadVariable("Pphi",  &fPphi,        EDSingleX);
  return;
}

//-----------------------------------------------------------------------------
void TA2Physics::Reconstruct()
{
}
ClassImp(TA2Physics)
