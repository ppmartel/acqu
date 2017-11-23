//--Author	JRM Annand   24th May 2005	
//--Rev         JRM Annand   14th Sep 2005  re-do phase-space generators
//--Rev         JRM Annand   24th Apr 2006  non-uniform decay particle dist.
//--Rev         JRM Annand   14th Feb 2007  Gen2Decay mods
//--Rev         JRM Annand   16th Jul 2008  QF from TMCGenerator to Gen2Decay
//--Rev         JRM Annand   25th Jan 2009  Redo constructers for PDG ID
//--Rev         JRM Annand   27th Jan 2009  Feedback QF momentum to beam energy
//--Rev         JRM Annand    5th Feb 2009  Bug fix SetDist( Int_t, Char_t*....
//--Rev         JRM Annand    5th Mar 2009  Bug fix ResetT()
//--Rev 	JRM Annand...10th May 2009  QF without beam-energy compensation
//--Rev 	JRM Annand... 5th Jun 2009  Add GetFirstDecay() Gen1Decay()
//--Rev 	JRM Annand...10th Jun 2009  Fix for BoostLab()
//--Rev 	JRM Annand...27th Jan 2010  Gen2Decay() resonant mass distr.
//--Rev 	JRM Annand...25th Feb 2012  Out-of-plane angle
//--Update 	JRM Annand...27th Feb 2012  Use degrees option

//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data 
//
// TMCParticle
//
// Particle specification and kinematics generation for the Monte Carlo
// event generator TMCGenerator


#include "TMCParticle.h"
#include "TMCGenerator.h"
//#include "SysUtil.h"


//-----------------------------------------------------------------------------
  TMCParticle::TMCParticle( const Char_t* name, TRandom* rand, Double_t mass,
			    Int_t ipdg, Bool_t decay, Bool_t track, Bool_t ud )
  :TNamed( name, "Particle" )
{
  // No PDG data base supplied, read in particle mass

  fRandom = rand;                      // random number generator
  fPDG = NULL;                         // flag no PDG
  fPDGindex = ipdg;
  fMass = mass;                        // store mass
  fIsDecay = decay;                    // stable?
  fIsTrack = track;                    // trackable?
  fUseDegrees = ud;                    // input angles in deg. option
  Init();
}

//-----------------------------------------------------------------------------
TMCParticle::TMCParticle( const Char_t* name, TRandom* rand, TParticlePDG* pdg,
			  Bool_t decay, Bool_t track, Bool_t ud )
  :TNamed( name, "Particle" )
{
  // Use PDG data base, read in PDG index

  fRandom = rand;                           // random-number generator
  fPDG = pdg;                               // PDG data base
  fPDGindex = pdg->PdgCode();               // PDG index
  fMass = pdg->Mass() ;                     // mass from data base
  fIsDecay = decay;                         // stable?
  fIsTrack = track;                         // trackable?
  fUseDegrees = ud;                    // input angles in deg. option
  Init();
}

//-----------------------------------------------------------------------------
TMCParticle::TMCParticle( const Char_t* name, TMCParticle* p )
  :TNamed( name, "Particle" )
{
  // "Copy" constructor, copy particle properties
  // but allow name override

  *this = *p;
  if( name )
    this->SetName(name);
}

//---------------------------------------------------------------------------
TMCParticle::~TMCParticle()
{
  Flush();
}

//-----------------------------------------------------------------------------
void TMCParticle::Flush( )
{
  // Free up allocated memory

  if( fP4 ) delete fP4;
  if( fP4d ) delete fP4d;
  if( fDecayList ) delete fDecayList;
  if( fThetaDist ) delete fThetaDist;
  if( fCosThDist ) delete fCosThDist;
  if( fPhiDist ) delete fPhiDist;
  if( fTDist ) delete fTDist;
  if( fPDist ) delete fPDist;
  if( fOOPDist ) delete fOOPDist;
  if( fMd ) delete fMd;
  if( fMd2 ) delete fMd2;
}

