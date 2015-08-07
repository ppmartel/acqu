#include "TA2CB.h"

//Default list of detector classes that the TA2CB apparatus may contain
enum { ECylMWPC, EPlasticPID, ECalArray };

static Map_t kValidDetectors[] =
{
  {"TA2CylMWPC",        ECylMWPC},
  {"TA2PlasticPID",     EPlasticPID},
  {"TA2CalArray",       ECalArray},
  {NULL,                -1}
};

ClassImp(TA2CB)

//-----------------------------------------------------------------------------

TA2CB::TA2CB(const char* name, TA2System* analysis)  : TA2Apparatus(name, analysis, kValidDetectors),
    fPIDCuts()
{
  //Initialise/clear detector pointers
  fCal = NULL;
  fPID = NULL;
  fTracker = NULL;

  //Initialise reconstruction parameters to reasonable defaults
  fPIDTimeShift = 0.0;
  for(Int_t t=0; t<3; t++) fPosShift[t] = 0.0;

  fMaxDPhiPID = 15.0;
  fMaxDPhiMWPC = 15.0;
  fMaxDThetaMWPC = 10.0;
  fRatio2 = (fMaxDPhiMWPC/fMaxDThetaMWPC) * (fMaxDPhiMWPC/fMaxDThetaMWPC);
  fMaxDOmega2 = fMaxDPhiMWPC * fMaxDPhiMWPC;

  //No droop correction for light attenuation in PID strips by default
  bPIDIsDroop = false;

  //Number of defined DeltaE-E cuts
  nPIDCount = 0;
}

//-----------------------------------------------------------------------------

TA2CB::~TA2CB()
{

}

//-----------------------------------------------------------------------------

TA2DataManager* TA2CB::CreateChild(const char* name, Int_t dclass)
{
  //Create TA2Detector classes for use with TA2CB
  if(!name) name = Map2String(dclass);
  switch (dclass)
  {
   default:
    return NULL;
   case ECylMWPC:
    fTracker = new TA2CylMwpc(name, this);
    return fTracker;
   case EPlasticPID:
    fPID = new TA2PlasticPID(name, this);
    return fPID;
   case ECalArray:
    fCal = new TA2CalArray(name, this);
    return fCal;
  }
}

//-----------------------------------------------------------------------------

void TA2CB::LoadVariable()
{
  //Input name - variable pointer associations for any subsequent
  //cut or histogram setup.
  //LoadVariable( "name", pointer-to-variable, type-spec );
  //NB scaler variable pointers need the preceeding &
  //   array variable pointers do not.
  //type-spec ED prefix for a Double_t variable
  //          EI prefix for an Int_t variable
  //type-spec SingleX for a single-valued variable
  //          MultiX  for a multi-valued variable
  //                           Name          Pointer        Type
  TA2DataManager::LoadVariable("E",          fECrystal,     EDSingleX);
  TA2DataManager::LoadVariable("EOR",        fECrystalOR,   EDMultiX);
  TA2DataManager::LoadVariable("DeltaE",     fEPlastic,     EDSingleX);
  TA2DataManager::LoadVariable("DeltaEOR",   fEPlasticOR,   EDMultiX);
  TA2DataManager::LoadVariable("DeltaPhi",   fDeltaPhi,     EDMultiX);

  TA2DataManager::LoadVariable("Gamma",      &nGamma,       EISingleX);

  TA2Apparatus::LoadVariable();
}

//-----------------------------------------------------------------------------

void TA2CB::PostInit()
{
  fMaxParticle = fCal->GetMaxCluster();

  //Get PID geometry parameters
  if(fPID)
  {
    nPlastic = fPID->GetNelement();

    fPIDPhiPlastic = new Double_t[nPlastic];
    for(Int_t t=0; t<nPlastic; t++)
      fPIDPhiPlastic[t]= fPID->GetPosition()[t]->Z();
    fPIDRadius = fPID->GetPosition()[0]->X()/10.0;
    fPIDLength = 30.0;

    //Allocate memory for histograms
    fEPlastic = new Double_t[nPlastic]; //Energies of charged particles in the PID
    fECrystal = new Double_t[nPlastic]; //Energies of charged clusters in the NaI array

    fDeltaPhi = new Double_t[nPlastic*fMaxParticle+1];   //Phi difference between NaI and PID
    fEPlasticOR = new Double_t[nPlastic*fMaxParticle+1]; //Energies of charged particles in the PID
    fECrystalOR = new Double_t[nPlastic*fMaxParticle+1]; //Energies of charged clusters in the NaI array
  }

  //Allocate memory for particle data
  fType = new Int_t[fMaxParticle];
  fDet = new Int_t[fMaxParticle];
  fSize = new Int_t[fMaxParticle];
  fCentral = new Int_t[fMaxParticle];
  fTime = new Double_t[fMaxParticle];
  fParticles = new TA2Particle[fMaxParticle];

  TA2Apparatus::PostInit();
}

