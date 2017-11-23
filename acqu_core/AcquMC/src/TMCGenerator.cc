//--Author	JRM Annand   24th May 2005	      
//--Rev         JRM Annand   18th Oct 2005  Debug hbook ntuple output    
//--Rev         JRM Annand   26th Apr 2006  Add options    
//--Rev         JRM Annand   11th May 2006  TH1D randoms, m runs of n events   
//--Rev 	JRM Annand...21st Jan 2007  4v0 update, fix beam
//--Rev 	JRM Annand...16st Apr 2008  Add TMCdS5MDMParticle
//--Rev 	JRM Annand... 3rd Jun 2008...const Char_t*...gcc 4.3
//--Rev 	JRM Annand... 3rd Jul 2008  Recoil particle for QF
//--Rev 	JRM Annand...16th Jul 2008  Move QF back to TMCParticle
//--Rev 	JRM Annand...25th Jan 2009  GetPDG_ID
//--Rev 	JRM Annand...27th Jan 2009  Give particles fBeam not fP4Beam
//--Rev 	DI Glazier...12th Feb 2009  Add InitTreeParticles SaveEventAsParticles
//--Rev 	JRM Annand...10th May 2009  QF without beam-energy compensation
//--Rev 	JRM Annand... 5th Jun 2009  Mods for beam-parent particles
//--Rev 	JRM Annand...20th Jul 2009  Beam primed...check energy
//--Rev 	JRM Annand... 1st Sep 2009  delete[]
//--Rev  	JRM Annand...25th Feb 2012  Out-of-plane angle
//--Rev  	JRM Annand...27th Feb 2012  Use-degrees option, and Theta-dist
//--Update 	JRM Annand...26th Apr 2013  Bug fix InitParticleDist()

//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCGenerator
//
// Monte Carlo Kinematics Generator
// Beam + Target -> Reaction -> Products
// Store
// 1) Interaction vertex in target (x,y,z)
// 2) Beam 4-momentum
// 3) 4-momenta of reaction products
// Output as ROOT tree (decodable by macro MCReplay.C)
// OR as HBOOK ntuple, in identical format to mkin for input
// to CB/TAPS GEANT-3 model cbsim

#include "TMCGenerator.h"
#include "MCNtuple.h"
#include "ARFile_t.h"
#include "TParticle.h"
#include "TBenchmark.h"
#include "TLeaf.h"
#include "CMakeConfig.h"



// Main command options
static const Map_t kMCcmd[] = {
  {"Number-Throws:",        EMCNThrow},
  {"Beam:",                 EMCBeam},
  {"Target:",               EMCTarget},
  {"Particle:",             EMCParticle},
  {"Use-Degrees:",          EMCUseDegrees},
  {"P-Distr:",              EMCParticleDist},
  {"Resonance:",            EMCResonance},
  {"Tree-Output:",          EMCTreeOut},
  {"Ntuple-Output:",        EMCNtupleOut},
  {"Particle-Output:",      EMCPartOut},
  {NULL,                    -1}
};

// decay-particle distributions
// default is isotropic
static const Map_t kMCDistOpt[] = {
  {"P-Dist",                EMCParticlePDist},
  {"T-Dist",                EMCParticleTDist},
  {"Theta-Dist",            EMCParticleThetaDist},
  {"Phi-Dist",              EMCParticlePhiDist},
  {"Cos(2*Phi)",            EMCParticleCos2Phi},
  {"Sin**2(Theta)",         EMCParticleSinSqTheta},
  {"Cos(Theta)",            EMCParticleCosThDist},
  {"OOP-Dist",              EMCParticleOOPDist},
  {NULL,                    -1}
};


static const Map_t kMCDecayOpt[] = {
  {"Rad-Res",               EMCParticleRadRes},
  {"Res-Rad",               EMCParticleResRad},
  {"Quasi-Free",            EMCParticleQF},
  {"Q-Quasi-Free",          EMCParticleQQF},
  {"Phase-Space",           EMCParticlePhaseSpace},
  {"Quasi-Free-NBC",        EMCParticleQF_NBC},
  {NULL,                    -1}
};


