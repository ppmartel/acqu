#include "TA2BasePhysics.h"
#include "TAcquFile.h"

//For software trigger keyword parsing
enum { ETrigger=5000, ETriggerParam, ESumModel, EGainsNaI, EThresBaF2, EThresNaI, EScaleNaI };

static const Map_t kTriggKeys[] =
{
  {"Trigger:",      ETrigger},
  {"TriggerParam:", ETriggerParam},
  {"SumModel:",     ESumModel},
  {"GainsNaI:",     EGainsNaI},
  {"ThresBaF2:",    EThresBaF2},
  {"ThresNaI:",     EThresNaI},
  {"ScaleNaI:",     EScaleNaI},
  {NULL,            -1}
};


//-----------------------------------------------------------------------------

TA2BasePhysics::TA2BasePhysics(const char* Name, TA2Analysis* Analysis) : TA2AccessSQL(Name, Analysis)
{
  //Pointers are declared in TA2Physics, but no memory is allocated. So, here 'dummy' content is
  //produced to avoid error messages in Analysis.log due to null pointers used in LoadVariable()
  fPmiss = new Double_t[1];
  fMmiss = new Double_t[1];
  fPmiss[0] = EBufferEnd;
  fMmiss[0] = EBufferEnd;

  AddCmdList(kTriggKeys); //Enables keyword recognition for SetConfig()

  //Initialize trigger variables
  DoTrigger = false;
  SimulateTrigger = false;
  UseSumModel = false;

  ESumThres = 0.0;
  Rings = 0;
  for(UInt_t m=1; m<5; m++)
  {
    UseM[m]= false;
    PrescaleM[m] = 0;
  }

  //Clear calibration and threshold arrays for CB and TAPS channels
  for(Int_t t=0; t<720; t++) //720 NaI crystals
  {
    GainsNaI[t] = 1.0;
    ThresNaI[t] = 0.0;
    SigmaNaI[t] = 0.0;
  }
  ScaleNaI = 1.0;
  for(Int_t t=0; t<438; t++) //up to 438 BaF2/PbWO4 crystals
  {
    ThresBaF2[t] = 0.0;
    SigmaBaF2[t] = 0.0;
  }
  //By default, produce no TA2Particle filled ROOT files
  bDoROOT = false;

}

//-----------------------------------------------------------------------------

TA2BasePhysics::~TA2BasePhysics()
{
  if(bDoROOT)
  {
    EventFile->Write();
    EventFile->Close();
  }
}

//---------------------------------------------------------------------------

