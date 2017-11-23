//--Author	JRM Annand    4th Feb 2003
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand...30th Sep 2003...Incorporate TA2DataManager
//--Rev 	JRM Annand...27th Sep 2004...Incorporate some PDG stuff
//--Rev 	JRM Annand...18th Oct 2004...Periodic procedures
//--Rev 	JRM Annand... 1st Dec 2004...DeleteArrays
//--Rev 	JRM Annand...14th Jan 2005...fTreeFileName
//--Rev 	JRM Annand...22nd Mar 2005...PostInit order, particle ID
//--Rev 	JRM Annand...15th Apr 2005...General-purpose particle ID cuts
//--Rev 	JRM Annand...20th Apr 2005   Add local 4-mom & PDG-code arrays
//--Rev 	JRM Annand...21st Jan 2007   4v0 update
//--Rev 	JRM Annand...31st Jan 2007   Particle-ID cuts
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Rev 	JRM Annand...24th Nov 2008...add TA2Particle
//--Update	P Ott     ...15th May 2013...add Int_t GetNParticle()
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2Apparatus
//
// Base case for apparatus classes...cannont be instantiated itself
// Specify set of procedures which user-defined derivatives must implement
// User-defined apparatus classes must inherit from this
//
//

#include "TA2Apparatus.h"
#include "TAcquRoot.h"
#include "TA2Analysis.h"
#include "TA2Particle.h"               // Info on particles

// Valid Keywords for command-line setup of apparatus
enum { EAppDetector, EAppCondition, EAppReconstruct, EAppInitialise,
       EAppDisplay, EAppParticleID, EAppParticleCuts, EAppMisc, EAppPeriod,
       EAppEndOfFile, EAppFinish, EAppSaveEvent };
static const Map_t kAppKeys[] = {
  {"Detector:",       EAppDetector},
  {"Data-Cut:",       EAppCondition},
  {"Reconstruct:",    EAppReconstruct},
  {"Initialise:",     EAppInitialise},
  {"Display:",        EAppDisplay},
  {"ParticleID:",     EAppParticleID},
  {"ParticleID-Cut:", EAppParticleCuts},
  {"Misc:",           EAppMisc},
  {"Period:",         EAppPeriod},
  {"EndFile:",        EAppEndOfFile},
  {"Finish:",         EAppFinish},
  {"Save-Event:",     EAppSaveEvent},
  {NULL,              -1}
};


//-----------------------------------------------------------------------------
TA2Apparatus::TA2Apparatus( const char* name, TA2System* analysis,
			    const Map_t* dets, const Map_t* hists)
  :TA2HistManager( name, kAppKeys, hists )
{
  // Save pointer to TAcquRoot Acqu-to-Root interface class after
  // checking its an TAcquRoot type of class.
  // Pass main command string/key list to TA2System and histogram list
  // (if any) to TA2HistManager
  // Zero or NULL private variables
  //
  if( !analysis->InheritsFrom("TA2Analysis") ){
    PrintError("","<TA2Apparatus/Physics non-TA2Analysis parent>",EErrFatal);
    return;
  }
  fParent = (TA2DataManager*)analysis;
  fP4 = NULL;
  fParticleInfo = NULL;
  fMinv = NULL;
  fPDGtype = NULL;
  fPDG_ID = NULL;
  fParticleID = NULL;
  fROOToutput = NULL;
  fNparticle = 0;
  fMaxParticle = 0;
  kValidChild = dets;
  fIsReconstruct = EFalse;
  fPCut = NULL;
  fPCutStart = NULL;
  fNSectorCut = NULL;
  fNCut = fNSector = 0;
}


//-----------------------------------------------------------------------------
TA2Apparatus::~TA2Apparatus()
{
  // Free up allocated memory...after checking its allocated
  // detector and cuts lists
  //
  DeleteArrays();
}

