//--Author	JRM Annand   30th Jun 2007  Move foam to generator	      
//--Rev 	JRM Annand...16st Apr 2008  Add TMCdS5MDMParticle option
//--Rev 	JRM Annand...25th Jun 2008  General foam weighting option
//--Rev 	JRM Annand...26th Jun 2008  Different models into this class
//--Rev 	JRM Annand...17th Sep 2008  Weighting options extended
//--Update	JRM Annand...25th Jan 2009  Check not beam for fReaction

//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCFoamGenerator
//
// Monte Carlo Kinematics Generator
// Use TFoam to sample N-dimensional kinematic distributions

#include "TMCFoamGenerator.h"
#include "MCNtuple.h"
#include "ARFile_t.h"



// Main command options
static const Map_t kMCFoamCmd[] = {
  {"Foam-Option:",       EMCFoamOpt},
  {"Foam-Distribution:", EMCFoamDist},
  {"Foam-Limits:",       EMCFoamLimits},
  {"Foam:",              EMCFoamParticle},
  {"Foam-Weight:",       EMCFoamWgt},
  {"Beam-Pol:",          EMCFoamPolB},
  {"Target-Pol:",        EMCFoamPolT},
  {"Recoil-Pol:",        EMCFoamPolR},
  {"Foam-Model:",        EMCFoamModel},
  {NULL,                 -1}
};
// Weighting options for Foam function
static const Map_t kMCFoamWgt[] = {
  {"Bremsstrahlung",     EWgtBrem},      // Bremsstrahlung
  {"SinTheta",           EWgtSinTh},     // Polar angle phase space
  {"Brem-Res",           EWgtBremRes},   // Brem + resonance
  {"Unity",              EWgtUnity},     // No weight
  {NULL,                 -1}
};
// Modeling options for Foam function
static const Map_t kMCFoamModel[] = {
  {"SAID",               EMCFoamSAIDM },      // SAID DB pion eta production
  {"MDM",                EMCFoamMDMM },       // Radiative pion/eta production
  {"Dgp",                EMCFoamDgpM },       // Deuteron photodisintegration
  {"Default",            EMCFoamDefaultM},    // No weight
  {NULL,                 -1}
};
// Modeling options for Foam function
static const Map_t kMCFoamParticle[] = {
  {"MDM",                EMCFoamMDMdS5},     // g p -> g' pi N final state
  {"Default",            EMCFoamDefault},    // Default
  {NULL,                 -1}
};


//-----------------------------------------------------------------------------
TMCFoamGenerator::TMCFoamGenerator( const Char_t* name, Int_t seed )
  :TMCGenerator( name, seed  )
{
  // Basic initialisation of MC generator
  // Initialise random-number generator and PDG database
  AddCmdList( kMCFoamCmd );                  // foam-specific cmds
  fFoamList = NULL;
  fFoamX = NULL;
  fInt = NULL;
  fX = fXscale = fScale = NULL;
  fFoamInitOpt = new Double_t[8];
  fFoamInitOpt[0] = 1000;      // # cells allocated in N-dim space
  fFoamInitOpt[1] = 200;       // # events for cell for MC exploration
  fFoamInitOpt[2] = 8;         // # bins in edge-histogram for MC exploration
  fFoamInitOpt[3] = 1;         // 0=weighted events, 1=wgt-const-1 MC events
  fFoamInitOpt[4] = 2;         // Max wgt reduction, =1 for variance reduction
  fFoamInitOpt[5] = 25;        // Max # eff wgt=1 events/bin, =0 deactivates
  fFoamInitOpt[6] = 1;         // Output verbosity level 0,1,2
  fFoamInitOpt[7] = 1.1;       // Max wgt used to get wgt=1 MC events
  fNDim = 0;
  fFnOpt = 0;
}

//---------------------------------------------------------------------------
TMCFoamGenerator::~TMCFoamGenerator()
{
  Flush();
}