//-----------------------------------------------------------------------------
TMCGenerator::TMCGenerator( const Char_t* name, Int_t seed )
  :TA2System( name, kMCcmd  )
{
  // Basic initialisation of MC generator
  // Initialise random-number generator and PDG database
  fSeed = seed;
  fNrun = 1;
  fRand = new TRandom( seed );
  fPDG = new TDatabasePDG();
  char* dbname = BuildName(ENV_OR_CMAKE("ROOTSYS", CMAKE_ROOTSYS), "/etc/pdg_table.txt" );
  fPDG->ReadPDGTable(dbname);
  delete[] dbname;
  fTargetMass = fTargetRadius = fTargetLength = 0.0;
  fTargetCentre = fVertex = fBeamCentre = NULL;
  fP4Target = fP4Beam = NULL;
  fBeam = fTarget = fReaction = NULL;
  fNThrow = fNParticle = 0;
  fResonance = new TList();
  fParticle = new TList();
  fReactionList = new TList();
  fTreeFileName = fNtpFileName = NULL;
  fTreeFile = fNtpFile = NULL;
  fTree = NULL;
  fBranch = NULL;
  fEvent = NULL;
  fClonePtcl=NULL;
  fIsTreeOut = fIsNtupleOut = fIsPartOut = fIsError = kFALSE;
  Char_t* logfile = BuildName("AcquMC.log");
  SetLogFile(logfile);
  delete[] logfile;
  fUseDegrees = kFALSE;
}

//---------------------------------------------------------------------------
TMCGenerator::~TMCGenerator()
{
  Flush();
}

//---------------------------------------------------------------------------
void TMCGenerator::Flush()
{
  // Free up allocated memory
  if( fRand ) delete fRand;
  if( fPDG ) delete fPDG;
  if( fTargetCentre ) delete fTargetCentre;
  if( fVertex ) delete fVertex;
  if( fBeamCentre ) delete fBeamCentre;
  if( fP4Target ) delete fP4Target;
  if( fP4Beam ) delete fP4Beam;
  if( fBeam ) delete fBeam;
  if( fReaction ) delete fReaction;
  if( fParticle ){
    TIter next(fParticle);
    TMCParticle* p;
    while( (p = (TMCParticle*)next()) ) delete p;
    delete fParticle;
  }
  if( fReactionList ) delete fReactionList;
  if( fResonance ){
    TIter next(fResonance);
    TMCResonance* p;
    while( (p = (TMCResonance*)next()) ) delete p;
    delete fResonance;
  }
  if( fIsTreeOut ){
    delete fBranch[EBranchPID]; delete fBranch[EBranchP4];
    delete (Int_t*)fEvent[EBranchPID]; delete (Double_t*)fEvent[EBranchP4];
  }
  if( fIsNtupleOut ){
    delete fBranch[EBranchPID]; delete (Float_t*)fEvent[EBranchPID];
  }    
  if( fEvent ) delete fEvent;
  if( fBranch ) delete fBranch;
  if( fTreeFileName ) delete[] fTreeFileName;
  if( fNtpFileName ) delete[] fNtpFileName;
  if( fTree ) delete fTree;
  if( fNtuple ) delete fNtuple;
  if( fTreeFile ) delete fTreeFile;
  if( fNtpFile ) delete fNtpFile;
  if( fClonePtcl) fClonePtcl->Delete();
}

