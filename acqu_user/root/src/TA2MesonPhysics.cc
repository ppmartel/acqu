#include "TA2MesonPhysics.h"


//-----------------------------------------------------------------------------

TA2MesonPhysics::TA2MesonPhysics(const char* Name, TA2Analysis* Analysis) : TA2BasePhysics(Name, Analysis)
{
  //Initialise some analysis parameters
  Threshold = 140.0;
  MinvLo = 115.0;
  MinvHi = 150.0;
  MmissLo = 900.0;
  MmissHi = 980.0;
  kMeson = 111;
}

//-----------------------------------------------------------------------------

TA2MesonPhysics::~TA2MesonPhysics()
{

}

//---------------------------------------------------------------------------

void TA2MesonPhysics::SetConfig(Char_t* line, Int_t key)
{
  TA2BasePhysics::SetConfig(line, key);
}

//---------------------------------------------------------------------------

void TA2MesonPhysics::LoadVariable()
{
  Char_t Name[256];
  Char_t* VarName;

  //Call default LoadVariable()
  TA2BasePhysics::LoadVariable();

  //Analysis stuff (2gamma invariant mass)
  TA2DataManager::LoadVariable("MesonMinv", &MesonMinv, EDSingleX);
  TA2DataManager::LoadVariable("EventTime", EventTime, EDMultiX);

  TA2DataManager::LoadVariable("PhotonThetaCB",    PhotonThetaCB, EDSingleX);
  TA2DataManager::LoadVariable("ProtonThetaCB",   &ProtonThetaCB, EDSingleX);
  TA2DataManager::LoadVariable("PhotonThetaTAPS",  PhotonThetaTAPS, EDSingleX);
  TA2DataManager::LoadVariable("ProtonThetaTAPS", &ProtonThetaTAPS, EDSingleX);
  TA2DataManager::LoadVariable("PhotonEnergyCB",    PhotonEnergyCB, EDSingleX);
  TA2DataManager::LoadVariable("ProtonEnergyCB",   &ProtonEnergyCB, EDSingleX);
  TA2DataManager::LoadVariable("PhotonEnergyTAPS",  PhotonEnergyTAPS, EDSingleX);
  TA2DataManager::LoadVariable("ProtonEnergyTAPS", &ProtonEnergyTAPS, EDSingleX);
  TA2DataManager::LoadVariable("PhotonSizeCB",    PhotonSizeCB, EISingleX);
  TA2DataManager::LoadVariable("ProtonSizeCB",   &ProtonSizeCB, EISingleX);
  TA2DataManager::LoadVariable("PhotonSizeTAPS",  PhotonSizeTAPS, EISingleX);
  TA2DataManager::LoadVariable("ProtonSizeTAPS", &ProtonSizeTAPS, EISingleX);
  TA2DataManager::LoadVariable("PhotonCentralCB",    PhotonCentralCB, EISingleX);
  TA2DataManager::LoadVariable("ProtonCentralCB",   &ProtonCentralCB, EISingleX);
  TA2DataManager::LoadVariable("PhotonCentralTAPS",  PhotonCentralTAPS, EISingleX);
  TA2DataManager::LoadVariable("ProtonCentralTAPS", &ProtonCentralTAPS, EISingleX);
  TA2DataManager::LoadVariable("ProtonVetoCB",   &ProtonVetoCB, EISingleX);
  TA2DataManager::LoadVariable("ProtonVetoTAPS", &ProtonVetoTAPS, EISingleX);
  TA2DataManager::LoadVariable("ProtonDeltaECB",   &ProtonDeltaECB, EDSingleX);
  TA2DataManager::LoadVariable("ProtonDeltaETAPS", &ProtonDeltaETAPS, EDSingleX);

  for(Int_t w=1; w<NWINDOW+1; w++)
  {
    //Missing mass spectra
    sprintf(Name, "MesonMmissW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, MesonMmiss[w], EDMultiX);

    //Lab angular end energy distributions
    sprintf(Name, "LabEMesonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, LabEMeson[w], EDMultiX);

    sprintf(Name, "LabThetaMesonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, LabThetaMeson[w], EDMultiX);

    sprintf(Name, "LabPhiMesonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, LabPhiMeson[w], EDMultiX);

    sprintf(Name, "LabEProtonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, LabEProton[w], EDMultiX);

    sprintf(Name, "LabThetaProtonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, LabThetaProton[w], EDMultiX);

    sprintf(Name, "LabPhiProtonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, LabPhiProton[w], EDMultiX);

    sprintf(Name, "LabEBeamW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, LabEBeam[w], EDMultiX);

    sprintf(Name, "LabChBeamW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, LabChBeam[w], EIMultiX);

    //CM angular end energy distributions
    sprintf(Name, "CMEMesonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, CMEMeson[w], EDMultiX);

    sprintf(Name, "CMThetaMesonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, CMThetaMeson[w], EDMultiX);

    sprintf(Name, "CMPhiMesonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, CMPhiMeson[w], EDMultiX);

    sprintf(Name, "CMEProtonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, CMEProton[w], EDMultiX);

    sprintf(Name, "CMThetaProtonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, CMThetaProton[w], EDMultiX);

    sprintf(Name, "CMPhiProtonW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, CMPhiProton[w], EDMultiX);

    sprintf(Name, "CMETotalW%d", w);
    VarName = new Char_t[strlen(Name)+1];
    strcpy(VarName,Name);
    TA2DataManager::LoadVariable(VarName, CMETotal[w], EDMultiX);
  }
}

//---------------------------------------------------------------------------

void TA2MesonPhysics::PostInit()
{
  //Call default PostInit()
  TA2BasePhysics::PostInit();

  EventTime = new Double_t[nBeam]; //Timing spectrum

  //These spectra are created for each defined time window (prompt/random)
  for(Int_t w=1; w<NWINDOW+1; w++)
  {
    //Missing Mass
    MesonMmiss[w] = new Double_t[nBeam];
    //Lab distributions
    LabEMeson[w] = new Double_t[nBeam];
    LabThetaMeson[w] = new Double_t[nBeam];
    LabPhiMeson[w] = new Double_t[nBeam];
    LabEProton[w] = new Double_t[nBeam];
    LabThetaProton[w] = new Double_t[nBeam];
    LabPhiProton[w] = new Double_t[nBeam];
    LabEBeam[w] = new Double_t[nBeam];
    LabChBeam[w] = new Int_t[nBeam];

    //CM distributions
    CMEMeson[w] = new Double_t[nBeam];
    CMThetaMeson[w] = new Double_t[nBeam];
    CMPhiMeson[w] = new Double_t[nBeam];
    CMEProton[w] = new Double_t[nBeam];
    CMThetaProton[w] = new Double_t[nBeam];
    CMPhiProton[w] = new Double_t[nBeam];
    CMETotal[w] = new Double_t[nBeam];
  }

  KinFitter = new TA2CBKinematicFitter(2, 1, 0);
}

//-----------------------------------------------------------------------------

void TA2MesonPhysics::Reconstruct()
{
  //Perform basic physics tasks
  TA2BasePhysics::Reconstruct();

  Double_t dTime;
  Int_t iWindow;
  TLorentzVector P4Miss;

  //Initialise array counters
  VarInit();

  //Now start with event reconstruction
  nMeson = 0;
  if(nPhoton==2) //Select 2 gamma events (pi0/eta -> 2gamma)
  {
    //Combine 2 photons to meson and calculate invariant mass
    Meson = Photon[0].Add(Photon[1]);
    MesonMinv = Meson.GetMass();

    //Accept as meson if invariant mass in correct range
    if((MesonMinv > MinvLo) && (MesonMinv < MinvHi))
    {
      nMeson = 1;
      CorrectMatrix();
      MesonCorr.SetParticleID(kMeson); //This sets the particle ID code correctly
    }
  }

  //Did we find a meson within the correct mass range?
  if(nMeson==1)
  {
    //Plot histograms of some additional TA2Particle properties
    ParticleProperties();

    //For all possible beam photons...
    for(Int_t t=0; t<nTagged; t++)
    {
      //...find the corresponding time window (0 if not in any window)
      iWindow = 0;
      dTime = Tagged[t].GetTime() - MesonCorr.GetTime();     //Calculate time 'difference'
      for(Int_t w = 1; w<NWINDOW+1; w++)                     //Compare with lower/upper bounds
        if((dTime > Window[w][0]) && (dTime < Window[w][1])) //of all defined windows
          iWindow = w;

      //Plot timing information for all tagger hits
      EventTime[nEventTime] = dTime;
      nEventTime++;

      //If beam photon energy above reaction threshold and photon time within any window...
      if((iWindow > 0) && (Tagged[t].GetEnergy() > Threshold))
      {
        //...calculate missing mass
        P4Miss = fP4target[0] + Tagged[t].GetP4() - MesonCorr.GetP4();
        MesonMmiss[iWindow][nMesonMmiss[iWindow]] = P4Miss.M();
        nMesonMmiss[iWindow]++;

        //Accept event if missing mass is within valid range
        if((P4Miss.M() > MmissLo) && (P4Miss.M() < MmissHi))
        {
          FillLabSpectra(t, iWindow); //Final state distribution for Lab frame
          FillCMSpectra(t, iWindow);  //Final state distribution for CM frame
        }
      }
    }
  }

  //Clean up and set EBufferEnd end markers for arrays
  TermArrays();
}

//-----------------------------------------------------------------------------

void TA2MesonPhysics::VarInit()
{
  //Initally, set single value histograms to EBufferEnd (this value will not be plotted)
  MesonMinv = EBufferEnd;

  //Clear particle properties
  for(UInt_t t=0; t<2;t++)
  {
    PhotonThetaCB[t] = EBufferEnd;
    PhotonThetaTAPS[t] = EBufferEnd;
    PhotonEnergyCB[t] = EBufferEnd;
    PhotonEnergyTAPS[t] = EBufferEnd;
    PhotonSizeCB[t] = EBufferEnd;
    PhotonSizeTAPS[t] = EBufferEnd;
    PhotonCentralCB[t] = EBufferEnd;
    PhotonCentralTAPS[t] = EBufferEnd;
  }
  ProtonThetaCB = EBufferEnd;
  ProtonThetaTAPS = EBufferEnd;
  ProtonEnergyCB = EBufferEnd;
  ProtonEnergyTAPS = EBufferEnd;
  ProtonSizeCB = EBufferEnd;
  ProtonSizeTAPS = EBufferEnd;
  ProtonCentralCB = EBufferEnd;
  ProtonCentralTAPS = EBufferEnd;
  ProtonVetoCB = EBufferEnd;
  ProtonVetoTAPS = EBufferEnd;
  ProtonDeltaECB = EBufferEnd;
  ProtonDeltaETAPS = EBufferEnd;

  //Reset array counters for multi value histograms
  nEventTime = 0;
  for(Int_t w=1; w<NWINDOW+1; w++)
  {
    nMesonMmiss[w] = 0;
    nLab[w] = 0;
    nCM[w] = 0;
  }
}

//-----------------------------------------------------------------------------

void TA2MesonPhysics::TermArrays()
{
  //Set EBufferEnd end markers on multi value histograms
  EventTime[nEventTime] = EBufferEnd;
  for(Int_t w=1; w<NWINDOW+1; w++)
  {
    MesonMmiss[w][nMesonMmiss[w]] = EBufferEnd;

    LabEMeson[w][nLab[w]] = EBufferEnd;
    LabThetaMeson[w][nLab[w]] = EBufferEnd;
    LabPhiMeson[w][nLab[w]] = EBufferEnd;
    LabEProton[w][nLab[w]] = EBufferEnd;
    LabThetaProton[w][nLab[w]] = EBufferEnd;
    LabPhiProton[w][nLab[w]] = EBufferEnd;
    LabEBeam[w][nLab[w]] = EBufferEnd;
    LabChBeam[w][nLab[w]] = EBufferEnd;

    CMEMeson[w][nCM[w]] = EBufferEnd;
    CMThetaMeson[w][nCM[w]] = EBufferEnd;
    CMPhiMeson[w][nCM[w]] = EBufferEnd;
    CMEProton[w][nCM[w]] = EBufferEnd;
    CMThetaProton[w][nCM[w]] = EBufferEnd;
    CMPhiProton[w][nCM[w]] = EBufferEnd;
    CMETotal[w][nCM[w]] = EBufferEnd;
  }
}

//-----------------------------------------------------------------------------

void TA2MesonPhysics::FillLabSpectra(Int_t nTag, Int_t iWindow)
{
  //Set energy and angular distributions in Lab frame for meson...
  LabEMeson[iWindow][nLab[iWindow]] = MesonCorr.GetEnergy();
  LabThetaMeson[iWindow][nLab[iWindow]] = MesonCorr.GetThetaDg();
  LabPhiMeson[iWindow][nLab[iWindow]] = MesonCorr.GetPhiDg();

  //...and proton (if detected)
  if(nProton==1)
  {
    LabEProton[iWindow][nLab[iWindow]] = Proton[0].GetEnergy();
    LabThetaProton[iWindow][nLab[iWindow]] = Proton[0].GetThetaDg();
    LabPhiProton[iWindow][nLab[iWindow]] = Proton[0].GetPhiDg();
  }
  else
  {
    LabEProton[iWindow][nLab[iWindow]] = EBufferEnd;
    LabThetaProton[iWindow][nLab[iWindow]] = EBufferEnd;
    LabPhiProton[iWindow][nLab[iWindow]] = EBufferEnd;
  }

  //Histogram beam energy
  LabEBeam[iWindow][nLab[iWindow]] = Tagged[nTag].GetEnergy();
  LabChBeam[iWindow][nLab[iWindow]] = Tagged[nTag].GetCentralIndex();

  nLab[iWindow]++;
}

//-----------------------------------------------------------------------------

void TA2MesonPhysics::FillCMSpectra(Int_t nTag, Int_t iWindow)
{
  TVector3 BoostVect;
  TLorentzVector P4;

  //Calculate boost vector
  BoostVect = -1.0 * (fP4target[0] + Tagged[nTag].GetP4()).BoostVector();

  //L-boost from Lab to CM frame
  P4 = MesonCorr.GetP4();
  P4.Boost(BoostVect);

  //Set energy and angular distributions in CM frame for meson...
  CMEMeson[iWindow][nCM[iWindow]] = P4.E();
  CMThetaMeson[iWindow][nCM[iWindow]] = P4.Theta() * TMath::RadToDeg();
  CMPhiMeson[iWindow][nCM[iWindow]] = P4.Phi() * TMath::RadToDeg();

  //...and proton (if detected)
  if(nProton==1)
  {
    P4 = Proton[0].GetP4();
    P4.Boost(BoostVect);

    CMEProton[iWindow][nCM[iWindow]] = P4.E();
    CMThetaProton[iWindow][nCM[iWindow]] = P4.Theta() * TMath::RadToDeg();
    CMPhiProton[iWindow][nCM[iWindow]] = P4.Phi() * TMath::RadToDeg();
  }
  else
  {
    CMEProton[iWindow][nCM[iWindow]] = EBufferEnd;
    CMThetaProton[iWindow][nCM[iWindow]] = EBufferEnd;
    CMPhiProton[iWindow][nCM[iWindow]] = EBufferEnd;
  }

  //Total CM energy W = sqrt(2*M_p*omega + M_p^2)
  CMETotal[iWindow][nCM[iWindow]] = TMath::Sqrt(2.0*938.272*Tagged[nTag].GetEnergy() + 938.272*938.272);

  nCM[iWindow]++;
}

//-----------------------------------------------------------------------------

void TA2MesonPhysics::ParticleProperties()
{
  //Pick up particle properties for the 2 decay photons
  for(Int_t p=0; p<2; p++)
  {
    if(Photon[p].HasApparatus(EAppCB)) //Photon was detected and identified in CB apparatus
    {
      PhotonThetaCB[p]   = Photon[p].GetThetaDg();
      PhotonEnergyCB[p]  = Photon[p].GetEnergy();
      PhotonSizeCB[p]    = Photon[p].GetClusterSize();
      PhotonCentralCB[p] = Photon[p].GetCentralIndex();
    }
    else if(Photon[p].HasApparatus(EAppTAPS)) //Photon was detected and identified in TAPS apparatus
    {
      PhotonThetaTAPS[p]   = Photon[p].GetThetaDg();
      PhotonEnergyTAPS[p]  = Photon[p].GetEnergy();
      PhotonSizeTAPS[p]    = Photon[p].GetClusterSize();
      PhotonCentralTAPS[p] = Photon[p].GetCentralIndex();
    }
  }

  //Pick up particle properties for the recoil proton, if detected
  if(nProton==1)
  {
    if(Proton[0].HasApparatus(EAppCB)) //Proton was detected and identified in CB apparatus
    {
      ProtonThetaCB   = Proton[0].GetThetaDg();
      ProtonEnergyCB  = Proton[0].GetKinetic();
      ProtonSizeCB    = Proton[0].GetClusterSize();
      ProtonCentralCB = Proton[0].GetCentralIndex();
      if(Proton[0].HasDetector(EDetPID)) //Proton was also detected in PID detector
      {
        ProtonVetoCB    = Proton[0].GetVetoIndex();
        ProtonDeltaECB  = Proton[0].GetVetoEnergy();
      }
    }
    else if(Proton[0].HasApparatus(EAppTAPS)) //Proton was detected and identified in TAPS apparatus
    {
      ProtonThetaTAPS   = Proton[0].GetThetaDg();
      ProtonEnergyTAPS  = Proton[0].GetKinetic();
      ProtonSizeTAPS    = Proton[0].GetClusterSize();
      ProtonCentralTAPS = Proton[0].GetCentralIndex();
      if(Proton[0].HasDetector(EDetVeto)) //Proton was also detected in Veto detector
      {
        ProtonVetoTAPS    = Proton[0].GetVetoIndex();
        ProtonDeltaETAPS  = Proton[0].GetVetoEnergy();
      }
    }
  }
}

//-----------------------------------------------------------------------------

void TA2MesonPhysics::ParseMisc(char* line)
{
  Char_t sWord[256];

  //Get keyword
  if(sscanf(line, "%s", sWord)!=1) return;

  if(!strcmp("Threshold", sWord))
  {
    sscanf(line, "%*s %lf", &Threshold);
    printf("Reaction threshold is %f\n", Threshold);
    return;
  }

  if(!strcmp("InvariantMass", sWord))
  {
    sscanf(line, "%*s %lf %lf", &MinvLo, &MinvHi);
    printf("Invariant mass cut window from %f to %f\n", MinvLo, MinvHi);
    return;
  }

  if(!strcmp("MissingMass", sWord))
  {
    sscanf(line, "%*s %lf %lf", &MmissLo, &MmissHi);
    printf("Missing mass cut window from %f to %f\n", MmissLo, MmissHi);
    return;
  }

  if(!strcmp("MesonID", sWord))
  {
    sscanf(line, "%*s %d", &kMeson);
    printf("Meson ID is %d\n", kMeson);
    printf(" %d = pi0\n", kPi0);
    printf(" %d = eta\n", kEta);
    return;
  }

  TA2BasePhysics::ParseMisc(line);
}

//-----------------------------------------------------------------------------

void TA2MesonPhysics::CorrectMatrix()
{
  KFPhoton[0].Set4Vector(Photon[0].GetP4());
  KFPhoton[1].Set4Vector(Photon[1].GetP4());
  KFPhoton[0].SetResolutions(Photon[0].GetSigmaTheta(), Photon[0].GetSigmaPhi(), Photon[0].GetSigmaE());
  KFPhoton[1].SetResolutions(Photon[1].GetSigmaTheta(), Photon[1].GetSigmaPhi(), Photon[1].GetSigmaE());

  KinFitter->Reset();
  KinFitter->AddPosKFParticle(KFPhoton[0]);
  KinFitter->AddPosKFParticle(KFPhoton[1]);
  KinFitter->AddInvMassConstraint(MASS_PIZERO);
  KinFitter->Solve();

  KFMeson = KinFitter->GetTotalFitParticle();

  MesonCorr.SetP4(KFMeson.Get4Vector());
}

//-----------------------------------------------------------------------------

ClassImp(TA2MesonPhysics)