//-----------------------------------------------------------------------------

void TA2CB::DoParticleIdent()
{
  Bool_t bCharged;     //Charged/uncharged particle?
  Int_t fPIDIndex;     //Number of hit PID element
  Double_t fPIDEnergy; //Energy of hit PID element
  Double_t fPIDTime;   //Time of hit PID element
  Double_t fMass;      //Mass in MeV of particle hypothesis

  //Clear counters and arrays
  nDeltaPhi = 0;
  nCharged = 0;
  for(Int_t t=0; t<nPlastic; t++)
  {
    fEPlastic[t] = ENullHit;//EBufferEnd;
    fECrystal[t] = ENullHit;//EBufferEnd;
  }

  //Check all particles detected in NaI
  for(Int_t t=0; t<fNparticle; t++)
  {
    //Ask PID and NaI for information about charged particles
    bCharged = MakeDEvE(&fP4[t], &fDet[t], &fPIDEnergy, &fPIDTime, &fPIDIndex);

    if(bCharged) //It's charged, so it made something in the PID
    {
      //Get energies in PID and NaI
      fEPlastic[fPIDIndex] = fPIDEnergy * TMath::Sin(fP4[t].Theta());
      fECrystal[fPIDIndex] = fP4[t].E();

      //Histogram charged particle DEvE for all elements
      fEPlasticOR[nCharged] = fEPlastic[fPIDIndex];
      fECrystalOR[nCharged] = fECrystal[fPIDIndex];
      nCharged++;

      //Until now it is an unknown charged particle
      fType[t] = kRootino;
      fMass = fParticleID->GetMassMeV(kRootino);

      //Check if DEvE is inside any of the cuts for this PID element
      for(Int_t c=0; c<nPIDCuts; c++)
        if(fPIDIndex==fPIDElement[c])
          if(fPIDCuts[c]->IsInside(fECrystal[fPIDIndex], fEPlastic[fPIDIndex]))
          {
            fType[t] = fPIDCode[c];                       //Set particle ID code...
            fMass = fParticleID->GetMassMeV(fPIDCode[c]); //...and particle mass
          }

      //Modify original 4-momentum and build list of particle types and used informations
      fP4[t] = ParticleMass(fP4[t].Vect(), fECrystal[fPIDIndex], fMass);
      //fTime[t] = fPIDTime + PIDTimeShift;    //Use PID Time for charged particles
      fDet[t]+=EDetPID;                      //NaI and PID build this 4-momentum

      //Update TA2Particle data with PID information
      fParticles[t].SetVetoTime(fPIDTime + fPIDTimeShift);
      fParticles[t].SetVetoEnergy(fEPlastic[fPIDIndex]);
      fParticles[t].SetVetoIndex(fPIDIndex);
    }
    else //PID says it's uncharged...
    {
      if(MakeTrack(&fP4[t])) //...but MWPC says, it's charged. This particle I can't identify.
      {
        fType[t] = kRootino;
        fDet[t]+=EDetMWPC;
      }
      else //...and MWPC also says, it's uncharged. This is interpreted as a photon.
      {
        fType[t] = kGamma;
        nGamma++;
      }
    }
    //Set TA2Particle data for identified particle type
    fParticles[t].SetP4(fP4[t]);
    fParticles[t].SetParticleIDA(fType[t]);
    fParticles[t].SetDetectorA(fDet[t] - fParticles[t].GetDetectors());
  }

  //Terminate arrays
  fDeltaPhi[nDeltaPhi] = EBufferEnd;
  fEPlasticOR[nCharged] = EBufferEnd;
  fECrystalOR[nCharged] = EBufferEnd;
}