//-----------------------------------------------------------------------------
void TMCGenerator::SetConfig(Char_t* line, Int_t key)
{
  // Parse a line of configuration information.
  // Recognised command strings are listed in Map_t kMCcmd[] array above

  Char_t name[256];
  Char_t opt[256];
  //  Char_t* file;
  //  Double_t dop[16];
  Int_t iopt;
  Double_t width, low, high;
  TMCResonance* res;
  //  TMCFoamParticle* foam;
  TMCParticle* particle;

  switch (key){
  case EMCNThrow:
    // # throws, seed for random-number generator and # runs
    if( sscanf(line,"%d%d%d",&fNThrow, &fSeed, &fNrun ) < 1 ){
      PrintError(line,"<MCGenerator # throws parse>",EErrFatal);
      return;
    }
    break;
  case EMCBeam:
    // Beam Parameters
    ParseBeam(line);
    break;
  case EMCTarget:
    // Target parameters
    ParseTarget( line );
    break;
  case EMCParticle:
    // Reaction-product-particle parameters
    ParseParticle( line );
    break;
  case EMCParticleDist:
    // Enter distribution function into particle class
    // Kinetic energy, momentum, polar angle, asymuthal angle
    if( sscanf( line, "%s%s", name, opt ) != 2 )
      PrintError( line,"<Particle Distribution parse error>",EErrFatal );
    if(  !(particle = (TMCParticle*)(fParticle->FindObject( name ))) )
      PrintError( line,"<Particle Distribution: unknown particle>",EErrFatal );
    iopt = Map2Key( opt, kMCDistOpt );
    line = strstr( line, opt ) + strlen(opt);
    InitParticleDist( line, particle, iopt );
    break;
  case EMCResonance:
    // Baron-resonances...large mass uncertainty
    // These have a mass distribution (BreitWigner) as opposed to fixed value
    // No restriction on mass distribution function form
    low = high = width = 0.0;
    if( sscanf(line,"%s%s%lf%lf%lf", name, opt, &width, &low, &high ) >= 2 ){
      particle = GetPDG_ID( name, opt );
      if( !particle )PrintError( opt, "<Unknown resonant particle ID>",
				 EErrFatal);
      res = new TMCResonance( name, particle, width, low, high );
      //      delete particle;
    }
    else
      PrintError(line,"<MCGenerator resonance config parse>", EErrFatal);
    fResonance->AddLast( res );
    break;
  case EMCTreeOut:
    // Generated events strored on disk in ROOT TTree output
    // Beam + vertex + TLorentzVector(s)
    if( sscanf(line, "%s", name) != 1 )
      PrintError(line,"<Tree file name parse>");
    fTreeFileName = BuildName( name );
    fIsTreeOut = kTRUE;
    break;
  case EMCPartOut:
    // Generated events strored on disk in ROOT TTree output
    // Beam + vertex + TParticle(s)
    if( sscanf(line, "%s", name) != 1 )
      PrintError(line,"<Tree file name parse>");
    fTreeFileName = BuildName( name );
    fIsPartOut = kTRUE;
    break;
  case EMCNtupleOut:
    // HBOOK Column-wise ntuple output.
    // This produces an event file identical in format to that
    // generated by mkin. Thus compatible with GEANT-3 CB-TAPS model cbsim
    if( sscanf(line, "%s", name) != 1 )
      PrintError(line,"<Ntuple file name parse>");
    fNtpFileName = BuildName( name );
    fIsNtupleOut = kTRUE;                 // Flag hbook ntuple output
    break;
  case EMCUseDegrees:
    // Input angles in degrees (also for cos(theta)
    fUseDegrees = kTRUE;
    break;
  default:
    PrintError( line, "<Unrecognised command>" );
    break;
  }
}

//-----------------------------------------------------------------------------
void TMCGenerator::PostInit( )
{
  // Finish initialisation
  // Ensure all initialisation of particle classes complete before MC loop
  fReaction->PostInit();
  fprintf( fLogStream, "Reaction Particle: %s\nPartcle List:\n",
	   fReaction->GetName());
  TIter next( fParticle );
  TMCParticle* p;
  while( (p = (TMCParticle*)next()) ){
    fprintf( fLogStream, "Listed Particle: %s\n", p->GetName() );
  }
}