void TA2BasePhysics::SetConfig(Char_t* line, Int_t key)
{
  FILE* TextFile;
  Int_t Dummy;
  //Any special command-line input for Crystal Ball apparatus
  switch (key)
  {
    case ETrigger:
      if(sscanf(line, "%d %d %d %d\n", &UseM[1], &UseM[2], &UseM[3], &UseM[4])==4)
      {
        printf("Trigger decoding enabled\n");
        for(UInt_t m=1; m<5; m++)
        {
          printf(" Accept multiplicity M%d events: ", m);
          if(UseM[m]) printf("yes\n"); else printf("no\n");
        }
        DoTrigger = true;
      }
      break;
    case ETriggerParam:
      if(sscanf(line, "%lf %lf %d %d %d %d %d %d\n", &ESumThres, &ESumSigma, &Rings, &PrescaleM[1], &PrescaleM[2], &PrescaleM[3], &PrescaleM[4], &NewTAPS)==8)
      {
        printf("Software L1 & L2 triggers enabled\n");
        printf(" Energy sum: %f +- %f\n", ESumThres, ESumSigma);
        printf(" Disabled TAPS rings: ");
        if(Rings < 3) printf("%d\n", Rings); else printf("all\n");
        for(UInt_t m=1; m<5; m++)
          printf(" M%d prescale factor: %d\n", m, PrescaleM[m]);
        printf(" TAPS configuration with %d crystals per block\n", NewTAPS);
        SimulateTrigger = true;
      }
      break;
    case ESumModel:
      if(sscanf(line, "%s\n", SumFilename)==1)
      {
        printf("Reading energy sum model from\n %s\n", SumFilename);
        TextFile = fopen(SumFilename, "r");
        for(Int_t t=0; t<2001; t++)
          if(fscanf(TextFile, "%d %lf", &Dummy, &SumModel[t]));
        fclose(TextFile);
        UseSumModel = true;
      }
      break;
    case EGainsNaI:
      if(sscanf(line, "%s\n", NaIFilename)==1)
      {
        printf("Reading NaI gains from\n %s\n", NaIFilename);
        TextFile = fopen(NaIFilename, "r");
        for(Int_t t=0; t<720; t++)
        {
          if(fscanf(TextFile, "%lf", &GainsNaI[t]));
          if(GainsNaI[t]==0.0) GainsNaI[t] = 1.0;
        }
        fclose(TextFile);
      }
      break;
    case EThresNaI:
      if(sscanf(line, "%s\n", NaIFilename)==1)
      {
        printf("Reading NaI thresholds from\n %s\n", NaIFilename);
        TextFile = fopen(NaIFilename, "r");
        for(Int_t t=0; t<720; t++)
          if(fscanf(TextFile, "%d %lf %lf", &Dummy, &ThresNaI[t], &SigmaNaI[t]));
        fclose(TextFile);
      }
      break;
    case EScaleNaI:
      if(sscanf(line, "%lf\n", &ScaleNaI)==1)
      {
        printf("Scale for NaI thresholds is %f\n", ScaleNaI);
      }
      break;
    case EThresBaF2:
      if(sscanf(line, "%s\n", BaF2Filename)==1)
      {
        printf("Reading BaF2 thresholds from\n %s\n", BaF2Filename);
        TextFile = fopen(BaF2Filename, "r");
        for(Int_t t=0; t<438; t++)
          if(fscanf(TextFile, "%d %lf %lf", &Dummy, &ThresBaF2[t], &SigmaBaF2[t]));
        fclose(TextFile);
      }
      break;
    default:
      //Call default SetConfig()
      TA2AccessSQL::SetConfig(line, key);
      break;
  }
}

//---------------------------------------------------------------------------

void TA2BasePhysics::LoadVariable()
{
  //Call default LoadVariable()
  TA2AccessSQL::LoadVariable();

  //Trigger histograms
  TA2DataManager::LoadVariable("ESum", &ESum, EDSingleX);
  TA2DataManager::LoadVariable("Mult", &Mult, EISingleX);

  TA2DataManager::LoadVariable("L1Pattern", &L1Pattern, EISingleX);
  TA2DataManager::LoadVariable("L2Pattern", &L2Pattern, EISingleX);

  TA2DataManager::LoadVariable("NTagged", &nTagged, EISingleX);
  TA2DataManager::LoadVariable("NPhoton", &nPhoton, EISingleX);
  TA2DataManager::LoadVariable("NProton", &nProton, EISingleX);
  TA2DataManager::LoadVariable("NPiPlus", &nPiPlus, EISingleX);
}

//---------------------------------------------------------------------------

void TA2BasePhysics::PostInit()
{
  //Call default PostInit()
  TA2AccessSQL::PostInit();

  //Int_t nChilds;
  
  //'Permutations:' line in configuration file gives maximum number of particles to be processed
  nBeam = fNpermutation[0]; //Tagger
  nBall = fNpermutation[1]; //Crystal Ball
  nWall = fNpermutation[2]; //TAPS
  nPart = nBall + nWall;    //Sum of final state particles (CB + TAPS)

  //Allocate memory
  Photon = new TA2Particle[nPart]; //TA2Particle carrying photon informations
  Proton = new TA2Particle[nPart]; //TA2Particle carrying proton informations
  PiPlus = new TA2Particle[nPart]; //TA2Particle carrying pi+ informations
  
}

//-----------------------------------------------------------------------------