//-----------------------------------------------------------------------------

TLorentzVector TA2CB::ParticleMass(TVector3 p_, Double_t T, Double_t M)
{
  TLorentzVector p4;

  Double_t E; //Total energy of particle (E = T + m)
  Double_t p; //Absolute value of particle 3-momentum (p = sqrt(E^2 - m^2) = sqrt(T^2 + 2*T*m))

  E = T + M;
  p = TMath::Sqrt(T*T + 2.0*T*M); //Build absolute value of particle 3-momentum p = sqrt(T^2 + 2*T*m)

  p4.SetE(E);                //Make correct 0-component for particle
  p4.SetVect(p_.Unit() * p); //Make correct 1..3-component for particle

  return p4;
}

//-----------------------------------------------------------------------------

Bool_t TA2CB::MakeTrack(TLorentzVector* fP4)
{
  if(!fTracker) return false;

  Double_t* fPhiWC;   //Phis of WC tracks
  Double_t* fThetaWC; //Thetas of WC tracks
  Double_t fPhiCB;    //Phi of CB 4-momentum
  Double_t fThetaCB;  //Theta of CB 4-momentum
  Double_t fDPhi;     //Difference between MWPC and Missing Phi
  Double_t fDTheta;   //Difference between MWPC and Missing Theta
  Double_t fDOmega2;  //Current squared sum of DPhi and DTheta
  Double_t fMin;      //Best squared sum of DPhi and DTheta
  Int_t iMin;         //Index of best track
  Int_t nTracks;      //Number of tracks in WC
  Bool_t bResult;     //Angles have been exchanged or not?

  //Get CB stuff
  fPhiCB = fP4->Phi();
  fThetaCB = fP4->Theta();

  //Get WC stuff
  nTracks = ((TA2CylMwpc*)fTracker)->GetNTrack();
  fPhiWC = ((TA2CylMwpc*)fTracker)->GetTrackPhi();
  fThetaWC = ((TA2CylMwpc*)fTracker)->GetTrackTheta();

  //Test with all MWPC tracks and find minimum
  iMin = 0;
  fMin = 1e38;
  for(Int_t t=0; t<nTracks; t++)
  {
    fDPhi = (fPhiCB - fPhiWC[t]) * TMath::RadToDeg();
    if(fDPhi > +180.0) fDPhi = +360.0 - fDPhi;
    if(fDPhi < -180.0) fDPhi = -360.0 - fDPhi;
    fDTheta = (fThetaCB - fThetaWC[t]) * TMath::RadToDeg();
    fDOmega2 = fDPhi*fDPhi + fDTheta*fDTheta * fRatio2;
    if(fDOmega2 < fMin)
    {
      fMin = fDOmega2;
      iMin = t;
    }
  }

  //Exchange angles and set vertex if lowest deviation is below limits in each angle
  bResult = false;
  if(nTracks)
    if(fMin < fMaxDOmega2)
    {
      fP4->SetTheta(fThetaWC[iMin]);
      fP4->SetPhi(fPhiWC[iMin]);
      bResult = true;
    }
  return bResult;
}

//-----------------------------------------------------------------------------