//-----------------------------------------------------------------------------
void TMCGenerator::InitTreeParticles( Char_t* name, Int_t run )
{
  // Save output in the TParticle class. The TTree of output data contains
  // a TClonesArray of TParticle
  
  if(fIsTreeOut){
    printf("Using original Tree output not TParticle.\n\
 To use TParticle please diasble Tree-Output:");
    PrintError(name,"<TParticle output disabled,overridden by standard TTree>");
    fIsPartOut=false;
    return;
  }

  // Set up the TClonesArray
  TMCParticle* particle;
  fClonePtcl = new TClonesArray("TParticle"); 
  TIter next( fParticle );
  TParticle* ptcl;

 //Now make entries for other particles to be tracked
  Int_t np = 0;
  while( ( particle = (TMCParticle*)next() ) ){
    if( (particle->IsTrack()) || (particle->GetName() == TString("Beam")) ){
      new( (*fClonePtcl)[np] ) TParticle(); 
      ptcl = (TParticle*)(fClonePtcl->At(np));
      ptcl->SetPdgCode(particle->GetPDGindex());
      np++;
    }
  }
  // Setup the name of the output file
  Char_t* fileName;
  if( fNrun > 1 ){
    Char_t r[16];
    sprintf( r, "_%d", run );
    fileName = BuildName( name, r, ".root" );
  }
  else fileName = BuildName( name, ".root" );
  // Create output root file, output tree and branches
  fTreeFile = new TFile(fileName,"RECREATE","MC_Event_File");
  delete[] fileName;
  fTree = new TTree("TParticles","MC-Generator");

  Int_t split=0;
  fTree->Branch("Particles",&fClonePtcl,128000,split);
  //  arr.BypassStreamer();
  for(Int_t i=0; i<np; i++){
    ptcl = (TParticle*)(fClonePtcl->At(i));
    printf("PDG = %d\n",ptcl->GetPdgCode());
  }

}

//-----------------------------------------------------------------------------
void TMCGenerator::InitTree( Char_t* name, Int_t run )
{
  // Initialise ROOT Tree output
  // Tree name is "Reaction_Kinematics"
  // Branch Names
  // Constants: static values...dimensions etc.
  // 4-momenta
  // PDG-indices

  // Data buffer and output-format setup
  Char_t* bname[] = { (Char_t*)"Setup", (Char_t*)"P4lab" };
  Char_t f1[256], f2[2048];
  sprintf( f1, "nParticle/I:IsTrack[%d]/I:PDGindex[%d]/I:",
	   fNParticle+1, fNParticle+1 );
  Char_t* format[] = { f1, f2 };

  // Create output storage buffers, if not previously done
  if( !fEvent ){
    fEvent = new void*[ENBranch];
    fEvent[0] = new Int_t[2*(fNParticle+1) + 1];
    fEvent[1] = new Double_t[4*(fNParticle+2) + 3];
  }

  // Particle properties and names of leaves
  Int_t* sta = (Int_t*)fEvent[0];
  *sta++ = fNParticle;                                 // # particles
  Char_t* pname;
  Char_t leafname[256];
  TIter next( fParticle );
  TMCParticle* particle;
  // Vertex and Beam vector component names
  sprintf(f2,"Vx/D:Vy/D:Vz/D:Bm_Px/D:Bm_Py/D:Bm_Pz/D:Bm_E/D:");
  while( ( particle = (TMCParticle*)next() ) ){
    *sta = (Int_t)particle->IsTrack();                // particle track ?
    *(sta+fNParticle) = particle->GetPDGindex();      // particle PDG index
    sta++;
    pname = (Char_t*)particle->GetName();
    if( !particle->IsTrack() ) continue;
    // Particle 4 momenta component names
    sprintf(leafname,"%s_Px/D:%s_Py/D:%s_Pz/D:%s_E/D:",
	    pname,pname,pname,pname);
    strcat(f2,leafname);
  }
  // remove final ':'
  Char_t* end = strrchr( f1,':' );
  *end = '\0';
  // remove final ':'
  end = strrchr( f2,':' );
  *end = '\0';
  // Setup the name of the output file
  Char_t* fileName;
  if( fNrun > 1 ){
    Char_t r[16];
    sprintf( r, "_%d", run );
    fileName = BuildName( name, r, ".root" );
  }
  else fileName = BuildName( name, ".root" );
  // Create output root file, output tree and branches
  fTreeFile = new TFile(fileName,"RECREATE","MC_Event_File");
  delete[] fileName;
  // Create Tree and branches
  fTree = new TTree("Reaction_Kinematics", this->ClassName());
  fBranch = new TBranch*[ENBranch];
  for( Int_t i=0; i<ENBranch; i++ ){
    fTree->Branch(bname[i],fEvent[i],format[i]);  // Map branch to data mem
    fBranch[i] = fTree->GetBranch(bname[i]);
  }
  fBranch[0]->Fill();                             // Static information
}


