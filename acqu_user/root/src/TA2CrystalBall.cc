//--Author	JRM Annand   20th Dec 2002
//--Rev 	JRM Annand...26th Feb 2003...1st production version
//--Rev 	JRM Annand... 1st Oct 2003...Incorp. TA2DataManager
//--Rev 	JRM Annand...12th Dec 2003...fMulti 2D plots
//--Rev 	JRM Annand...19th Feb 2004...User code
//--Rev   	E Downie.....16th Jan 2005...Clean up PID mods
//--Rev 	E Downie.....20th Jan 2005...Add MWPC track getting
//--Rev 	E Downie.....20th Feb 2005...Energy sum over PID joins,clean up
//--Rev 	JRM Annand...10th Nov 2006...Check MWPC defined in reconstruct
//--Update	JRM Annand....1st Feb 2007...OR dE-E variable for common cut
//--Description
//                *** Acqu++ <-> Root ***
// Online/Offline Analysis of Sub-Atomic Physics Experimental Data
//
// TA2CrystalBall
//
// User coded version of Crystal Ball apparatus.  Includes identification
// of charged particles, but no method for differentiation between g and n.

#include "TA2CrystalBall.h"

// Default list of detector classes that the TA2CrystalBall
// apparatus may contain
enum { ECB_PlasticPID, ECB_CalArray, ECB_CylMWPC };
static Map_t kValidDetectors[] = {
  {"TA2PlasticPID",     ECB_PlasticPID},
  {"TA2CalArray",       ECB_CalArray},
  {"TA2CylMWPC",        ECB_CylMWPC},
  {NULL, 		-1}
};

enum {ECB_Photon, ECB_Pi0, ECB_PiPlus, ECB_PiMinus, ECB_Proton, ECB_Neutron};

// Valid Keywords for command-line setup of CB apparatus
enum { ECBAngleLimits = 1000, ECBParticleCuts };
static const Map_t kCBKeys[] = {
  {"AngleLimits:",    ECBAngleLimits},
  {"ParticleID-Cut:", ECBParticleCuts},
  {NULL,            -1}
};

ClassImp(TA2CrystalBall)

//-----------------------------------------------------------------------------

TA2CrystalBall::TA2CrystalBall( const char* name, TA2System* analysis  )
               :TA2Apparatus( name, analysis, kValidDetectors )
{
  // Zero pointers and counters, add local SetConfig command list
  // to list of other apparatus commands
  fNaI = NULL;
  fPID = NULL;
  fMWPC = NULL;
  fPIDdPhi = fMWPCdTheta = NULL;
  fMaxPIDdPhi = fMaxMWPCdTheta = 0.0;
  fNCharged = 0;
  fAngDiffI = NULL;
  fPIDij = NULL;
  fPIDindex = fPIDdouble = NULL;
  fMWPCij = NULL;
  fMWPCindex = NULL;
  fDeltaE = fDeltaEdouble = fEch = fECentral = NULL;
  fDeltaTheta = fDeltaPhi = NULL;
  fDeltaEOR = fEchOR = 0.0;
  AddCmdList( kCBKeys );                  // for SetConfig()
}


//-----------------------------------------------------------------------------

TA2CrystalBall::~TA2CrystalBall()
{
  // Free up allocated memory

  // Needs upgrade

  if(fDeltaE) delete fDeltaE;
  if(fPIDdPhi) delete fPIDdPhi;
  if(fMWPCdTheta) delete fMWPCdTheta;
}

//-----------------------------------------------------------------------------

TA2DataManager* TA2CrystalBall::CreateChild(const char* name, Int_t dclass)
{
  // Create a TA2Detector class for use with the TA2CrystalBall
  // Valid detector types are...
  // 1. TA2PlasticPID
  // 2. TA2CalArray
  // 3. TA2CylMWPC

  if(!name) name = Map2String(dclass);
  switch (dclass)
  {
   case ECB_PlasticPID:
    fPID = new TA2PlasticPID(name, this);
    return fPID;
   case ECB_CalArray:
    fNaI = new TA2CalArray(name, this);
    return fNaI;
   case ECB_CylMWPC:
    fMWPC = new TA2CylMwpc(name, this);
    return fMWPC;
   default:
    return NULL;
  }
}