//-----------------------------------------------------------------------------
void TMCParticle::Init( )
{
  // Basic "NULL" initialisation...common to different constructors

  fParent = NULL;
  fDecayList = NULL;
  fThetaDist = NULL;
  fCosThDist = NULL;
  fPhiDist = NULL;
  fTDist = NULL;
  fPDist = NULL;
  fOOPDist = NULL;
  //  fMassQF = 0.0;
  fMd = NULL;
  fMd2 = NULL;
  fP4d = NULL;
  fTcm = 0.0;
  fWt = 0.0;
  fWtMax = 0.0;
  fThetaLow = 0.0;
  fThetaHigh = TMath::Pi();
  fCosThLow = -1.0;
  fCosThHigh = 1.0;
  fPhiLow = 0.0;
  fPhiHigh = TMath::TwoPi();
  fTLow = fTHigh = 0.0;
  fPLow = fPHigh = 0.0;
  fOOPLow = fOOPHigh = 0.0;
  fNd = 0;
  fIsResonance = kFALSE;
  fDecayMode = EMCParticlePhaseSpace;
  fP4 = new TLorentzVector(0.0,0.0,0.0,fMass);   // space for 4 momentum
  fBeam = NULL;                                  // only used for QF
}

//-----------------------------------------------------------------------------
void TMCParticle::PostInit( )
{
  // Construct parameter arrays for decay-particle for N-body phase-space
  // decay generator. Call PostInit for all decay particles

  if( !fNd ) return;                 // no decay particles
  fMd = new Double_t[fNd];           // decay masses
  fMd2 = new Double_t[fNd];          // masses **2
  fP4d = new TLorentzVector*[fNd];   // pointers to decay 4 momenta
  TIter nextp( fDecayList );         // linked-list iterator
  TMCParticle* decay;
  TMCParticle* decay1;
  for( Int_t n=0; n<fNd; n++ ){
    decay = (TMCParticle*)nextp();   // decay product
    fMd[n] = decay->GetMass();       // get its mass,
    fMd2[n] = fMd[n]*fMd[n];         // save mass**2
    fP4d[n] = decay->GetP4();        // ptr to decay 4-momentum
    decay->PostInit();               // Call PostInit for decay product
  }
  // Check for radiative decay of a resonance to itself
  if( (this->InheritsFrom("TMCResonance")) && (fNd == 2) ){
    nextp.Reset();
    decay = (TMCParticle*)nextp();     // 1st decay product
    decay1 = (TMCParticle*)nextp();    // 2nd decay product
    if( fDecayMode == EMCParticleRadRes ){
      if( (decay->GetPDGindex() != kGamma) ||
	  (!decay1->InheritsFrom("TMCResonance")) )
	fDecayMode = EMCParticlePhaseSpace;
    }
    else if( fDecayMode == EMCParticleResRad ){
      if( !(decay->InheritsFrom("TMCResonance")) ||
	  (decay1->GetPDGindex() != kGamma) )
	fDecayMode = EMCParticlePhaseSpace;
    }
  }
}

//----------------------------------------------------------------------------
TH1D** TMCParticle::FindDist( Int_t type, Char_t* name,
			      Double_t min, Double_t max )
{
  // Find pointer to distribution and save min and max values of
  // distribution variable.
  // Distribution variable can be
  // momentum, kinetic energy, polar angle, asimuthal angle
  //
  strcpy( name, this->GetName() );
  switch( type ){
  case EMCParticlePDist:
    // momentum distribution
    strcat( name, "_PDist" );
    if( min != max ){ fPLow = min; fPHigh = max; }
    return &fPDist;
    break;
  case EMCParticleTDist:
    // kinetic energy distribution
    strcat( name, "_TDist" );
    if( min != max ){ fTLow = min; fTHigh = max; }
    return &fTDist;
    break;
  case EMCParticleThetaDist:
    // polar angle distribution
    strcat( name, "_ThetaDist" );
    if( fUseDegrees ){
      min *= TMath::DegToRad();
      max *= TMath::DegToRad();
    }
    if( min != max ){ fThetaLow = min; fThetaHigh = max; }
    return &fThetaDist;
    break;
  case EMCParticleCosThDist:
    // cos polar angle distribution
    strcat( name, "_CosThDist" );
    if( fUseDegrees ){
      Double_t temp = min;
      min = max;
      max = temp;
      min = TMath::Cos(min*TMath::DegToRad());
      max = TMath::Cos(max*TMath::DegToRad());
    }
    if( min != max ){ fCosThLow = min; fCosThHigh = max; }
    return &fCosThDist;
    break;
  case EMCParticlePhiDist:
    // asimuthal angle distribution
    strcat( name, "_PhiDist" );
    if( fUseDegrees ){
      min *= TMath::DegToRad();
      max *= TMath::DegToRad();
    }
    if( min != max ){ fPhiLow = min; fPhiHigh = max; }
    return &fPhiDist;
  case EMCParticleOOPDist:
    // out-of-plane angle distribution
    strcat( name, "_OOPDist" );
    if( fUseDegrees ){
      min *= TMath::DegToRad();
      max *= TMath::DegToRad();
    }
    if( min != max ){ fOOPLow = min; fOOPHigh = max; }
    return &fOOPDist;
    break;
  default:
    return NULL;
  }
}