//-----------------------------------------------------------------------------
void TMCGenerator::SaveEventAsParticles(  )
{
  // Save Event in TParticle format if this is enabled at configuration
  // if not Check if HBOOK ntuple output enabled
 
  if( fIsNtupleOut ) SaveNtuple();
  if( !fIsPartOut ) return;
  if( fIsTreeOut ) return;

  TMCParticle* particle;
  TIter next( fParticle );
  TLorentzVector* P4;
  Double_t vx = fVertex->X();
  Double_t vy = fVertex->Y();
  Double_t vz = fVertex->Z();
  Int_t np = 0;
  TParticle *ptcl;
  TClonesArray &clP = *fClonePtcl;
  ptcl = (TParticle*)clP.At(np);
  ptcl->SetMomentum(fP4Beam->X(),fP4Beam->Y(),fP4Beam->Z(),fP4Beam->E());
  ptcl->SetProductionVertex(fBeamCentre->X(),fBeamCentre->Y(),
			    fBeamCentre->Z(),0);
  np++;
  while( (particle = (TMCParticle*)next()) ){
    if( particle->IsTrack() ){
      P4 = particle->GetP4();
      ptcl = (TParticle*)(fClonePtcl->At(np));
      ptcl->SetMomentum(P4->X(), P4->Y(), P4->Z(), P4->E());
      ptcl->SetProductionVertex(vx,vy,vz,0);
      np++;
    }
  }
  fTree->Fill();
}

//-----------------------------------------------------------------------------
void TMCGenerator::SaveEvent(  )
{
  // Save Event in ROOT format if this is enabled at configuration
  // if not Check if HBOOK ntuple output enabled
  // Vertex and 4-momenta components saved as doubles

  if( fIsNtupleOut ) SaveNtuple();
  if( fIsPartOut ) SaveEventAsParticles();
  if( !fIsTreeOut ) return;

  TMCParticle* particle;
  TIter next( fParticle );
  Double_t* p4 = (Double_t*)fEvent[1];
  TLorentzVector* P4;
  //
  // Save vertex in target and beam 4 momentum
  *p4++ = fVertex->X(); *p4++ = fVertex->Y(); *p4++ = fVertex->Z();
  *p4++ = fP4Beam->X(); *p4++ = fP4Beam->Y();
  *p4++ = fP4Beam->Z(); *p4++ = fP4Beam->E();
  //
  // Save particle 4 momenta
  while( (particle = (TMCParticle*)next()) ){
    if( particle->IsTrack() ){
      P4 = particle->GetP4();
      *p4++ = P4->X(); *p4++ = P4->Y(); *p4++ = P4->Z(); *p4++ = P4->E();
    }
  }
  fBranch[1]->Fill();
}

//-----------------------------------------------------------------------------
void TMCGenerator::CloseEvent(  )
{
  // Print out some Tree statistics
  // and close tree file

  if( fIsNtupleOut ) CloseNtuple();
  if( (!fIsTreeOut)&&(!fIsPartOut) ) return;
  fTree->Print();                  // Print summary of Tree
  fTreeFile->Write();              // Save tree to file
  fTree->Delete();                 // Delete the tree
  fTreeFile->Close();              // close the tree file
  fTree = NULL;
  fTreeFile = NULL;
}

//-----------------------------------------------------------------------------
void TMCGenerator::ParseBeam( Char_t* line )
{
  // Extract beam information from input line
  // Basic input...
  // Mass (or PDG id) source-position

  Char_t rname[32];
  Double_t x,y,z;

  if( sscanf(line,"%s%lf%lf%lf",rname, &x, &y, &z ) != 4 )
    PrintError(line,"<Beam configuration parse>", EErrFatal);
  fBeam = GetPDG_ID( (Char_t*)"Beam", rname );
  if( !fBeam )PrintError( rname, "<Unknown beam particle ID>", EErrFatal);
  fBeamCentre = new TVector3( x,y,z );
  fP4Beam = fBeam->GetP4();
  fParticle->AddLast( fBeam );
  fNParticle++;
}

