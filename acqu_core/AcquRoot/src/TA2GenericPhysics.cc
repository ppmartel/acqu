//--Author	JRM Annand   18th Feb 2004   Use def physics
//--Rev
//--Rev         JRM Annand   13th May 2004   Start general physics suite
//--Rev         JRM Annand   28th Apr 2004   General photo-meson methods
//--Rev         JRM Annand   13th Jul 2005   SortNPhoton bugs
//--Rev         JRM Annand   25th Jul 2005   ED bug fix fP4tot
//--Rev         JRM Annand   22nd Sep 2006   pi+n analysis add lin pol
//--Rev         JRM Annand   14th Dec 2006   fM2gCBTAPS for online diagnostic
//--Rev         JRM Annand   14th Mar 2007   fEmProton for D(g,np) calib.
//--Rev         JRM Annand   21st Jul 2008   Compton scattering
//--Author	JRM Annand   24th Nov 2008
//--Rev
//--Update
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TA2GenericPhysics
//
// Framework for reaction reconstruction (physics) classes
// 

#include "TA2GenericPhysics.h"
#include "TAcquRoot.h"
#include "TA2Analysis.h"
#include "TA2Tagger.h"
#include "TAcquFile.h"
#include "TA2Particle.h"

// Valid Keywords for command-line setup of GenericPhysics
enum { EMiscGenPhys = 1000 };
static const Map_t kGenPhysKeys[] = {
  {"Misc-GenPhys:",        EMiscGenPhys},
  {NULL,            -1}
};


//-----------------------------------------------------------------------------
TA2GenericPhysics::TA2GenericPhysics( const char* name, TA2Analysis* analysis )
  :TA2Physics( name, analysis ) {
  // Initialise GenericPhysics variables here
  // Default null pointers, zeroed variables

  fTAGG = fCB = fTAPS = fTOF = NULL;
  fGenApp = NULL;
  fP4photon = fP4proton = fP4piplus = fP4neutron = fP4pi0 = fP4gprime
    = fP4eta = fP4rootino = NULL;
  fNphoton = fNproton = fNpiplus = fNneutron = fNpi0 = fNgprime
    = fNeta = fNrootino = 0;
  fNGenApp = 0;
  fEmP = fEmR = NULL;
  fMaxParticle = fMaxTagg = 0;

  AddCmdList( kGenPhysKeys );       // command-line recognition for SetConfig()
}


//-----------------------------------------------------------------------------
TA2GenericPhysics::~TA2GenericPhysics()
{
  // Free up allocated memory...after checking its allocated
  // detector and cuts lists
}

//-----------------------------------------------------------------------------
void TA2GenericPhysics::SetConfig(Char_t* line, Int_t key)
{
  // Any special command-line input for Crystal Ball apparatus

  switch (key){
  case EMiscGenPhys:
    //  sample
    break;
  default:
    // default main apparatus SetConfig()
    TA2Physics::SetConfig( line, key );
    break;
  }
}

//---------------------------------------------------------------------------
void TA2GenericPhysics::PostInit()
{
  // Initialise links to apparati
  // Initialisation will abort if Tagger not initialised
  // CB/TAPS/TOF optional
  //

  Int_t i,j;
  const Char_t* genAppName[] = { "CB", "TAPS", "TOF", NULL };

  fTAGG = (TA2Tagger*)((TA2Analysis*)fParent)->GetChild("TAGG");
  if(!fTAGG) PrintError("","<No Tagger class found in annalysis>",EErrFatal);
  else{
    fTAGGinfo = fTAGG->GetParticleInfo();
    fMaxTagg = fTAGG->GetMaxParticle();
  }
  fGenApp = new TA2Apparatus*[EMaxGenApp];
  fGenAppInfo = new TA2Particle*[EMaxGenApp];
  for( i=0,j=0; genAppName[i]; i++ ){
    fGenApp[j] =(TA2Apparatus*)((TA2Analysis*)fParent)->GetChild(genAppName[i]);
    if( fGenApp[j] ){
      fGenAppInfo[j] = fGenApp[j]->GetParticleInfo();
      fMaxParticle += fGenApp[j]->GetMaxParticle();
      j++;
      switch(i){
      case 0:
      default:
	fCB = fGenApp[j];
	break;
      case 1:
	fTAPS = fGenApp[j];
	break;
      case 2:
	fTOF = fGenApp[j];
	break;
      }
    }
    else
      fprintf(fLogStream, "<TA2GenericPhysics no apparatus named %s found>\n",
	      genAppName[i]);
  }
  fNGenApp = j;
  fEm = new Double_t[fMaxTagg * 2];
  fEmP = fEm; fEmR = fEm + fMaxTagg;
  // containers for sorted 4-momentum pointers
  TLorentzVector* p4;
  fP4photon =  new TLorentzVector*[fMaxParticle];   
  fP4proton =  new TLorentzVector*[fMaxParticle];
  fP4piplus =  new TLorentzVector*[fMaxParticle];
  fP4neutron = new TLorentzVector*[fMaxParticle];
  fP4pi0 =     new TLorentzVector*[fMaxParticle];
  p4 = new TLorentzVector[fMaxParticle];           // space for pi0 4-mom
  for(i=0; i<fMaxParticle; i++) fP4pi0[i] = p4 + i;
  fP4gprime =  new TLorentzVector*[fMaxParticle];
  fP4eta =     new TLorentzVector*[fMaxParticle];
  p4 = new TLorentzVector[fMaxParticle];           // space for eta 4-mom
  for(i=0; i<fMaxParticle; i++) fP4eta[i] = p4 + i;
  fP4rootino = new TLorentzVector*[fMaxParticle];
  p4 = new TLorentzVector[fMaxParticle];           // space for unknown 4-mom
  for(i=0; i<fMaxParticle; i++) fP4rootino[i] = p4 + i;


  // Default physics initialisation
  TA2Physics::PostInit();
}

