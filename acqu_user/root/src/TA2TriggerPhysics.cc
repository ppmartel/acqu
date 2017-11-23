#include "TA2TriggerPhysics.h"
#include "TAcquFile.h"
#include <time.h>


//-----------------------------------------------------------------------------

TA2TriggerPhysics::TA2TriggerPhysics(const char* Name, TA2Analysis* Analysis) : TA2Physics(Name, Analysis)
{

}

//-----------------------------------------------------------------------------

TA2TriggerPhysics::~TA2TriggerPhysics()
{

}

//---------------------------------------------------------------------------

void TA2TriggerPhysics::PostInit()
{

  ThresNaI = new Double_t[720];
  ThresBaF2 = new Double_t[438];

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  TA2Physics::PostInit();

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  //Get apparati and NaI/BaF2 detector arrays
  CB = (TA2CB*)((TA2Analysis*)fParent)->GetChild("CB");
  TAPS = (TA2Taps*)((TA2Analysis*)fParent)->GetChild("TAPS");
  NaI = (TA2CalArray*)CB->GetChild("NaI",  "TA2ClusterDetector");
  BaF2 = (TA2TAPS_BaF2*)TAPS->GetChild("BaF2", "TA2ClusterDetector");
}

//-----------------------------------------------------------------------------

void TA2TriggerPhysics::LoadVariable()
{
  TA2Physics::LoadVariable();

  TA2DataManager::LoadVariable("NaIThres",  ThresNaI,  EDSingleX);
  TA2DataManager::LoadVariable("BaF2Thres", ThresBaF2, EDSingleX);
}

//-----------------------------------------------------------------------------

void TA2TriggerPhysics::Reconstruct()
{
  //Speed measurement
  Speed();

  //Method: Plot only 3 highest energies for M3 events
  Double_t EnergyNaI[45][16];
  Int_t CrystalNaI[45][16];
  Double_t EnergyBaF2[6][73];
  Int_t CrystalBaF2[6][73];

  Double_t MaxEnergy[51];
  Int_t MaxCrystal[51];
  Int_t MaxApparatus[51];

  UInt_t NProcElem = 0;
  Int_t Multi = 0;

  //Clear spectra
  for(UInt_t u=0; u<720; u++)
    ThresNaI[u] = EBufferEnd;
  for(UInt_t u=0; u<438; u++)
    ThresBaF2[u] = EBufferEnd;
  for(Int_t u=0; u<51; u++)
  {
     MaxCrystal[u] = -1;
     MaxApparatus[u] = -1;
     MaxEnergy[u] = EBufferEnd;
  }

  //Cancel, if either scaler read or trigger patterns not accessible
  if(gAR->IsScalerRead()) return;
  if(!fADC) return;

  //Get energy sum events and find out multiplicity
  Bool_t IsCB = (fADC[0] & 0x01); //Accept only events with energy sum
  if(fADC[1] & 0x10) Multi = 1;
  if(fADC[1] & 0x20) Multi = 2;
  if(fADC[1] & 0x40) Multi = 3;
  if(fADC[1] & 0x80) Multi = 4;

  if(IsCB && (Multi > 0))
  {
    //Loop over all CB discriminators
    for(UInt_t i=0; i<45; i++)
    {
      //Loop over all 16 discriminator channels
      for(UInt_t j=0; j<16; j++)
      {
        //Pick up all energies and crystal numbers according to discriminator blocks
        EnergyNaI[i][j] = ((TA2CalArray*)NaI)->GetEnergyAll(i*16 + j);
        CrystalNaI[i][j] = (i*16 + j);
      }

      //Get maximum energy in discriminator block
      MaxEnergy[i] = 0.0;
      for(Int_t u=0; u<16; u++)
        if(EnergyNaI[i][u] > MaxEnergy[i])
        {
          MaxEnergy[i] = EnergyNaI[i][u];
          MaxCrystal[i] = CrystalNaI[i][u];
          MaxApparatus[i] = APP_CB;
        }
    }

    //Loop over the 6 TAPS blocks
    NProcElem = 0;
    for(UInt_t i=0; i<6; i++)
    {
      for(UInt_t j=12; j<73; j++) //Loop over all BaF2 channels in one block (PbWO4 should not contribute to trigger)
      {
        EnergyBaF2[i][j] =((TA2TAPS_BaF2*)BaF2)->GetEnergyAll(NProcElem + j);
        CrystalBaF2[i][j] = (NProcElem + j);
      }
      NProcElem+=73; //Count how many channels already have been processed

      //Get maximum energy in discriminator block
      MaxEnergy[i+45] = 0.0;
      for(UInt_t u=12; u<73; u++)
        if(EnergyBaF2[i][u] > MaxEnergy[i+45])
        {
          MaxEnergy[i+45] = EnergyBaF2[i][u];
          MaxCrystal[i+45] = CrystalBaF2[i][u];
          MaxApparatus[i+45] = APP_TAPS;
        }
    }

    //Sort discriminator blocks
    for(Int_t u=50; u>0; u--)
      for(Int_t v=0; v<u; v++)
        if(MaxEnergy[v] < MaxEnergy[v+1])
        {
          Swap(&MaxEnergy[v], &MaxEnergy[v+1]);
          Swap(&MaxCrystal[v], &MaxCrystal[v+1]);
          Swap(&MaxApparatus[v], &MaxApparatus[v+1]);
        }

    //Accept highest energies for given multiplicity
    for(Int_t u=0; u<Multi; u++)
    {
      if(MaxApparatus[u]==APP_CB)
        ThresNaI[MaxCrystal[u]] = MaxEnergy[u];
      else if(MaxApparatus[u]==APP_TAPS)
        ThresBaF2[MaxCrystal[u]] = MaxEnergy[u];
    }
  }
}

//-----------------------------------------------------------------------------

void TA2TriggerPhysics::Swap(Double_t* pA, Double_t* pB)
{
  Double_t fTmp;

  fTmp = *pA;
  *pA = *pB;
  *pB = fTmp;
}

//-----------------------------------------------------------------------------

void TA2TriggerPhysics::Swap(Int_t* pA, Int_t* pB)
{
  Int_t fTmp;

  fTmp = *pA;
  *pA = *pB;
  *pB = fTmp;
}

//-----------------------------------------------------------------------------

void TA2TriggerPhysics::Speed()
{
  static Int_t iStart = time(NULL);
  static Int_t nEvents = 0;

  nEvents++;
  if(time(NULL)-iStart>=60)
  {
    iStart = time(NULL);
    printf("Processed %d Events/sec\n", nEvents/60);
    nEvents = 0;
  }
}

//-----------------------------------------------------------------------------
ClassImp(TA2TriggerPhysics)