//---------------------------------------------------------------------------
void TMCFoamGenerator::Flush()
{
  // Delete allocated persistant memory
  TMCGenerator::Flush();
  if( fFoamList ) delete fFoamList;
  if( fFoamX ) delete fFoamX;
  if( fInt ) delete fInt;
  if( fX ) delete fX;
  if( fXscale ) delete fXscale;
  if( fScale ) delete fScale;
  if( fFoamInitOpt ) delete fFoamInitOpt;
}

//-----------------------------------------------------------------------------
void TMCFoamGenerator::SetConfig(Char_t* line, Int_t key)
{
  // Parse a line of configuration information.
  // Extra to TMCGenerator...Mod parameters for TFoam initialisation

  Double_t* opt;
  Double_t val[3];
  Int_t ival[16];
  TMCParticle* p;
  Int_t iopt;
  Char_t name[64];
  Char_t pname[64];
  TMCFoamParticle* foam;
  Double_t lowlim,highlim;         // to reset limits for variable ilim
  Int_t ikin;                      // kin variable on which to reset limits
  Int_t i, iwgt;                   // foam weighting
  switch (key){
  case EMCFoamOpt:
    // Supply parameters which control TFoam setup
    // Parse Error is fatal
    opt = fFoamInitOpt;
    iopt = sscanf(line,"%lf%lf%lf%lf%lf%lf%lf%lf",
		  opt, opt+1, opt+2, opt+3, opt+4, opt+5, opt+6, opt+7);
    if( iopt != 8) PrintError(line,"<Parse TFoam Initialise>",EErrFatal);
    break;
  case EMCFoamDist:
    if( sscanf(line,"%d%d%s", &fNDim, &fFnOpt, name) != 3)
      PrintError(line,"<Foam distribution parse error>",EErrFatal);
    if( fNDim >= EMCFoamMaxDim )
      PrintError(line,"<Foam #dimensions too large>", EErrFatal);
    fScale = new Double_t[2*fNDim];
    fXscale = new Double_t[fNDim];
    fX = new Double_t[fNDim];
    ReadDatabase( name );
    break;
  case EMCFoamLimits:
    // Set new limits on vector elements for foam
    // read vector index of variable, low and high values
    if( !fScale ){
      PrintError(line,"<Foam database must be input before resetting limits>");
      break;
    }
    iopt = sscanf(line,"%d%lf%lf",&ikin,&lowlim,&highlim);
    if( iopt != 3 )
      PrintError(line,"<Parse of kinematic limits>",EErrFatal);
    if( ikin >= fNDim ){
      PrintError(line,"<Invalid kimenatic variable index supplied>");
      break;
    }
    ikin *= 2;
    if( (lowlim >= fScale[ikin])  && (highlim <= fScale[ikin+1]) &&
	(highlim > lowlim) ){
      fScale[ikin] = lowlim;
      fScale[ikin+1] = highlim;
    }
    else{
      PrintError(line,"<Kinematic limit out of range>");
      break;
    }
    break;
  case EMCFoamParticle:
    // Particles using TFoam for multi-dimensional MC generation
    if( !fFoamList ) fFoamList = new TList();
    if( (iopt = sscanf(line,"%s%d%d%d%d%d%s",name,
		       ival,ival+1,ival+2,ival+3,ival+4,pname)) < 6 )
      PrintError(line,"<Foam input parse error>",EErrFatal);
    for( i=0; i<5; i++ ){
      if( ival[i] >= fNDim )
	PrintError("","<Foam-link Index outside of foam dimension>",EErrFatal);
    }
    // Search name on particle list
    p = (TMCParticle*)fParticle->FindObject(name);
    if( !p ) PrintError( line,"<Foamisation of particle>",EErrFatal );
    //
    // Choose foam particle type
    switch( Map2Key(pname, kMCFoamParticle) ){
    case EMCFoamDefault:
    default:
      foam = new TMCFoamParticle( name, p, fXscale, fScale, ival);
      break;
    case EMCFoamMDMdS5:
      foam = new TMCdS5MDMParticle( name, p, fXscale, fScale, ival);
      break;
    }
    fParticle->AddBefore( p, foam );
    fParticle->Remove( p );
    if( (p = (TMCParticle*)fReactionList->FindObject(name)) ){
      fReactionList->AddBefore( p, foam );
      fReactionList->Remove( p );
    }
    if( (p = foam->GetParent()) ) p->ReplaceDecay( foam );
    else{
      if( strcmp(name,"Beam") )
	fReaction = foam;             // reaction particle if not beam
    }
    fFoamList->AddLast( foam );
    break;
  case EMCFoamPolB:
    iopt = sscanf(line,"%lf%lf%lf", val,  val+1, val+2 );
    if( iopt != 3 )
      PrintError(line,"<Beam polarisation parse>",EErrFatal);
    fInt->SetPolB( val[0], val[1], val[2] );
    break;
  case EMCFoamPolT:
    iopt = sscanf(line,"%lf%lf%lf", val,  val+1, val+2 );
    if( iopt != 3 )
      PrintError(line,"<Target polarisation parse>",EErrFatal);
    fInt->SetPolT( val[0], val[1], val[2] );
    break;
  case EMCFoamPolR:
    iopt = sscanf(line,"%lf%lf%lf", val,  val+1, val+2 );
    if( iopt != 3 )
      PrintError(line,"<Recoil polarisation input>",EErrFatal);
    fInt->SetPolR( val[0], val[1], val[2] );
    break;
  case EMCFoamWgt:
    // Weighting of TFoam integrand density fn
    iopt = sscanf(line,"%s%d%lf%lf%lf",name,&i,val,val+1,val+2);
    if( iopt < 2 )
      PrintError(line,"<Foam integrand density weighting>",EErrFatal);
    if( !fInt )
      PrintError(line,"<Foam integrand not created for density wgt>",EErrFatal);
    iwgt = Map2Key(name, kMCFoamWgt);
    fInt->SetWgt( iwgt, i, val );
    break;
  case EMCFoamModel:
    // Choose particular model for TFoam calculation
    // Parse Error is fatal
    if( sscanf(line,"%s",name) < 1)
      PrintError(line,"<Parse TFoam model choice>",EErrFatal);
    fFoamModel = Map2Key(name, kMCFoamModel);
    break;
  default:
    // Otherwise call standard line parser
    TMCGenerator::SetConfig( line, key );
    break;
  }
}