Bool_t TA2CB::MakeDEvE(TLorentzVector* fP4, Int_t* fDet, Double_t* fPIDEnergy, Double_t* fPIDTime, Int_t* fPIDIndex)
{
  if(!fPID) return false;

  UInt_t PIDCorrPos;      //Position for calculating the droop correction factor
  Double_t PIDPathLength; //Path length of scintillation light
  Double_t PIDLostFac;    //Calculated droop correction factor
  Double_t fDPhi;         //Current phi difference between NaI cluster and PID hit
  Double_t fMin;          //Best phi difference between NaI cluster and PID hit
  Int_t iPID;             //Element of current PID hit
  Int_t iMin;             //Element of best PID hit
  Int_t nPID;             //Number of PID hits
  Bool_t bCharged;        //Particle is charged or not?

  //Get number of PID hits
  nPID = fPID->GetNhits();

  //Calculate Phi difference for all PID hits and find minimum
  iMin = 0;
  fMin = 1e38;
  for(Int_t s=0; s<nPID; s++)
  {
    iPID = fPID->GetHits(s);
    fDPhi = fPIDPhiPlastic[iPID] - (fP4->Phi() * TMath::RadToDeg());
    if(fDPhi > +180.0) fDPhi = +360.0 - fDPhi;
    if(fDPhi < -180.0) fDPhi = -360.0 - fDPhi;
    if(fabs(fDPhi) < fabs(fMin))
    {
      fMin = fDPhi;
      iMin = iPID;
    }
  }

  //Accept as charged if lowest deviation is below limit in phi
  bCharged = false;
  if(nPID)
  {
    if(fabs(fMin) < fMaxDPhiPID)
    {
      //If we have a correlated track in MWPC, use this as direction information (if not already used)
      if(!((*fDet) & EDetMWPC))
        if(MakeTrack(fP4)) (*fDet)+=EDetMWPC; //MWPC did help building this 4-momentum

      //Droop correction
      if((fP4->Theta()*TMath::RadToDeg())==90.0) //If theta = 90 set light path length by hand
        PIDPathLength = fPIDLength/2.0;
      else //Theta is not 90
        PIDPathLength = fPIDLength/2.0 - fPIDRadius/TMath::Tan(fP4->Theta()); //Calculate light path length

      //Calculating the correction factor by extrapolating linearly between two positions
      if(bPIDIsDroop)
      {
        for(PIDCorrPos=0; PIDCorrPos < nPIDDroopPoints-1; PIDCorrPos++) //Find the position in correction factor array
          if((PIDPathLength>=fPIDLostCorrPos[PIDCorrPos]) && (PIDPathLength<=fPIDLostCorrPos[PIDCorrPos+1])) break; //Between which positions?
        PIDLostFac = (PIDPathLength - fPIDLostCorrPos[PIDCorrPos])
                   * (fPIDLostCorrFac[PIDCorrPos+1] - fPIDLostCorrFac[PIDCorrPos])
                   / (fPIDLostCorrPos[PIDCorrPos+1] - fPIDLostCorrPos[PIDCorrPos])
                   + fPIDLostCorrFac[PIDCorrPos];
      }
      else
        PIDLostFac = 1.0; //Droop correction turned off

      //Store PID hit information for charged particle
      (*fPIDEnergy) = fPID->GetEnergy(iMin) * PIDLostFac;
      (*fPIDTime) = fPID->GetTime(iMin);
      (*fPIDIndex) = iMin;
      bCharged = true;
      fEPlasticOR[nCharged] = (*fPIDEnergy);
      fECrystalOR[nCharged] = fP4->E();
    }
    //Histogram phi differences for charged particles
    fDeltaPhi[nDeltaPhi] = fMin;
    nDeltaPhi++;
  }

  return bCharged;
}

//-----------------------------------------------------------------------------