//-----------------------------------------------------------------------------
void TA2GenericPhysics::LoadVariable( )
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

  TA2Physics::LoadVariable();
  TA2DataManager::LoadVariable("Nphoton",      &fNphoton,       EISingleX);
  TA2DataManager::LoadVariable("Nproton",      &fNproton,       EISingleX);
  TA2DataManager::LoadVariable("Npiplus",      &fNpiplus,       EISingleX);
  TA2DataManager::LoadVariable("Nneutron",     &fNneutron,      EISingleX);
  TA2DataManager::LoadVariable("Npi0",         &fNpi0,          EISingleX);
  TA2DataManager::LoadVariable("Ngprime",      &fNgprime,       EISingleX);
  TA2DataManager::LoadVariable("Neta",         &fNeta,          EISingleX);
  TA2DataManager::LoadVariable("Nrootino",     &fNrootino,      EISingleX);
  TA2DataManager::LoadVariable("EmP",          fEmP,            EDMultiX);
  TA2DataManager::LoadVariable("EmR",          fEmR,            EDMultiX);
  return;
}

//-----------------------------------------------------------------------------
void TA2GenericPhysics::Reconstruct()
{
  // Framework for physics reconstruction

  fNphoton = fNproton = fNpiplus = fNneutron = fNpi0 = fNgprime =
    fNeta = fNrootino = 0;                 // zero particle counters
  fNprompt = fNrandom = fNparaP = fNparaR = fNperpP = fNperpR = 0;

  //  fNparticle = ncb + ntaps + ntof;         // total number particles (hits)
  fP4tot.SetXYZT(0.0,0.0,0.0,0.0);         // zero total out 4-momentum
  //
  Int_t i,j;
  TA2Particle* pInfo;
  fNParticle = 0;
  // Get 4-momentum info from all apparati
  // and sort into particle type
  for( i=0; i<fNGenApp; i++ ){                // loop over apparati
    Int_t n = fGenApp[i]->GetNParticle();
    for( j=0; j<n; j++ ){                     // loop over apparati
      pInfo = fGenAppInfo[i] + j;
      fP4tot += pInfo->GetP4();
      fNParticle++;
      switch( pInfo->GetParticleID() ){         // PDG code
      case kGamma:                              // photon
	fP4photon[fNphoton] = pInfo->GetP4A();  // 4-momentum in photon list
	fNphoton++;
	break;
      case kProton:                             // proton
	fP4proton[fNproton] = pInfo->GetP4A();  // 4-momentum in proton list
	fNproton++;
	break;
      case kPiPlus:                             // pi+
	fP4piplus[fNpiplus] = pInfo->GetP4A();  // 4-momentum in pi+ list
	fNpiplus++;
	break;
      default:                                  // don't know
	fP4rootino[fNrootino] = pInfo->GetP4A();// 4 momentum in unknown list
	fNrootino++;                          
      }
    }
  }

  MarkEndBuffer();
}
ClassImp(TA2GenericPhysics)