//-----------------------------------------------------------------------------
void TMCFoamGenerator::PostInit( )
{
  // Set default initialise options
  // See TFoam reference at http://root.cern.ch
  // The vales here are the default ones which are used unless reset by
  // "Foam-Initialise:" command-line input
  // Work in progress!!!!

  if( fNDim ){
    fFoamX = new TFoam( "FoamX" );                // Create Simulator
    fFoamX->SetkDim( fNDim );                     // No. of dimensions
    fFoamX->SetnCells( (Long_t)fFoamInitOpt[0] ); // # cells
    fFoamX->SetnSampl( (Long_t)fFoamInitOpt[1] ); // # events for cell determ
    fFoamX->SetnBin( (Int_t)fFoamInitOpt[2] );    // # hist bins cell determ
    fFoamX->SetOptRej( (Int_t)fFoamInitOpt[3] );  // use default 1
    fFoamX->SetOptDrive( (Int_t)fFoamInitOpt[4] );// max-wgt/variance reduction
    fFoamX->SetEvPerBin( (Int_t)fFoamInitOpt[5] );// # effective events/bin
    fFoamX->SetChat( (Int_t)fFoamInitOpt[6] );    // verbosity
    fFoamX->SetMaxWtRej( fFoamInitOpt[7] );       // Max wgt for wgt=1 events
    fFoamX->SetRho(fInt);                         // Set N-dim distribution
    fFoamX->SetPseRan(fRand);                     // Set random generator
    fFoamX->Initialize();                         // Initialize simulator
  }
  TMCFoamParticle* beam = (TMCFoamParticle*)fParticle->FindObject("Beam");
  if( beam != fBeam ) fBeam = beam;
  // Call standard PostInit
  TMCGenerator::PostInit();
}

ClassImp(TMCFoamGenerator)