//----------------------------------------------------------------------------
void TMCParticle::SetDist( Int_t type,
			   Int_t npt, Double_t* xpt, Double_t* wpt,
			   Double_t min, Double_t max ) 
{
  // Load 1D distribution from arrays
  const Char_t* title = this->GetName();
  Char_t name[256];
  TH1D** dhist = FindDist(type, name, min, max);
  if( !npt ){
    dhist = NULL;
    return;
  }
  *dhist = new TH1D( name, title, npt, min, max );
  (*dhist)->FillN( npt, xpt, wpt, 1 );
}

//----------------------------------------------------------------------------
void TMCParticle::SetDist( Int_t type, TF1* func,
			   Int_t npt, Double_t min, Double_t max ) 
{
  // Load 1D distribution from function
  Double_t x = min;
  Double_t dx = (max - min)/(npt - 1);
  Double_t* xpt = new Double_t[npt];
  Double_t* wpt = new Double_t[npt];
  for( Int_t i=0; i<npt; i++ ){
    xpt[i] = x;
    wpt[i] = func->Eval(x);
    x += dx;
  }
  SetDist( type, npt, xpt, wpt, min,max );
  delete xpt;
  delete wpt;
}

//----------------------------------------------------------------------------
void TMCParticle::SetDist( Int_t type, Char_t* func, Double_t* A,
			   Int_t npt, Double_t min, Double_t max ) 
  // Load 1D distribution from a character-string name of a function
{
  if( strcmp( func, "Uniform") == 0 ){
    SetDist( type, 0, (Double_t*)NULL, (Double_t*)NULL, min, max );
    return;
  }
  Char_t dist[256];
  if( strstr(func,"%f") ){
    sprintf( dist, func, A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7] );
  }
  else
    strcpy(dist,func);
  TF1 tdist("temp", dist, min, max );
  SetDist( type, &tdist, npt, min, max );
}