//-----------------------------------------------------------------------------
void TMCGenerator::ParseTarget( Char_t* line )
{
  // Extract target information from input line
  // Target mass (GeV), Target radius (cm), Target length (cm)
  // Target centre coordinates x,y,z (cm)

  Double_t x,y,z;
  Char_t rname[32];
  if( sscanf(line,"%s%lf%lf%lf%lf%lf",
	     rname, &fTargetRadius,
	     &fTargetLength, &x, &y, &z) < 6 ){
    PrintError(line,"<MCGenerator target config parse>");
    return;
  }
  fTarget = GetPDG_ID( (Char_t*)"Target", rname );
  if( !fTarget )PrintError( rname, "<Unknown target particle ID>", EErrFatal);
  fTargetMass = fTarget->GetMass();
  if( !fTargetCentre ){
    fTargetCentre = new TVector3( x,y,z );
    //    fP4Target = new TLorentzVector( 0.0, 0.0, 0.0, fTargetMass );
    fP4Target = fTarget->GetP4();
    fVertex = new TVector3( 0.0,0.0,0.0 );
  }
}

//-----------------------------------------------------------------------------
void TMCGenerator::ParseParticle( Char_t* line )
{
  // Extract particle information from input line
  // Optional Parameters...
  // Mass of nuclear constituent (nucleon) involved in quasi-free process
  // Momentum distribution of constituent...
  //    Read either from named file...x[i], f(x[i])
  //    Or constructed from "function string"
  // Min, max values of distribution
  // Parameters of function (only if function string supplied)

  Char_t name[256];
  Char_t rname[256];
  Char_t pname[256];
  Char_t popt[256];
  //  Double_t opt[8];
  //  Int_t ipdg;
  Int_t iopt;
  //  Double_t mass;
  Int_t isDecay, isTrack;
  TMCParticle* particle;
  TMCParticle* parent;

  // Scan in basic parameters from input line
  if( sscanf(line,"%s%s%d%d%s%s",name, rname, &isDecay, &isTrack,
		  pname, popt ) < 6 )
    PrintError(line,"<Particle configuration parse>", EErrFatal);

  // Decide how to treat particle
  if( (particle = (TMCParticle*)fParticle->FindObject(name)) ){
    particle->SetDecay(isDecay);
    particle->SetTrack(isTrack);
  }
  else if( (particle = (TMCParticle*)fResonance->FindObject(rname)) ){
    particle = new TMCResonance( name, particle );
    particle->SetDecay(isDecay);
    particle->SetTrack(isTrack);
    fParticle->AddLast( particle );
  }
  else {
    particle = GetPDG_ID( name, rname, isDecay, isTrack );
    if( !particle )PrintError( rname, "<Unknown particle ID>", EErrFatal);
    fParticle->AddLast( particle );
  }
  // Particle associated with beam...eg. e' in electron scattering
  if( !strcmp("Beam",pname) ){
    fBeam->AddDecay( particle );
    fBeam->SetDecay(1);
  }
  // Otherwise its a reaction product
  else{
    fReactionList->AddLast( particle );
    // If there is no parent particle, then this one is the "reaction particle"
    // with beam+target 4-momentum, otherwise its a reaction product
    if( ( parent = (TMCParticle*)fReactionList->FindObject(pname) ) == NULL ){
      if(!fReaction) fReaction = particle;
      else PrintError(line,"<No parent found, but reaction particle defined>",
		      EErrFatal);
    }
    else{ parent->AddDecay( particle );}
  }
  fNParticle++;
  line = strstr( line, popt );
  iopt = Map2Key( popt, kMCDecayOpt );
  particle->SetDecayMode(iopt);
  particle->SetBeam( fBeam );

}