inline void TA2CB::ParseMisc(char* line)
{
  Char_t sWord[256];
  Char_t sGetter[256];
  Char_t cLetter;
  Char_t* pNumbers;

  Char_t CutName[256];
  Char_t FileName[256];
  Int_t Element;
  Int_t Code;
  TFile* PIDCutFile;


  //Get keyword
  if(sscanf(line, "%s", sWord)!=1)
  {
    printf("Initialized number of droop parameters with default 2\n");
    return;
  }

  pNumbers = line + strlen(sWord) + 1;

  if(!strcmp("Droop", sWord))
  {
    printf("Droop correction turned on\n");
    bPIDIsDroop = true;
    return;
  }

  if(!strcmp("DroopPoints", sWord))
  {
    if(sscanf(line, "%*s %u", &nPIDDroopPoints)!=1)
    {
      nPIDDroopPoints = 2;
      printf("Initializing number of droop parameters with 2\n");
    }
    if(nPIDDroopPoints<2)
    {
      nPIDDroopPoints = 2;
      printf("Setting number of points to 2\n");
    }
    if(bPIDIsDroop)
      printf("%u points used for the droop correction\n", nPIDDroopPoints);

    fPIDLostCorrPos = new Double_t[nPIDDroopPoints];
    fPIDLostCorrFac = new Double_t[nPIDDroopPoints];
    return;
  }

  if(!strcmp("DroopLength", sWord))
  {
    sscanf(line, "%*s %lf", &fPIDLength);
    printf("PID plastic strip length set to %f\n", fPIDLength);
    return;
  }

  if(!strcmp("DroopPosition", sWord))
  {
    for(UInt_t l=0; l<nPIDDroopPoints; l++)
    {
      sscanf(pNumbers, "%c", &cLetter);
      while(cLetter==' ')
      {
        pNumbers++;
        sscanf(pNumbers, "%c", &cLetter);
      }
      if(sscanf(pNumbers, "%s", sGetter)!=1)
      {
        printf("Turning the droop correction off\n");
        bPIDIsDroop = false;
        return;
      }
      pNumbers += strlen(sGetter);
      sscanf(sGetter, "%lf", &fPIDLostCorrPos[l]);
    }
    if(fPIDLostCorrPos[0]!=0.0)
    {
      printf("Setting first position to 0.0\n");
      fPIDLostCorrPos[0] = 0.0;
    }
    if(fPIDLostCorrPos[nPIDDroopPoints-1]!=fPIDLength)
     {
      printf("Setting last position to %f\n", fPIDLength);
      fPIDLostCorrPos[nPIDDroopPoints-1] = fPIDLength;
    }
    return;
  }

  if(!strcmp("DroopFactor", sWord))
  {
    for(UInt_t l=0; l<nPIDDroopPoints; l++)
    {
      sscanf(pNumbers, "%c", &cLetter);
      while(cLetter == ' ')
      {
        pNumbers++;
        sscanf(pNumbers, "%c", &cLetter);
      }
      if(sscanf(pNumbers, "%s", sGetter) != 1)
      {
        printf("Turning the droop correction off\n");
        bPIDIsDroop = false;
        return;
      }
      pNumbers += strlen(sGetter);
      sscanf(sGetter, "%lf", &fPIDLostCorrFac[l]);
    }
    return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if(!strcmp("PIDTimeShift", sWord))
  {
    sscanf(line, "%*s %lf", &fPIDTimeShift);
    printf("Shift for PID timing set to %f\n", fPIDTimeShift);
    return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if(!strcmp("PosShift", sWord))
  {
    sscanf(line, "%*s %lf %lf %lf", &fPosShift[0], &fPosShift[1], &fPosShift[2]);
    printf("CB Shift for target position set to (%f, %f, %f)\n",
           fPosShift[0], fPosShift[1], fPosShift[2]);
    return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if(!strcmp("AngleLimits", sWord))
  {
    sscanf(line, "%*s %lf %lf %lf", &fMaxDPhiPID, &fMaxDPhiMWPC, &fMaxDThetaMWPC);
    printf("Angle differencs for charged particles set to \n");
    printf(" %lf (DPhi_PID) and %lf, %lf (DPhi_MWPC, DTheta_MWPC)\n", fMaxDPhiPID, fMaxDPhiMWPC, fMaxDThetaMWPC);
    fRatio2 = (fMaxDPhiMWPC/fMaxDThetaMWPC) * (fMaxDPhiMWPC/fMaxDThetaMWPC);
    fMaxDOmega2 = fMaxDPhiMWPC * fMaxDPhiMWPC;
    return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if(!strcmp("PIDCuts", sWord))
  {
    sscanf(line, "%*s %d", &nPIDCuts);
    printf("Using %d PID cuts\n", nPIDCuts);
    //fPIDCut.resize(nPIDCuts);
    fPIDCode = new Int_t[nPIDCuts];
    fPIDElement = new Int_t[nPIDCuts];
    return;
  }

  if(!strcmp("PIDCut", sWord))
  {
    sscanf(line, "%*s %d %d %s %s", &Element, &Code, CutName, FileName);
    PIDCutFile = new TFile(FileName, "READ");
    TCutG* PIDCutPtr = (TCutG*)PIDCutFile->Get(CutName);
    fPIDCuts.push_back(PIDCutPtr);
    fPIDCode[nPIDCount] = Code;
    fPIDElement[nPIDCount] = Element;
    printf("Cut %s enabled for PDG ID %d and PID element %d\n", fPIDCuts[nPIDCount]->GetName(), fPIDCode[nPIDCount], fPIDElement[nPIDCount]);
    PIDCutFile->Close();
    nPIDCount++;
    return;
  }

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  printf("Misc keyword %s not supported\n", sWord);
  return;
}

//-----------------------------------------------------------------------------