//-----------------------------------------------------------------------------

void TA2CrystalBall::LoadVariable( )
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
  TA2DataManager::LoadVariable("NCharged",  &fNCharged,      EISingleX);
  TA2DataManager::LoadVariable("DeltaE",    fDeltaE,         EDSingleX);
  TA2DataManager::LoadVariable("DeltaEdouble",fDeltaEdouble, EDSingleX);
  TA2DataManager::LoadVariable("Echarged",  fEch,            EDSingleX);
  TA2DataManager::LoadVariable("ECentral",  fECentral,       EDSingleX);
  TA2DataManager::LoadVariable("DeltaPhi",  fDeltaPhi,       EDSingleX);
  TA2DataManager::LoadVariable("DeltaTheta",fDeltaTheta,     EDMultiX);
  TA2DataManager::LoadVariable("DeltaEOR",  &fDeltaEOR,      EDSingleX);
  TA2DataManager::LoadVariable("EchargedOR",&fEchOR,         EDSingleX);
  //
  TA2Apparatus::LoadVariable();
}

//-----------------------------------------------------------------------------

void TA2CrystalBall::SetConfig(Char_t* line, Int_t key)
{
  // Any special command-line input for Crystal Ball apparatus

  switch (key){
  case ECBAngleLimits:
    // Angle difference limits
    if( sscanf(line,"%lf%lf",&fMaxPIDdPhi,&fMaxMWPCdTheta) != 2 ){
      PrintError(line,"<CB angle difference limits>");
      return;
    }
    break;
  default:
    // default main apparatus SetConfig()
    TA2Apparatus::SetConfig( line, key );
    break;
  }
}

//-----------------------------------------------------------------------------

void TA2CrystalBall::PostInit( )
{
  // Initialise arrays used to correlate hits in NaI, PiD and MWPC detectors.
  // Load 2D cuts file and get the contained cuts classes
  // Demand particle ID class...if not there self destruct
  // Does not come back if called with EErrFatal

  if(!fParticleID)
    PrintError("", "<Configuration aborted: ParticleID class MUST be specified>", EErrFatal);
  // CB-PID Phi diff array
  fMaxParticle    = fNaI->GetMaxCluster();
  fNaISigmaTime   = ((TA2CalArray*)fNaI)->GetSigmaTime();
  fParticle       = new TA2Particle[fMaxParticle];
  for (Int_t i =0; i < fMaxParticle; i++) fParticle[i].SetPDG(fParticleID->GetPDG());
  if( fPID ){
    Int_t nmax = fMaxParticle * fPID->GetNelem();
    fPIDdPhi = new Double_t[nmax];
    fPIDindex = new Int_t[fMaxParticle];
    fPIDdouble = new Int_t[fMaxParticle];
    fPIDij = new Int_t[nmax];
    fDeltaE = new Double_t[fPID->GetNelem()];
    fDeltaEdouble = new Double_t[fPID->GetNelem()];
    fEch = new Double_t[fPID->GetNelem()];
    fECentral = new Double_t[fPID->GetNelem()];
    fDeltaPhi = new Double_t[fPID->GetNelem()];
    //    fPIDTimeRes = fPId->GetSigmaTime();
    if(fMWPC)
    {
      if((UInt_t)fMWPC->GetMaxTrack() > fPID->GetNelem())
        nmax = fMaxParticle * fMWPC->GetMaxTrack();
    }
    fAngDiffI = new Int_t[nmax];
  }
  // CB-MWPC angle (between vectors) diff array
  if(fMWPC)
  {
    fMWPCdTheta = new Double_t[fMaxParticle * fMWPC->GetMaxTrack()];
    fMWPCindex = new Int_t[fMaxParticle];
    fMWPCij = new Int_t[fMaxParticle * fMWPC->GetMaxTrack()];
    fDeltaTheta = new Double_t[fMWPC->GetMaxTrack()];
  }

  // Finally call the default apparatus post initialise
  TA2Apparatus::PostInit();
}