void TA2BasePhysics::Reconstruct()
{
  //Call default Reconstruct()
  TA2AccessSQL::Reconstruct();

  nPhoton = 0;
  nProton = 0;
  nPiPlus = 0;
  nTagged = 0;
  ESum = EBufferEnd; //Clear CB energy sum spectrum
  Mult = EBufferEnd; //Clear multiplicity spectrum
  L1Pattern = 0x00;
  L2Pattern = 0x00;

  //Evaluate trigger
  TriggerProcessHW();          //Collect hardware trigger information (if available)
  TriggerProcessSW();          //Test trigger conditions in software (if requested)
  if(!TriggerDecode()) return; //If trigger not fulfilled, go to next event

  //Write all TA2Particles to ROOT file
  if(bDoROOT)
  {
    Event.Clear();
    Event.SetL1Pattern(L1Pattern);
    Event.SetL2Pattern(L2Pattern);
    
    //Pick up particles from whatever tagger class is used
    if(fTagger)
      for(Int_t nTagger=0; nTagger<fTagger->GetNParticle(); nTagger++)
        Event.AddBeam(fTagger->GetParticles(nTagger));
      
    //Pick up particles from whatever CB class is used
    if(fCB)
      for(Int_t nCB=0; nCB<fCB->GetNParticle(); nCB++)
        Event.AddParticle(fCB->GetParticles(nCB));

    //Pick up particles from TAPS class
    if(fTAPS)
      for(Int_t nTAPS=0; nTAPS<fTAPS->GetNParticle(); nTAPS++)
        Event.AddParticle(fTAPS->GetParticles(nTAPS));
      
    EventTree->Fill();
    Event.SetEventNumber();
  }

  //Collect detected photons, protons and pi+ from CB
  if(fCB) //Collect particles in CB, if corresponding class (TA2CB) is available...
    for(Int_t nCB=0; nCB<fCB->GetNParticle(); nCB++)
    {
      if(fCB->GetParticles(nCB).GetParticleID()==kGamma) //If it is a photon in CB...
      {
        Photon[nPhoton] = fCB->GetParticles(nCB);        //...copy to photon TA2Particle array
        nPhoton++;
      }
      else if(fCB->GetParticles(nCB).GetParticleID()==kProton) //If it is a proton in CB...
      {
        Proton[nProton] = fCB->GetParticles(nCB);              //...copy to proton TA2Particle array
        nProton++;
      }
      else if(fCB->GetParticles(nCB).GetParticleID()==kPiPlus) //If it is a proton in CB...
      {
        PiPlus[nPiPlus] = fCB->GetParticles(nCB);              //...copy to proton TA2Particle array
        nPiPlus++;
      }
    }

  //Collect detected photons, protons and pi+ from TAPS
  if(fTAPS) //Collect particles in TAPS, if available
    for(Int_t nTAPS=0; nTAPS<fTAPS->GetNParticle(); nTAPS++)
    {
      if(fTAPS->GetParticles(nTAPS).GetParticleID()==kGamma) //If it is a photon in TAPS...
      {
        Photon[nPhoton] = fTAPS->GetParticles(nTAPS);        //...copy to photon TA2Particle array
        nPhoton++;
      }
      else if(fTAPS->GetParticles(nTAPS).GetParticleID()==kProton) //If it is a proton in TAPS...
      {
        Proton[nProton] = fTAPS->GetParticles(nTAPS);              //...copy to proton TA2Particle array
        nProton++;
      }
      else if(fTAPS->GetParticles(nTAPS).GetParticleID()==kPiPlus) //If it is a proton in TAPS...
      {
        PiPlus[nPiPlus] = fTAPS->GetParticles(nTAPS);              //...copy to proton TA2Particle array
        nPiPlus++;
      }
    }

  //Get Tagger information (TA2Particle for each tagged beam photon)
  if(fTagger)
  {
	Tagged = fTagger->GetParticles();
	nTagged = fTagger->GetNParticle();
  }
  
}

//-----------------------------------------------------------------------------

void TA2BasePhysics::ParseMisc(char* line)
{
  FILE* WinFile;
  Char_t sWord[256];

  //Get keyword
  if(sscanf(line, "%s", sWord)!=1) return;

  if(!strcmp("OutputFile", sWord))
  {
    sscanf(line, "%*s %s", OutputFilename);
    printf("Writing TA2Particles to %s\n", OutputFilename);
    EventFile = new TFile(OutputFilename, "RECREATE", "EventFile", 3);
    EventTree = new TTree("EventTree", "EventTree");
    EventTree->Branch("Event", &Event);
    bDoROOT = true;
    return;
  }

  if(!strcmp("TimeWindows", sWord))
  {
    sscanf(line, "%*s %s", WindowFilename);
    printf("Time windows from:\n %s\n", WindowFilename);
    WinFile = fopen(WindowFilename, "r");
    for(Int_t t=1; t<NWINDOW+1; t++)
    {
      if(fscanf(WinFile, "%lf %lf", &Window[t][0], &Window[t][1]));
      printf("Time window %d from %f to %f\n", t, Window[t][0], Window[t][1]);
    }
    fclose(WinFile);
    return;
  }

  //Not a valid keyword found
  printf("Misc keyword %s not supported\n", sWord);
  return;
}