//-----------------------------------------------------------------------------
void TA2Apparatus::DeleteArrays()
{
  // Free up allocated memory...after checking its allocated
  // detector and cuts lists
  //
  if( fParticleID ) delete fParticleID;
  if( fMinv ) delete fMinv;
  if( fROOToutput ) delete (Char_t*)fROOToutput;
}

//---------------------------------------------------------------------------
void TA2Apparatus::PostInit()
{
  // Some further general purpose apparatus variable initialisation
  // Kinematic and particle ID variables
  // Check if PDG database has been setup
  //
  if( !fParticleID ){
    TA2Analysis* analysis = (TA2Analysis*)fParent;
    fParticleID = analysis->GetParticleID();
  }
  if( fParticleID )
    fPDGtype = fParticleID->GetPDGtype();         // considered particle IDs
  fP4 = new TLorentzVector[fMaxParticle];         // 4-vector buffer
  fParticleInfo = new TA2Particle[fMaxParticle];  // particle-info buffer
  fPDG_ID = new Int_t[fMaxParticle];              // ID of each particle
  fP4tot.SetXYZT(0,0,0,0);                        // zero fP4tot
  fMinv = new Double_t[fMaxParticle];             // invariant masses
  // output buffer for "physical" ie 4-vector output
  fROOToutput = new Char_t*[ sizeof(Int_t)*(fMaxParticle+1) +
			       sizeof(Double_t)*fMaxParticle*4 ];
  TA2DataManager::PostInit();                       // cuts and histograms
}

//-----------------------------------------------------------------------------
void TA2Apparatus::SetConfig(char* line, int key)
{
  // Generic apparatus setup procedure to perform
  // 	1. Loading and setup of TA2Detector classes
  //	2. Loading and setup of condition classes
  //	3. Switch on/off reconstruction methods
  //	4. Call any further methods to complete initialisation
  //	5. Setup histograms
  //
  TA2Analysis* analysis;
  switch (key){
  case EAppDetector:
    // Add Detector to list
    AddChild( line );
    break;
  case EAppCondition:
    // Add Condition to list
    ParseCut( line );
    break;
  case EAppReconstruct:
    // Switch on reconstruction
    fIsReconstruct = ETrue;
    break;
  case EAppDisplay:
    // Display setup
    ParseDisplay( line );
    break;
  case EAppInitialise:
    // To do at the end of initialisation
    PostInitialise();
    break;
  case EAppParticleID:
    // Setup particle identification
    analysis = (TA2Analysis*)fParent;             // main analysis class
    if( analysis->GetParticleID() ){
      fParticleID = analysis->GetParticleID();
      break;
    }
    fParticleID = new TA2ParticleID( line );      // new PDG database
    analysis->SetParticleID( fParticleID );       // return it to analysis
    break;
  case EAppMisc:
    // Wild card initialisation
    ParseMisc( line );
    break;
  case EAppPeriod:
    // Periodic procedures frequency specify
    ParsePeriod( line );
    break;
  case EAppEndOfFile:
    // Turn on end-of-file procedures
    ParseEndOfFile( line );
    break;
  case EAppFinish:
    // Turn on end-of-run procedures
    ParseFinish( line );
    break;
  case EAppSaveEvent:
    // Turn on reduced data saving
    SetSaveEvent();
  case EAppParticleCuts:
    // Particle identification cut file
    if( sscanf(line,"%d%d",&fNCut,&fNSector) < 2 ){
      PrintError(line,"<Particle-ID cut: line format>");
      return;
    }
    SortParticleIDCuts();
    break;
  default:
    PrintError( line, "<Unrecognised Configuration keyword>" );
    break;
  }
  return;
}


//-----------------------------------------------------------------------------
void TA2Apparatus::LoadVariable( )
{
  // Input name - variable pointer associations for any subsequent
  // cut or histogram setup
  //
  TA2DataManager::LoadVariable("Mtot", &fMtot,      EDSingleX);
  TA2DataManager::LoadVariable("Minv", fMinv,       EDSingleX);
  TA2DataManager::LoadVariable("Etot", &fEtot,      EDSingleX);
  TA2DataManager::LoadVariable("Nhit", &fNparticle, EISingleX);
  return;
}