//----------------------------------------------------------------------------
Bool_t TMCParticle::Gen2Decay( ) 
{
  // Fast algorithm for "decay" to 2 particles
  // In the case of the reaction (beam+target) particle the "decay"
  // products are just the reaction products
  // Phase-space or non-uniform angular distributions
  // Specials...

  Double_t M = fP4->M();         // particle mass
  //  Double_t th = fP4->Theta()*TMath::RadToDeg();
  Double_t md,md2,ed,pd;         // mass, energy, momentum decay particle
  ed = 0.0;                      // default energy not sampled
  TIter next( fDecayList );      // list of decay/reaction products
  TMCParticle* d0 = (TMCParticle*)next(); // 1st particle
  TMCParticle* d1 = (TMCParticle*)next(); // 2nd particle
  //  Double_t bx = fBeam->GetP4()->X();
  //  Double_t by = fBeam->GetP4()->Y();
  //  Double_t bz = fBeam->GetP4()->Z();
  //  Double_t be = fBeam->GetP4()->T();

  switch( fDecayMode ){
  default:
    // phase space
    // check if the particle is stable or not
    if( !d0->IsResonance() ) md2 = fMd2[0];
    else {
      md = d0->Mass();
      d0->SetMass(md);
      md2 = md*md;
    }
    break;
  case EMCParticleResRad:
    // Radiative decay of broad resonance R to itself R*
    // Sample mass of R* from Breit-Wigner
    // (or whatever is entered as the mass distribution)
    // and set this as the mass of the daughter resonance
    // R* must be 1st daughter, Gamma' the 2nd
    do { md = d0->Mass(); }while( md >= M ); // sample new mass after decay
    d0->SetMass( md );                       // and store it
    md2 = md*md;
    break;
  case EMCParticleRadRes:
    // Radiative decay of broad resonance to itself
    // Sample energy of G' according to input energy distribution
    // Set mass of R* to M - Eg'
    // G' must be 1st daughter, R* the 2nd
    pd = ed = d0->Energy();
    md = M - ed;
    d1->SetMass( md );
    fMd2[1] = md*md;
    break;

  case EMCParticleQF:
  case EMCParticleQF_NBC:
    // QF interaction of beam with "particle" d0 in nucleus e.g. N or N-N pair
    // spectator remainder is d1
  case EMCParticleQQF:
    // 2nd QF level, e.g. interaction of beam with "parton" d0 in nucleus
    // e.g. single N of interacting N-N pair
    TLorentzVector* QF = d0->GetP4();
    TLorentzVector* rQF = d1->GetP4();
    TLorentzVector* p4bm = fBeam->GetP4();
    *QF = d0->P4();                      // Sample the momentum
    *rQF = -(*QF);                       // & recoil in local cm (=lab) frame
    Double_t m = d1->GetMass();
    Double_t p = rQF->P();
    rQF->SetE(TMath::Sqrt(p*p + m*m));   // Set recoil on-shell mass
    // Standard QF with beam energy compensation
    if( fDecayMode == EMCParticleQF ){
      Double_t eb = p4bm->E();           // beam energy
      Double_t m2qf = QF->M2();          // qf particle Mass**2
      TLorentzVector qf = *fP4 - *rQF;   // = beam + QF particle
      Double_t eb1 = (qf.M2() - m2qf)/(2*TMath::Sqrt(m2qf));// eff beam energy
      TLorentzVector dBm = *p4bm;        // save beam momentum
      fBeam->ResetT(eb - eb1);           // new beam momentum
      dBm = dBm - *p4bm;
      *fP4 = *fP4 - dBm;                 // correct reaction ptcl for new beam
      *QF = *fP4 - *rQF;                 // new qf reaction particle
    }
    // Standard QF without beam energy compensation
    else if( fDecayMode == EMCParticleQF_NBC ){
      *QF = *fP4 - *rQF;                 // new qf reaction particle
    }
    // 2nd level QF
    else {
      *fP4 = *fP4 - *p4bm;            // remove beam component
      BoostLab();                        // QF & Recoil to lab frame
      *QF = *p4bm + *fP4 - *rQF;      // = beam + QF**2 particle
    }
    // Check QF + Beam has a physical invariant mass
    if( QF->M() < d0->GetMass() ) return kFALSE;
    else return kTRUE;
  } 
  // End of QF stuff

  // Anything but QF continues here
  if( !ed ){
    ed = (M*M + md2 - fMd2[1])/(2.0*M);           // E decay product 1
    if( ed < 0.0 ) return kFALSE;
    if( (pd = ed*ed - md2) < 0.0 ) return kFALSE;
    pd = TMath::Sqrt( pd );                       // |p| decay product 1
  }

  // Sample polar and asimuthal angle distribution
  Double_t costh, sinth, phi;                   // CM production angles
  costh = d0->CosTheta();
  sinth = TMath::Sqrt(1.0 - costh*costh);
  phi = d0->Phi();
  // Set components of 4-momenta in CM
  Double_t x = pd * sinth * TMath::Cos( phi );
  Double_t y = pd * sinth * TMath::Sin( phi );
  Double_t z = pd * costh;
  fP4d[0]->SetXYZT(x,y,z,ed);
  fP4d[1]->SetXYZT( -x,-y,-z, TMath::Sqrt( pd*pd + fMd2[1] ) );
  // Boost CM -> Lab. Boost vector from 4-momentum of parent particle
  BoostLab( );
  x = fP4d[0]->Theta()*TMath::RadToDeg();
  x = fP4d[1]->Theta()*TMath::RadToDeg();
  return kTRUE;
}