//-----------------------------------------------------------------------------

void TA2CrystalBall::Reconstruct()
{
  // 1st level particle reconstruction from Crystal Ball
  // Hit in NaI array required, Check if any PID or MWPC

  Int_t nNaI = fNaI->GetNCluster();
  if(nNaI < 1) return;                         // no hit in NaI..no particles
  HitCluster_t** clNaI;
  if(((TA2CalArray*)fNaI)->IsUCLAClustering())
    clNaI = ((TA2CalArray*)fNaI)->GetClusterUCLA();
  else
    clNaI = fNaI->GetCluster();                // NaI cluster struct pointers

  UInt_t* idNaI = fNaI->GetClustHit();         // NaI indices active clusts
  Int_t nPID, nMWPC;                           // # PID hits, # MWPC tracks
  if(!fPID) nPID = 0;                          // no PID
  else nPID = fPID->GetNhits();                // PID available, get # hits
  if(!fMWPC) nMWPC = 0;                        // no MWPC
  else nMWPC = fMWPC->GetNTrack();             // MWPC available, get # tracks

  Double_t theta, energy;

  // No PID...assume all particles are photons as 1st guess and store
  // photon 4-vectors. Save PDG particle-type indices (kGamma) for future
  // convenience and make a fast exit
  if(!nPID)
  {
    for(Int_t i=0; i<nNaI; i++)
    {
      energy = (clNaI[idNaI[i]])->GetEnergy();
      theta  = (clNaI[idNaI[i]])->GetTheta();
      fParticleID->SetP4(fP4+i, kGamma, energy, (clNaI[idNaI[i]])->GetMeanPosition());
      fPDG_ID[i] = kGamma;
      // Fill fParticle array with relevant information
      fParticle[i].Reset();
      fParticle[i].SetTime((clNaI[idNaI[i]])->GetTime());
      fParticle[i].SetClusterSize((clNaI[idNaI[i]])->GetNhits());
      fParticle[i].SetCentralIndex((clNaI[idNaI[i]])->GetIndex()); // Set index of cluster central hit
      fParticle[i].SetApparatus(EAppCB);
      fParticle[i].SetDetector(EDetNaI);
      fParticle[i].SetP4(kGamma, energy, (clNaI[idNaI[i]])->GetMeanPosition());
      fParticle[i].SetSigmaE(((TA2CalArray*)fNaI)->GetSigmaEnergy(energy));
      fParticle[i].SetSigmaTheta(((TA2CalArray*)fNaI)->GetSigmaTheta());
      fParticle[i].SetSigmaPhi(((TA2CalArray*)fNaI)->GetSigmaPhi(theta));
      fParticle[i].SetSigmaTime(fNaISigmaTime);
    }
    fNparticle = nNaI;
    fNCharged = 0;
    if( fMWPC ) fDeltaTheta[0] = EBufferEnd;
    return;
  }

  // PID is there...automatic variables for particle ID analysis
  HitCluster_t* cl;                // cluster from NaI (or other EM cal)
  Int_t i,j,k,l,m;                 // loop indicies etc.
  Int_t* ijp = fPIDij;
  Int_t* ijm = fMWPCij;
  Double_t* diffPID = fPIDdPhi;    // CB-PID phi difference array ptr.
  Double_t* diffMWPC = fMWPCdTheta;// CB-MWPC angle difference array ptr

  // Ensure energy/angle record arrays initialised to "zero"
  for(j=0; j<(Int_t)fPID->GetNelem(); j++)     // Do all PID elements
  {
    fDeltaE[j] = (Double_t)ENullHit;
    fDeltaEdouble[j] = (Double_t)ENullHit;;
    fEch[j] = (Double_t)ENullHit;
    fECentral[j] = (Double_t)ENullHit;
    fDeltaPhi[j] = (Double_t)ENullHit;
  }

  // Sort CB-PID and CB-MWPC angle differences for all NaI clusters
  // all PID elements and all MWPC tracks
  for(i=0; i<nNaI; i++)                          // loop cluster hits
  {
    cl = clNaI[idNaI[i]];                        // -> cluster
    fPIDdouble[i] = ECBNullPartner;              // init no PID double hit
    fPIDindex[i] = ECBNullPartner;               // init no PID partner
    if( fMWPC ) fMWPCindex[i] = ECBNullPartner;  // init no MWPC partner
    // Loop over PID hits... store PID-CB phi differences
    for( j=0; j<nPID; j++ )
    {
      *diffPID++ = TMath::Abs(fPID->GetPosition(fPID->GetHits(j))->Z() - cl->GetPhi());
      *ijp++ = i | (j<<16);                     // store i,j indices
    }
    // Loop over MWPC tracks...store MWPC-CB angle differences
    for( j=0; j<nMWPC; j++ ){
      *diffMWPC++ =
	TMath::RadToDeg() *
	fMWPC->GetTrack(j)->GetDirCos().Angle( *cl->GetMeanPosition() );
      *ijm++ = i | (j<<16);                     // store i,j indices
    }
  }
  // Sort PID-CB phi difference arrays in ascending order
  // If phi difference below acceptable limit store PID - CB combination
  // Also look for doubles and store 2nd-PID - CB combination
  TMath::Sort(nNaI*nPID, fPIDdPhi, fAngDiffI, EFalse);
  for(i=0; i<nPID; i++)
  {
    if( fPIDdPhi[ fAngDiffI[i] ] > fMaxPIDdPhi ) break;// ang diff within lim?
    m = fPIDij[fAngDiffI[i]];                          // get indices
    j = m & 0xffff;                                    // CB index
    k = (m>>16) & 0xffff;                              // PID index
    if(fPIDindex[j] == ECBNullPartner)
      fPIDindex[j] = k;
    else if(fPIDdouble[j] == ECBNullPartner)
      if(TMath::Abs(fPID->GetHits(fPIDindex[j]) - fPID->GetHits(k))== 1) fPIDdouble[j] = k;
  }
  // Sort MWPC-CB angle difference array in ascending order
  // If angle difference below acceptable limit store MWPC - CB combination
  if(nMWPC)
  {
    TMath::Sort(nNaI*nMWPC, fMWPCdTheta, fAngDiffI, EFalse);
    for(i=0; i<nMWPC; i++)
    {
      if(fMWPCdTheta[fAngDiffI[i]] > fMaxMWPCdTheta) break; // within lim?
      m = fMWPCij[fAngDiffI[i]];                                // get indices
      j = m & 0xffff;                                           // CB index
      k = (m>>16) & 0xffff;                                     // MWPC index
      if(fMWPCindex[j] == ECBNullPartner) fMWPCindex[j] = k;
    }
  }

  // Loop over NaI clusters, check for correlated PID and MWPC
  // Store diagnostic variables and do particle ID by dE-E
  const TVector3* trackDir;                            // track direction
  for(i=0,l=0; i<nNaI; i++)
  {
    cl = clNaI[idNaI[i]];                         // CB cluster
    fParticle[i].Reset();            // Register particle as CB particle
    fParticle[i].SetApparatus(EAppCB);            // Register particle as CB particle
    fParticle[i].SetDetector(EDetNaI);            // Register particle as NaI particle
    trackDir = cl->GetMeanPosition();             // direction from CB
    energy   = cl->GetEnergy();                   // Get Energy & Theta and use to store
    theta    = cl->GetTheta();                    // resolution of particle observables
    fParticle[i].SetSigmaE(((TA2CalArray*)fNaI)->GetSigmaEnergy(energy));
    fParticle[i].SetSigmaTheta(((TA2CalArray*)fNaI)->GetSigmaTheta());
    fParticle[i].SetSigmaPhi(((TA2CalArray*)fNaI)->GetSigmaPhi(theta));
    fParticle[i].SetSigmaTime(fNaISigmaTime);
    fParticle[i].SetTime(cl->GetTime());          // Set particle time
    fParticle[i].SetClusterSize(cl->GetNhits());  // Set no. of hits in cluster
    fParticle[i].SetCentralIndex(cl->GetIndex()); // Set index of cluster central hit
    j = fPIDindex[i];
    if(j==ECBNullPartner)
      fPDG_ID[i] = kGamma;
    else
    {
      k = fPID->GetHits(j);                      // PID element index
      // PID - CB energies for correlated hit
      fDeltaE[k] = fPID->GetEnergy(k)
	         * TMath::Abs(TMath::Sin(cl->GetTheta() * TMath::DegToRad()));
      fEch[k] = cl->GetEnergy();                 // cluster energy
      fECentral[k] = cl->GetCentralFrac();       // cluster central fraction
      fDeltaPhi[k] = *(fPIDdPhi + i*nPID + j);   // phi difference
      // Add TA2Particle information for PID coincidence
      fParticle[i].SetDetectorA(EDetPID);
      fParticle[i].SetVetoIndex(k);
      fParticle[i].SetVetoEnergy(fPID->GetEnergy(k));
      fParticle[i].SetVetoTime(fPID->GetTime(k));
      // Check PID double hits...add double to fDeltaE
      if(fPIDdouble[i] != ECBNullPartner)
      {
	fDeltaEdouble[k] = fPID->GetEnergy(fPID->GetHits(fPIDdouble[i]))
	                 * TMath::Abs(TMath::Sin(cl->GetTheta() * TMath::DegToRad()));
	fDeltaE[k] += fDeltaEdouble[k];
      }
      // Save MWPC-CB angle differences
      if(fMWPC)
      {
        j = fMWPCindex[i];
	if(j!=ECBNullPartner)
        {
	  fDeltaTheta[l++] = *(fMWPCdTheta + i*nMWPC + j);
	  trackDir = fMWPC->GetTrack(j)->GetDirCosPtr();
	  fParticle[i].SetDetectorA(EDetMWPC);
	}
      }
      // Now check particle ID by dE-E (if its initialised)
      fPDG_ID[i] = kRootino;  fNCharged++;    // start unknown, charged
      if(fPCut)                               // If cuts sorted
      {
        m = fPCutStart[k];                    // Start cut index for pid[k]
        fDeltaEOR = fDeltaE[k];               // OR for common dE-E cut
        fEchOR = fEch[k];                     // OR for common dE-E cut
        for(Int_t n=0; n<fNSectorCut[k]; n++) // Loop over specified cuts
        {
          if(fPCut[m]->Test())                // Condition met?
          {
            fPDG_ID[i] = GetCutPDGIndex(m);   // OK get associated PDG index
            break;                            // cut OK so exit loop
          }
          m++;                                // for next try
        }
      }
    }
    // Set 4 vector on the basis of ID above
    fParticleID->SetP4(fP4+i, fPDG_ID[i], cl->GetEnergy(), trackDir);
    // Fill fParticle member with relevant information
    fParticle[i].SetP4(fPDG_ID[i], cl->GetEnergy(), trackDir);

    fP4tot+=fP4[i];                    // accumulate total 4 mom.
  }
  if(fMWPC) fDeltaTheta[l] = EBufferEnd;
  fNparticle = nNaI;
  if(!fNparticle) return;
  fMtot = fP4tot.M();
}

//-----------------------------------------------------------------------------

inline void TA2CrystalBall::Cleanup()
{
 // Clear any arrays holding variables
 TA2DataManager::Cleanup();

 if(fNparticle)
   for(Int_t i=0; i<fNparticle; i++)
     fPDG_ID[i] = 0;
 fNparticle = 0;
 fNCharged = 0;
 fP4tot.SetXYZT(0.,0.,0.,0.);
}

//-----------------------------------------------------------------------------