//-----------------------------------------------------------------------------

void TA2BasePhysics::TriggerProcessSW()
{
  if(!SimulateTrigger) return;

  Double_t ENaI = 0.0;       //Energy of a single NaI channel
  Double_t EBaF2 = 0.0;      //Energy of a single BaF2 channel
  Double_t EDisc[45];        //Energy in every CB discriminator
  UInt_t DoneBaF2 = 0;       //Number of already processed TAPS channels
  UInt_t Skip = 0;
  Bool_t FlagDisc;           //Flag whether a discriminator contributed to multiplicity
  Bool_t FlagBlock[6] =      //Flag, wether a TAPS block contributed to multiplicity
         {0,0,0,0,0,0};
  Bool_t TrigEnergy = false; //1st level trigger (CB energy sum)
  Bool_t TrigM[5] =          //2nd level triggers (Multiplicity M1-M4)
         {false,false,false,false,false};
  static UInt_t LatchM[5] =  //Prescaled events counter (Multiplicity M1-M4)
                {0,0,0,0,0};

  //Initialise L1 and L2 trigger
  ESum = 0.0;
  Mult = 0;

  //CB trigger: Loop over all 45 CB discriminators
  for(UInt_t i=0; i<45; i++)
  {
    EDisc[i] = 0.0;
    FlagDisc = false;
    //Loop over all 16 discriminator channels
    for(UInt_t j=0; j<16; j++)
    {
      ENaI = fNaI->GetEnergyAll(i*16 + j);            //Get energy of crystal
      if(ENaI > gRandom->Gaus(ThresNaI[i*16 + j]*ScaleNaI, SigmaNaI[i*16 + j]*ScaleNaI))
        FlagDisc = true;                             //Check if crystal above threshold (block marked as hit)
      ENaI/=GainsNaI[i*16 + j];                      //Reconstruct original analogue signal ('de-calibrate')...
      EDisc[i]+=ENaI;                                //...and sum up analogue signals in block
    }
    ESum+=EDisc[i];      //Add analoge block sum to total CB energy sum,
    if(FlagDisc) Mult++; //If block marked as hit, contribute to multiplicity
  }
  if(ESum > 2000.0) ESum = 2000.0;

  //TAPS trigger (if enabled): Loop over the 6 TAPS blocks
  if(Rings < 3)
  {
    for(UInt_t i=0; i<6; i++)
    {
      if(Rings==1) Skip = (NewTAPS-61)/3;
      if(Rings==2) Skip = (NewTAPS-61);
      //Loop over all channels in one block: 73 elements in case of BaF2 & PbWO4, 64 in case of BaF2 only
      for(UInt_t j=Skip; j<NewTAPS; j++)
      {  
        EBaF2 = fBaF2PWO->GetEnergyAll(DoneBaF2 + j); //Get energy of crystal
        if(EBaF2 > gRandom->Gaus(ThresBaF2[DoneBaF2 + j], SigmaBaF2[DoneBaF2 + j]))
          FlagBlock[i] = true;                    //Check if crystal above threshold (block marked as hit)
      }
      DoneBaF2+=NewTAPS;            //Count how many channels have been processed so far
      if(FlagBlock[i]) Mult++; //If block marked as hit, contribute to multiplicity
    }
  }

  //Evaluate 1st level trigger and reset 2nd level trigger options
  if(UseSumModel)
    TrigEnergy = (gRandom->Rndm() < SumModel[(Int_t)ESum]*ScaleNaI);    //Use efficiency table for energy sum threshold
  else
    TrigEnergy = (ESum > gRandom->Gaus(ESumThres*ScaleNaI, ESumSigma*ScaleNaI)); //Smear out energy sum threshold
  for(UInt_t m=1; m<5; m++) TrigM[m] = false;

  //1st level trigger fulfilled?
  if(TrigEnergy)
    for(UInt_t m=1; m<5; m++)
    {
      if(Mult>=m)                   //Accept if correct multiplicity ...
      {
        if(LatchM[m]==PrescaleM[m]) //... and correct prescaling
        {
          TrigM[m] = true;  //Accept event and ...
          LatchM[m] = 0;    //... reset prescaler count
        }
        else
        {
          TrigM[m] = false; //Discard event, because M trigger fulfilled, ...
          LatchM[m]++;      //... but event needs to be scaled away
        }
      }
    }

  //Create trigger pattern equivalent to hardware trigger in fADC[0,1]
  L1Pattern = 0x00;
  L2Pattern = 0x07; //Set 'internal' bits
  if(TrigEnergy) L1Pattern = (L1Pattern | 0x01);
  if(TrigM[1])   L2Pattern = (L2Pattern | 0x10);
  if(TrigM[2])   L2Pattern = (L2Pattern | 0x20);
  if(TrigM[3])   L2Pattern = (L2Pattern | 0x40);
  if(TrigM[4])   L2Pattern = (L2Pattern | 0x80);
}