//----------------------------------------------------------------------------
Bool_t TMCParticle::GenNDecay( ) 
{
  //  Generate an N-Body random final state.
  //  Use Raubold Lynch method ( F.James CERN 68-15 (1968) )
  //  Borrows from ROOT Class TGenPhaseSpace
  //  Check sufficient energy available for the final state
  //  before starting kinematics

  fTcm = fP4->M();
  Int_t n,m;
  for( n=0; n<fNd; n++ ){ fTcm -= fMd[n]; }
  if( fTcm <= 0.0 ) return kFALSE;             // not sufficient energy
  Double_t emmax = fTcm + fMd[0];
  Double_t emmin = 0;
  Double_t wt = 1;
  for (n=1; n<fNd; n++) {
    emmin += fMd[n-1];
    emmax += fMd[n];
    wt *= Pdk(emmax, emmin, fMd[n]);
  }
  fWtMax = 1/wt;                               // max weight for gating events

  Double_t rnd[fNd];
  Double_t invMas[fNd];
  Double_t pd[fNd];
  Int_t irnd[fNd];
  rnd[0] = 0.0; rnd[fNd-1] = 1.0;
  do{
    switch( fNd ){
    default:
      fRandom->RndmArray(fNd-2, rnd+1);         // random values [0,1]
      TMath::Sort(fNd, rnd, irnd, kFALSE);      // sort random val ascending
      break;
    case 3:                                     // 3 decay particles
      rnd[1] = fRandom->Uniform(0.0,1.0);
      irnd[0] = 0; irnd[1] = 1; irnd[2] = 2;
      break;
    case 2:
      irnd[0] = 0; irnd[1] = 1;
      break;
    }
    wt = 0.0;
    for (n=0; n<fNd; n++) {
      wt += fMd[n];
      invMas[n] = rnd[irnd[n]]*fTcm + wt;
    }
    wt = fWtMax;
    for (n=0; n<fNd-1; n++) {
      pd[n] = Pdk(invMas[n+1],invMas[n],fMd[n+1]);
      wt *= pd[n];
    }
  }while( fRandom->Uniform(0.0,fWtMax) > wt );
  fWt = wt;
  //
  // Specification of 4-momenta (Raubold-Lynch method)
  //
  fP4d[0]->SetPxPyPzE(0, pd[0], 0 , TMath::Sqrt(pd[0]*pd[0]+fMd[0]*fMd[0]) );
  for(n=1;;){
    fP4d[n]->SetPxPyPzE(0, -pd[n-1], 0 , 
		      TMath::Sqrt(pd[n-1]*pd[n-1]+fMd[n]*fMd[n]) );

    Double_t cosZ   = fRandom->Uniform(-1.,1.);
    Double_t angY = fRandom->Uniform(0.0, TMath::TwoPi());;
    for (m=0; m<=n; m++) {
      fP4d[m]->RotateZ( TMath::ACos(cosZ) );
      fP4d[m]->RotateY( angY );
    }
    if( n == fNd-1 ) break;
    Double_t beta = pd[n] / sqrt(pd[n]*pd[n] + invMas[n]*invMas[n]);
    for (m=0; m<=n; m++) fP4d[m]->Boost(0,beta,0);
    n++;
  }
  // Boost CM -> Lab
  BoostLab( );
  return kTRUE;
}

//----------------------------------------------------------------------------
void TMCParticle::AddDecay( TMCParticle* decay )
{
  // add particle to list of decay particles

  decay->SetParent( this );
  if( !fDecayList ) fDecayList = new TList();
  fDecayList->AddLast( decay );
  fNd++;
}

//----------------------------------------------------------------------------
TMCParticle*  TMCParticle::ReplaceDecay( TMCParticle* decay )
{
  // replace particle with same-name particle decay 
  // in list of decay particles

  Char_t* name = (Char_t*)decay->GetName();
  TMCParticle* p = (TMCParticle*)fDecayList->FindObject( name );
  if( !p ) return NULL;
  fDecayList->AddBefore( p, decay );
  fDecayList->Remove( p );
  p = (TMCParticle*)fDecayList->FindObject( name );
  return p;
}
ClassImp(TMCParticle)