//-----------------------------------------------------------------------------
void TMCGenerator::InitParticleDist( Char_t* line, TMCParticle* particle,
				    Int_t iopt )
{
  // Parse entry of distribution info
  // Either from a file containing pairs of x,f(x) values
  //   file name and min,max values of TH1D containing distribution
  // Or from a string specifying a function
  //   Enter function string, #pts to generate, min, max values of TH1D
  //   containing distribution, and any parameters relating to the distribution
  // Use SetDist() method of TMCParticle

  Double_t min, max;             // min, max values of distribution (TH1D)
  Double_t A[8];                 // optional parameters for input function
  Char_t dist[256];              // file name or distribution string
  Int_t npt;                     // # points at which distribution stored
  Int_t n = sscanf( line, "%s%lf%lf%d%lf%lf%lf%lf%lf%lf%lf%lf",
		    dist, &min, &max, &npt, A,A+1,A+2,A+3,A+4,A+5,A+6,A+7 );
  // Distribution from file
  // Open and read file line by line
  if( n == 3 ){
    Char_t* distfile;
    Char_t* dline;
    if( strchr( dist, '/' ) ) distfile = BuildName( dist );
    else distfile = BuildName(ENV_OR_CMAKE("acqu",CMAKE_ACQU_USER), "/data/", dist);
    ARFile_t distf( distfile, "r", this );
    delete[] distfile;
    Double_t xpt[8192];          // array of x values
    Double_t wpt[8192];          // array of f(x) values
    npt = 0;
    while( (dline = distf.ReadLine()) ){
      if( sscanf( dline,"%lf%lf",xpt+npt,wpt+npt ) == 2 ) npt++;
    }
    particle->SetDist( iopt, npt, xpt, wpt, min, max );
  }
  // Distribution from string
  else if( n >= 4 ){
    particle->SetDist( iopt, dist, A, npt, min, max );
  }
  // Parse error...fatal
  else
    PrintError(line,"<Particle distribution parse>",EErrFatal);
}
 
//-----------------------------------------------------------------------------
void TMCGenerator::Generate( )
{
  // Loop to generate beam on target
  // and then produce all product partices of the reaction
  //
  TBenchmark *bmark=new TBenchmark();
  bmark->Start("AcquMC");

  Int_t m,n;
  // Loop over #runs (each of fNThrow events)...default = 1
  for( m=0; m<fNrun; m++ ){
    // Initialise output file
    if( fIsTreeOut ) InitTree( fTreeFileName, m );
    if( fIsNtupleOut ) InitNtuple( fNtpFileName, m );
    if( fIsPartOut ) InitTreeParticles( fTreeFileName, m );
    // Loop over # throws of the dice...ie # events generated
    for( n=0; n<fNThrow; n++ ) GEvent();
    CloseEvent();
  }
  bmark->Stop("AcquMC");
  bmark->Show("AcquMC");
  delete bmark;

}

//-----------------------------------------------------------------------------
void TMCGenerator::InitNtuple( Char_t* name, Int_t run )
{
  // Open TTree file and create ntuple which mimics mkin format
  // Ntuple variables are...
  // Vertex x,y,z
  // Beam Px,Py,Pz,Ptot,Etot
  // Particle Px,Py,Pz,Ptot,Etot....for all particles stored

  TMCParticle* particle;
  TIter next( fParticle );
  Char_t* temp;
  Char_t ntpNames[1024];   // names of ntuple variables
  Int_t nvar = 8;          // vertex x,y,z, beam Px,Py,Pz,Ptot,E
  //  Int_t lrecl = 1024;      // hbook record length
  Int_t ig3;               // Geant-3 particle ID
  Int_t i = 0;

  if( !fEvent ) fEvent = new void*[1];
  fEvent[0] = new Float_t[4096];

  strcpy( ntpNames,
	  "X_vtx:Y_vtx:Z_vtx:Px_bm:Py_bm:Pz_bm:Pt_bm:En_bm" );
  while( (particle = (TMCParticle*)next()) ){
    if( !particle->IsTrack() ) continue;
    i++;
    temp = ntpNames + strlen(ntpNames);
    ig3 = PDGtoG3( particle->GetPDGindex() );
    sprintf(temp,
	   ":Px_l%02d%02d:Py_l%02d%02d:Pz_l%02d%02d:Pt_l%02d%02d:En_l%02d%02d",
	    i, ig3, i, ig3, i, ig3, i, ig3, i, ig3 ); 
    nvar += 5;                                // particle Px,Py,Pz,Ptot,E
  }
  Char_t* fileName;
  if( fNrun > 1 ){
    Char_t r[16];
    sprintf( r, "_%d", run );
    fileName = BuildName( name, r,".root" );
  }
  else fileName = BuildName( name, ".root" );
  fNtpFile = new TFile(fileName, "RECREATE", "MC_Ntuple_File");
  fNtuple = new TNtuple("h1", this->ClassName(), ntpNames);
  return;

}