//-----------------------------------------------------------------------------

void TA2BasePhysics::TriggerProcessHW()
{
  if(SimulateTrigger) return;

  if(fADC)
  {
    L1Pattern = fADC[0] & 0xFF;
    L2Pattern = fADC[1] & 0xFF;
  }
}

//-----------------------------------------------------------------------------

Bool_t TA2BasePhysics::TriggerDecode()
{
  //Scaler events are not interesting here
  if(gAR->IsScalerRead()) return false;

  //Trigger simulation enabled?
  if(!DoTrigger) return true;

  //ADC 0: (L1)
  //  1 $01 00000001: CB energy sum
  //  2 $02 00000010: none
  //  4 $04 00000100: PID
  //  8 $08 00001000: TAPS OR
  // 16 $10 00010000: TAPS pedestal pulser
  // 32 $20 00010000: TAPS PbWO4
  // 64 $40 00010000: TAPS M2
  //128 $80 00010000: CB pulser
  // =>
  //  1 $01 00000001: CB energy sum
  //  9 $09 00001001: CB energy sum & TAPS OR

  //ADC 1: (L2)
  //  1 $01 00000001: 'internal' (!TAPS pedestal pulser?)
  //  2 $02 00000010: 'internal' (always 1)
  //  4 $04 00000010: 'internal' (always 1)
  //  8 $08 00000100: Tagger coincidence (not connected?)
  // 16 $10 00010000: M1
  // 32 $20 00100000: M2
  // 64 $40 01000000: M3
  //128 $80 10000000: M4
  // =>
  //  6 $06 00000110  internal (TAPS pedestal readout?)
  // 39 $27 00100111  M2
  //103 $67 01100111  M2, M3
  //103 $E7 11100111  M2, M3, M4

  Bool_t HasEnergy;
  Bool_t HasM[5];

  HasEnergy = (L1Pattern & 0x01) && (L2Pattern & 0x01); //Accept events with CB energy sum & events which are not TAPS pedestal events
  HasM[1] = HasEnergy && (L2Pattern & 0x10) && UseM[1]; //Accept events with CB energy sum & multiplicity M1 if wanted
  HasM[2] = HasEnergy && (L2Pattern & 0x20) && UseM[2]; //Accept events with CB energy sum & multiplicity M2 if wanted
  HasM[3] = HasEnergy && (L2Pattern & 0x40) && UseM[3]; //Accept events with CB energy sum & multiplicity M3 if wanted
  HasM[4] = HasEnergy && (L2Pattern & 0x80) && UseM[4]; //Accept events with CB energy sum & multiplicity M4 if wanted

  if(!HasEnergy) ESum = EBufferEnd; //If 1st level trigger not fulfilled, do not histogram energy sum

  return (HasM[1] || HasM[2] || HasM[3] || HasM[4]); //Trigger if one or more (Esum & Mx) condition(s) fulfilled
}

//-----------------------------------------------------------------------------
ClassImp(TA2BasePhysics)