//-----------------------------------------------------------------------------
void TA2Apparatus::InitSaveTree( )
{
  // Setup raw data output in ROOT Tree format
  // Process data file name....look for suffix .dat, .rd0 etc.
  // .dat Raw data file in Acqu Mk1 format
  // .rd0 Raw data file in ROOT tree format....1st filter pass of analysis
  // .rdN Nth filter pass of analysis
  //
  if( ! fIsSaveEvent ) return;                   // check if data save enabled
  const Char_t* name = "Physics.root";           // name of tree file
  fTreeFileName = new Char_t[strlen(name) + 1];
  strcpy(fTreeFileName, name); 
  fNbranch = 1;
  Char_t* form[] =
    { (Char_t*)"np/i:particleID[np]/i:Px[np]/D:Py[np]/D:Pz[np]/D:E[np]/D:" };
  Char_t* brname[] = { (Char_t*)"4-Vectors" };
  TA2DataManager::InitSaveTree( form, &fROOToutput, (Char_t*)"Physics Tree",
				brname);
}

//-----------------------------------------------------------------------------
void TA2Apparatus::SortParticleIDCuts()
{
  // Look for Particle ID cuts in the miscellaneous cut list
  // Cut options opt[0] = sector index, opt[1] = PDG particle code
  // Sort cuts into an indexed array for ease of access
  // Modified 31/01/07 to allow a single (set of) cut(s) to be applied
  // to multiple 2D plots, e.g. dE-E PID/CB particle-ID "bananas"
  //
  if(! fMiscCuts){
    PrintError("","<No misc cutlist for sorting particle-ID cuts>");
    return;
  }
  Int_t i;                                     // general indices
  // Create & initialise cut-ptr array, start-index 
  // and number of cuts for each sector
  fNSectorCut = new Int_t[fNSector];           // # part ID cuts/ PID sector
  fPCut = new TA2Cut*[fNCut];                  // array cut ptrs
  fPCutStart = new Int_t[fNSector];            // cut start index each sect
  for(i=0; i<fNSector; i++) fNSectorCut[i] = 0;// zero them
  for(i=0; i<fNCut; i++) fPCut[i] = NULL;      // zero ptrs
  for(i=0; i<fNSector; i++) fPCutStart[i] = -1;// start indices undefined
  //
  TIter nextcut( fMiscCuts->GetCutList() );    // linked list iterator
  TA2Cut* mcut;                                // cut class
  Int_t ncut = 0;                              // to check # cuts
  // Find the total number of valid cuts in the list
  // per PID element (sector)
  while( (mcut = (TA2Cut*)nextcut()) ){
    if( (i = (Int_t)mcut->GetOpt(0)) >= fNSector ) continue;
    if( !fNSectorCut[i] ) fPCutStart[i] = ncut;
    fPCut[ncut] = mcut;
    fNSectorCut[i]++;
    ncut++;
  }
  // Seek 1st defined set of sector cuts
  // If none its a fatal error
  for(i=0; i<fNSector; i++){ if( fNSectorCut[i] ) break; }
  if( i == fNSector )PrintError("","<No particle-ID cuts found>", EErrFatal);
  fprintf(fLogStream,
	  " Found %d particle-ID 2D Cuts, sector %d, %d cuts/sector\n",
	  ncut, i, fNSectorCut[i]);
  // 1st found set of cuts taken as default
  ncut = i;

  // Non-initialised cuts for sectors initialised to point to the default cut
  for(i=0; i<fNSector; i++){
    if( fNSectorCut[i] ) continue;
    fNSectorCut[i] = fNSectorCut[ncut];
    fPCutStart[i] = fPCutStart[ncut];
  }
}

ClassImp(TA2Apparatus)