//-----------------------------------------------------------------------------
void TMCGenerator::SaveNtuple()
{
  // Save output as an ntuple, mimicing mkin format
  // Vertex coordinates
  // x, y, z
  // Beam 4 momentum (Pt = magnitude of momentum, E = total energy)
  // Px/Pt. Py/Pt, Pz/Pt, Pt, E
  // Each "tracked" particle...
  // Px/Pt. Py/Pt, Pz/Pt, Pt, E
  // This format is recognised by the GEANT3 model of CB/TAPS cbsim

  TMCParticle* particle;              // generated particles
  TIter next( fParticle );
  Float_t* p4 = (Float_t*)fEvent[0];  // output buffer
  Double_t ptot;                      // total momentum

  // Save vertex
  *p4++ = fVertex->X(); *p4++ = fVertex->Y(); *p4++ = fVertex->Z();
  // Save beam parameters
  TLorentzVector* P4 = fP4Beam;
  ptot = (P4->Vect()).Mag();
  *p4++ = P4->X()/ptot; *p4++ = P4->Y()/ptot; *p4++ = P4->Z()/ptot;
  *p4++ = ptot;
  *p4++ = P4->E();
  // Save particles which are marked "track"
  while( (particle = (TMCParticle*)next()) ){
    if( !particle->IsTrack() ) continue;
    P4 = particle->GetP4();
    ptot = (P4->Vect()).Mag();
    *p4++ = P4->X()/ptot; *p4++ = P4->Y()/ptot; *p4++ = P4->Z()/ptot;
    *p4++ = ptot;
    *p4++ = P4->E();
  }
  // Write the event to the ntuple file
  fNtuple->Fill((Float_t*)fEvent[0]);
}

//-----------------------------------------------------------------------------
void TMCGenerator::CloseNtuple(  )
{
  // Ensure ntuple data flushed to TTree file
  // Print ntuple statistics and close TTree file
  //
  fNtpFile->Write();
  fNtuple->Print();
  fNtpFile->Close();
}

//-----------------------------------------------------------------------------
TMCParticle* TMCGenerator::GetPDG_ID( Char_t* name, Char_t* rname,
				      Int_t isDecay, Int_t isTrack )
{
  // Look for particle of nucleus in data base
  // particles use root data base
  // nuclei use tables in MCNtuple.h
  //
  TMCParticle* p = NULL;
  TParticlePDG* pdg;
  Double_t mass;
  Int_t ipdg;

  // Deconstruct the string rname
  // Detect a floating point '.' assume its a mass
  // No PDG code available
  if( strchr( rname, '.') ){
    if( (sscanf(rname,"%lf", &mass) == 1) )
      p = new TMCParticle(name, fRand, mass, 0, isDecay, isTrack, fUseDegrees);
  }
  // Detect an integer....interpret as PDG code
  else if( (sscanf(rname,"%d",&ipdg) == 1) ){
    if( (pdg = fPDG->GetParticle(ipdg)) )
      p = new TMCParticle( name, fRand, pdg, isDecay, isTrack, fUseDegrees );
    else if( ipdg > ENullPDG ){
      mass = GetIonMass( ipdg );
      if( mass > 0.0 )
	p = new TMCParticle( name, fRand, mass, ipdg, isDecay, isTrack,
			     fUseDegrees );
    }
  }
  // Otherwise its a character string....a name
  else{
    if( (pdg = fPDG->GetParticle(rname)) )
      p = new TMCParticle( rname, fRand, pdg, isDecay, isTrack, fUseDegrees );
    else {
      mass = GetIonMass( rname );
      if( mass > 0.0 ){
	ipdg = GetIonPDG( rname );
	p = new TMCParticle( rname, fRand, mass, ipdg, isDecay, isTrack,
			     fUseDegrees );
      }
    }
  }
  return p;
}

ClassImp(TMCGenerator)
